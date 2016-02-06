#include "RequestQueuer.hpp"
#include "utils.hpp"

#include <minijson_writer.hpp>

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

WALLAROO_REGISTER(RequestQueuer);

processing::RequestQueuer::RequestQueuer()
        : logger(log4cpp::Category::getInstance("RequestQueuer")),
          requestProcessors("requestProcessors", RegistrationToken()),
          jsonReader(Json::Reader(Json::Features::strictMode())) {
}

void processing::RequestQueuer::Init() {
    requestProcessorExecutorThread.reset(new thread(&RequestQueuer::requestProcessorExecutorThreadFunction, this));
    int result = pthread_setname_np(requestProcessorExecutorThread->native_handle(), "reqProcExec");
    if (result != 0) {
        logger.error("Cannot set thread name: %s.", strerror(result));
    }
    logger.notice("Instance created.");
}

processing::RequestQueuer::~RequestQueuer() {
    requestsMutex.lock();
    finishCycleThread = true;
    requestsMutex.unlock();

    logger.notice("Waiting for processor thread to be stopped.");
    cv.notify_one();
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

    Json::Value req;
    bool parseSuccess = jsonReader.parse(requestString, req);

    if (not parseSuccess) {
        logger.error("Received request is not valid JSON document. See NOTICE for details.");
        logger.notice("Details of the invalid request: " + jsonReader.getFormattedErrorMessages());
        return INVALID_MESSAGE;
    }

    if (not req["serial"].isInt()) {
        logger.error("Request does not contain valid serial.");
        return INVALID_MESSAGE;
    }

    auto serial = req["serial"].asInt();

    if (serial != 0 and serial <= lastSerial) {
        logger.warn("Request has too old serial (%d). Skipping.", serial);
        return INVALID_MESSAGE;
    }

    if (requests.size() == REQUEST_QUEUE_MAX_SIZE) {
        logger.warn("Requests queue is full (%d). removing the oldest one.", REQUEST_QUEUE_MAX_SIZE);

        long id;

        tie(id, ignore, ignore) = requests.top();

        requests.pop();

        if (rejectedRequestRemover == nullptr) {
            logger.error("Cannot remove rejected request. No RejectedRequestRemover set.");
        } else {
            rejectedRequestRemover(id);
        }
    }

    if (serial == 0) {
        logger.notice("Request has the serial = 0, resetting counter and clearing queue.");
        for (unsigned int i = 0; i < requests.size(); ++i) {
            requests.pop();
        }
        lastSerial = 0;
    }

    long id = nextId();

    requests.push(make_tuple(id, address, req));

    logger.info("Pushed request with the serial %d. Queue size: %d.", serial, requests.size());

    cv.notify_one();

    return id;
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

        long id;
        asio::ip::address addr;
        Json::Value req;

        tie(id, addr, req) = requests.top();
        requests.pop();

        lk.unlock();

        auto serial = req["serial"].asInt();

        if (serial < lastSerial) {
            logger.warn("Request has too old serial (%d). Skipping (from executor).", serial);
            continue;
        } else {
            lastSerial = serial;
        }

        logger.info("Executing request with serial %d from %s.", serial, addr.to_string().c_str());

        char responseBuffer[1024];
        std::memset(responseBuffer, 0, 1024);
        boost::interprocess::bufferstream responseStream(responseBuffer, 1024);

        minijson::object_writer response(responseStream);

        response.write("serial", serial);

        auto execTimeMicroseconds = common::utils::measureTime<chrono::microseconds>([&]() {
            for (auto proc : requestProcessors) {
                std::shared_ptr<IRequestProcessor>(proc)->process(req, addr, response);
            }
        });

        response.write("timestamp", common::utils::getTimestamp());

        response.close();

        logger.info("Request %d executed in %d us.", serial, execTimeMicroseconds);

        bool skipResponse = req["skip_response"].asBool();

        if (skipResponse) {
            logger.debug("Skipping response sending.");
        } else {
            logger.debug(string("Response in JSON: ") + responseBuffer);
        }

        if (responseSender == nullptr) {
            logger.error("Cannot send response. No ResponseSender set.");
        } else {
            responseSender(id, responseBuffer, not skipResponse);
        }
    }
}

