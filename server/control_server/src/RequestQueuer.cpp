#include "RequestQueuer.hpp"
#include "utils.hpp"

#include <minijson_writer.hpp>
#include <minijson_reader.hpp>

#include <boost/interprocess/streams/bufferstream.hpp>

#include <iomanip>

using namespace std;
using namespace boost;
using namespace processing;

/**
* Defines the maximum requests queue size.
*/
constexpr int REQUEST_QUEUE_MAX_SIZE = 100;

/**
* If true, new requests will be skipped. If false, new requests will replace older ones.
*/
constexpr bool REQUEST_QUEUE_OVERFLOW_BEHAVIOR = false;

constexpr int REQUEST_MAX_LENGTH = 1024;
constexpr int RESPONSE_MAX_LENGTH = 512;

WALLAROO_REGISTER(RequestQueuer);

processing::RequestQueuer::RequestQueuer()
        : logger(log4cpp::Category::getInstance("RequestQueuer")),
          requestProcessors("requestProcessors", RegistrationToken()) {
}

void processing::RequestQueuer::Init() {
    requestProcessorExecutorThread.reset(new thread(&RequestQueuer::requestProcessorExecutorThreadFunction, this));
    common::utils::setThreadName(logger, requestProcessorExecutorThread.get(), "reqProcExec");
    logger.notice("Instance created.");
}

processing::RequestQueuer::~RequestQueuer() {
    requestsMutex.lock();
    finishCycleThread = true;
    requestsMutex.unlock();

    logger.notice("Waiting for processor thread to be stopped.");
    cv.notify_all();
    if (requestProcessorExecutorThread.get() != nullptr) {
        requestProcessorExecutorThread->join();
    }

    logger.notice("Instance destroyed.");
}

long processing::RequestQueuer::addRequest(string requestString, boost::asio::ip::address address) {
    unique_lock<mutex> lk(requestsMutex);

    logger.debug("Received request with the size of %d bytes.", requestString.length());

    if (requests.size() == REQUEST_QUEUE_MAX_SIZE and REQUEST_QUEUE_OVERFLOW_BEHAVIOR) {
        logger.warn("Requests queue is full (%d). Skipping request.", REQUEST_QUEUE_MAX_SIZE);
        return INVALID_MESSAGE;
    }

    std::shared_ptr<Request> request(new Request());

    try {
        minijson::const_buffer_context ctx(requestString.c_str(), requestString.size());
        minijson::parse_object(ctx, [&](const char *k, minijson::value v) {
            minijson::dispatch(k)
            << "serial" >> [&] { request->serial = v.as_long(); }
            << "skip_response" >> [&] { request->skipResponse = v.as_bool(); }
            << "tss" >> [&] { request->sendTimestamp = v.as_string(); }

            << minijson::any >> [&] { minijson::ignore(ctx); };
        });
    } catch (minijson::parse_error &e) {
        logger.error("Received request is not valid JSON document: %s.", e.what());
        return INVALID_MESSAGE;
    }

    if (request->serial < 0) {
        logger.error("Request does not contain valid serial.");
        return INVALID_MESSAGE;
    }

    if (request->serial != 0 and request->serial <= lastSerial) {
        logger.warn("Request has too old serial (%d). Skipping.", request->serial);
        return INVALID_MESSAGE;
    }

    request->receiveTimestamp = common::utils::getTimestamp();

    if (requests.size() == REQUEST_QUEUE_MAX_SIZE) {
        logger.warn("Requests queue is full (%d). removing the oldest one.", REQUEST_QUEUE_MAX_SIZE);

        auto toppedRequest = requests.top();

        requests.pop();

        if (rejectedRequestRemover == nullptr) {
            logger.error("Cannot remove rejected request. No RejectedRequestRemover set.");
        } else {
            rejectedRequestRemover(toppedRequest->internalId);
        }
    }

    if (request->serial == 0) {
        logger.notice("Request has the serial = 0, resetting counter and clearing queue.");
        for (unsigned int i = 0; i < requests.size(); ++i) {
            requests.pop();
        }
        lastSerial = 0;
    }

    request->internalId = nextId();
    request->ipAddress = address;
    request->reqJson = requestString;

    requests.push(request);

    logger.info("Pushed request with the serial %d. Queue size: %d.", request->serial, requests.size());

    cv.notify_one();

    return request->internalId;
}

int processing::RequestQueuer::getNumOfMessages() {
    unique_lock<mutex> lk(requestsMutex);
    return requests.size();
}

int processing::RequestQueuer::getNumOfProcessors() {
    return requestProcessors.size();
}

long processing::RequestQueuer::nextId() {
    static long id = 1;
    id++;
    return id;
}

void processing::RequestQueuer::requestProcessorExecutorThreadFunction() {
    while (true) {
        unique_lock<mutex> lk(requestsMutex);

        if (requests.empty()) {
            cv.wait(lk, [&]() {
                return finishCycleThread or (not requests.empty());
            });
        }

        if (finishCycleThread) {
            return;
        }

        auto request = requests.top();
        requests.pop();

        lk.unlock();

        if (request->serial < lastSerial) {
            logger.warn("Request has too old serial (%d). Skipping (from executor).", request->serial);
            continue;
        } else {
            lastSerial = request->serial;
        }

        logger.info("Executing request with serial %d from %s.", request->serial,
                    request->ipAddress.to_string().c_str());

        char responseBuffer[RESPONSE_MAX_LENGTH];
        std::memset(responseBuffer, 0, RESPONSE_MAX_LENGTH);
        boost::interprocess::bufferstream responseStream(responseBuffer, RESPONSE_MAX_LENGTH);

        minijson::object_writer response(responseStream);

        response.write("serial", request->serial);
        response.write("tss", request->sendTimestamp);
        response.write("tsr", request->receiveTimestamp);
        response.write("tspb", common::utils::getTimestamp());

        auto execTimeMicroseconds = common::utils::measureTime<chrono::microseconds>([&]() {
            for (auto proc : requestProcessors) {
                std::shared_ptr<IRequestProcessor>(proc)->process(*request, response);
            }
        });

        response.write("tspe", common::utils::getTimestamp());

        response.close();

        logger.info("Request %d executed in %d us.", request->serial, execTimeMicroseconds);

        if (request->skipResponse) {
            logger.debug("Skipping response sending.");
        } else {
            logger.debug("Response in JSON: %s", responseBuffer);
        }

        if (responseSender == nullptr) {
            logger.error("Cannot send response. No ResponseSender set.");
        } else {
            responseSender(request->internalId, responseBuffer, not request->skipResponse);
        }
    }
}

