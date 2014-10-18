/*
 * RequestQueuer.cpp
 *
 *  Project: server
 *  Created on: 26 kwi 2014
 *
 *  Copyright 2014 Michał Słomkowski m.slomkowski@gmail.com
 *
 *	This program is free software; you can redistribute it and/or modify it
 *	under the terms of the GNU General Public License version 3 as
 *	published by the Free Software Foundation.
 */

#include <chrono>
#include <iomanip>
#include <boost/format.hpp>

#include "utils.hpp"
#include "IRequestProcessor.hpp"
#include "RequestQueuer.hpp"

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
	logger.notice("Instance created.");
}

processing::RequestQueuer::~RequestQueuer() {
	requestsMutex.lock();
	finishCycleThread = true;
	requestsMutex.unlock();

	logger.notice("Waiting for processor thread to be stopped.");
	cv.notify_one();
	requestProcessorExecutorThread->join();

	logger.notice("Instance destroyed.");
}

long processing::RequestQueuer::addRequest(string requestString) {
	unique_lock<mutex> lk(requestsMutex);

	logger.debug((format("Received request with the size of %d bytes.") % requestString.length()).str());

	if (requests.size() == REQUEST_QUEUE_MAX_SIZE and REQUEST_QUEUE_OVERFLOW_BEHAVIOR) {
		logger.warn((format("Requests queue is full (%d). Skipping request.") % REQUEST_QUEUE_MAX_SIZE).str());
		return INVALID_MESSAGE;
	}

	Json::Value req;
	bool parseSuccess = jsonReader.parse(requestString, req);

	if (not parseSuccess) {
		logger.error("Received request is not valid JSON document. See NOTICE for details.");
		logger.notice("Details of the invalid request: " + jsonReader.getFormatedErrorMessages());
		return INVALID_MESSAGE;
	}

	if (not req["serial"].isInt()) {
		logger.error("Request does not contain valid serial.");
		return INVALID_MESSAGE;
	}

	auto serial = req["serial"].asInt();

	if (serial != 0 and serial <= lastSerial) {
		logger.warn((format("Request has too old serial (%d). Skipping.") % serial).str());
		return INVALID_MESSAGE;
	}

	if (requests.size() == REQUEST_QUEUE_MAX_SIZE) {
		logger.warn(
				(format("Requests queue is full (%d). removing the oldest one.") % REQUEST_QUEUE_MAX_SIZE).str());

		long id;

		tie(id, ignore) = requests.top();

		requests.pop();

		if (rejectedRequestRemover == nullptr) {
			logger.error("Cannot remove rejected request. No RejectedRequestRemover set.");
		} else {
			rejectedRequestRemover(id);
		}
	}

	if (serial == 0) {
		logger.notice("Request has the serial = 0, resetting counter.");
		lastSerial = 0;
	}

	long id = nextId();

	requests.push(make_pair(id, req));

	logger.info(
			(format("Pushed request with the serial %d. Queue size: %d.") % serial % requests.size()).str());

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
		Json::Value req;

		tie(id, req) = requests.top();
		requests.pop();

		lk.unlock();

		auto serial = req["serial"].asInt();

		if (serial < lastSerial) {
			logger.warn((format("Request has too old serial (%d). Skipping (from executor).") % serial).str());
			continue;
		} else {
			lastSerial = serial;
		}

		logger.info((format("Executing request with serial %d.") % serial).str());

		Json::Value response;

		response["serial"] = serial;

		auto execTimeMicroseconds = common::utils::measureTime<chrono::microseconds>([&]() {
			for (auto proc : requestProcessors) {
				std::shared_ptr<IRequestProcessor>(proc)->process(req, response);
			}
		});

		response["timestamp"] = common::utils::getTimestamp();

		logger.info((format("Request %d executed in %d us.") % serial % execTimeMicroseconds).str());

		bool skipResponse = req["skip_response"].asBool();

		if (skipResponse) {
			logger.debug("Skipping response sending.");
		} else {
			logger.debug(string("Response in JSON: ") + jsonWriter.write(response));
		}

		if (responseSender == nullptr) {
			logger.error("Cannot send response. No ResponseSender set.");
		} else {
			responseSender(id, jsonWriter.write(response), not skipResponse);
		}
	}
}

