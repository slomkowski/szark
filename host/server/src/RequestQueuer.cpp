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

#include <boost/format.hpp>

#include "RequestQueuer.hpp"

namespace processing {

/**
 * Defines the maximum requests queue size.
 */
const int REQUEST_QUEUE_MAX_SIZE = 100;

/**
 * If true, new requests will be skipped. If false, new requests will replace older ones.
 */
const bool REQUEST_QUEUE_OVERFLOW_BEHAVIOR = false;

WALLAROO_REGISTER(RequestQueuer);

RequestQueuer::RequestQueuer()
		: logger(log4cpp::Category::getInstance("RequestQueuer")),
				requestProcessors("requestProcessors", RegistrationToken()),
				jsonReader(Json::Reader(Json::Features::strictMode())) {

	requestProcessorExecutorThread.reset(new std::thread(&RequestQueuer::requestProcessorExecutorThreadFunction, this));

	logger.notice("Instance created.");
}

RequestQueuer::~RequestQueuer() {
	requestsMutex.lock();
	finishCycleThread = true;
	requestsMutex.unlock();

	logger.notice("Waiting for processor thread to be stopped.");
	cv.notify_one();
	requestProcessorExecutorThread->join();

	logger.notice("Instance destroyed.");
}

bool RequestQueuer::addRequest(std::string requestString) {
	std::unique_lock<std::mutex> lk(requestsMutex);

	logger.debug((boost::format("Received request with the size of %d bytes.") % requestString.length()).str());

	if (requests.size() == REQUEST_QUEUE_MAX_SIZE and REQUEST_QUEUE_OVERFLOW_BEHAVIOR) {
		logger.warn((boost::format("Requests queue is full (%d). Skipping request.") % REQUEST_QUEUE_MAX_SIZE).str());
		return false;
	}

	Json::Value req;
	bool parseSuccess = jsonReader.parse(requestString, req);

	if (not parseSuccess) {
		logger.error("Received request is not valid JSON document. See NOTICE for details.");
		logger.notice("Details of the invalid request: " + jsonReader.getFormatedErrorMessages());
		return false;
	}

	if (not req["serial"].isInt()) {
		logger.error("Request does not contain valid serial.");
		return false;
	}

	auto serial = req["serial"].asInt();

	if (serial != 0 and serial <= lastSerial) {
		logger.warn((boost::format("Request has too old serial (%d). Skipping.") % serial).str());
		return false;
	}

	if (requests.size() == REQUEST_QUEUE_MAX_SIZE) {
		logger.warn(
				(boost::format("Requests queue is full (%d). removing the oldest one.") % REQUEST_QUEUE_MAX_SIZE).str());
		requests.pop();
	}

	if (serial == 0) {
		logger.notice("Request has the serial = 0, resetting counter.");
		lastSerial = 0;
	}

	requests.push(req);

	logger.info(
			(boost::format("Pushed request with the serial %d. Queue size - %d.") % serial % requests.size()).str());

	cv.notify_one();

	return true;
}

int RequestQueuer::getNumOfMessagess() {
	std::unique_lock<std::mutex> lk(requestsMutex);
	return requests.size();
}

int RequestQueuer::getNumOfProcessors() {
	return requestProcessors.size();
}

void RequestQueuer::requestProcessorExecutorThreadFunction() {
	while (true) {
		std::unique_lock<std::mutex> lk(requestsMutex);

		//cv.wait(lk, [this]() {return finishCycleThread or (not requests.empty());});
		if (requests.empty()) {
			cv.wait(lk);
		}

		if (finishCycleThread) {
			return;
		}

		Json::Value req = requests.top();
		requests.pop();

		lk.unlock();

		auto serial = req["serial"].asInt();

		if (serial < lastSerial) {
			logger.warn((boost::format("Request has too old serial (%d). Skipping (from executor).") % serial).str());
			continue;
		} else {
			lastSerial = serial;
		}

		logger.info((boost::format("Executing request with serial %d.") % serial).str());

		Json::Value response;

		for (auto proc : requestProcessors) {
			response.append(std::shared_ptr<IRequestProcessor>(proc)->process(req));
		}

		//TODO trzeba coś zrobić z odpowiedzią
	}
}

} /* namespace processing */
