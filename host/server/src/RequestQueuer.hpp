/*
 * RequestQueuer.hpp
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
#ifndef REQUESTQUEUER_HPP_
#define REQUESTQUEUER_HPP_

#include <string>
#include <list>
#include <vector>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>

#include <log4cpp/Category.hh>
#include <wallaroo/registered.h>
#include <json/value.h>
#include <json/reader.h>

#include "IRequestProcessor.hpp"

namespace processing {

class IRequestQueuer {
public:
	/**
	 * Adds the request to the queue.
	 * @param request text of the request in JSON format
	 * @return true if the request was added to the queue
	 */
	virtual bool addRequest(std::string request) = 0;

	virtual int getNumOfMessagess() = 0;

	virtual int getNumOfProcessors() = 0;

	virtual ~IRequestQueuer() = default;
};

class RequestValueComparer {
public:
	bool operator()(const Json::Value &r1, const Json::Value &r2) const {
		return r1["serial"].asInt() < r2["serial"].asInt();
	}
};

class RequestQueuer: public wallaroo::Device, public IRequestQueuer {
public:
	RequestQueuer();
	~RequestQueuer();

	virtual bool addRequest(std::string requestString);
	virtual int getNumOfMessagess();
	virtual int getNumOfProcessors();

private:
	log4cpp::Category& logger;

	wallaroo::Plug<IRequestProcessor, wallaroo::collection> requestProcessors;

	std::mutex requestsMutex;
	std::priority_queue<Json::Value, std::vector<Json::Value>, RequestValueComparer> requests;
	volatile long lastSerial = 0;

	std::unique_ptr<std::thread> requestProcessorExecutorThread;
	std::condition_variable cv;
	volatile bool finishCycleThread = false;

	void requestProcessorExecutorThreadFunction();

	Json::Reader jsonReader;
};

} /* namespace processing */

#endif /* REQUESTQUEUER_HPP_ */
