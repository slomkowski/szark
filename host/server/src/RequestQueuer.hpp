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
#include <functional>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>

#include <log4cpp/Category.hh>
#include <wallaroo/registered.h>
#include <json/value.h>
#include <json/reader.h>
#include <json/writer.h>

#include "IRequestProcessor.hpp"

namespace processing {

typedef std::function<void(long, std::string, bool)> ResponseSender;
typedef std::pair<long, Json::Value> Request;

constexpr long INVALID_MESSAGE = -1;

class IRequestQueuer {
public:
	/**
	 * Adds the request to the queue.
	 * @param request text of the request in JSON format
	 * @return true if the request was added to the queue
	 */
	virtual long addRequest(std::string request) = 0;

	virtual int getNumOfMessagess() = 0;

	virtual int getNumOfProcessors() = 0;

	virtual void setResponseSender(ResponseSender s) = 0;

	virtual ~IRequestQueuer() = default;
};

class RequestValueComparer {
public:
	bool operator()(const Request &r1, const Request &r2) const {
		return r1.second["serial"].asInt() < r2.second["serial"].asInt();
	}
};

class RequestQueuer: public wallaroo::Device, public IRequestQueuer {
public:
	RequestQueuer();
	~RequestQueuer();

	virtual long addRequest(std::string requestString);
	virtual int getNumOfMessagess();
	virtual int getNumOfProcessors();

	void setResponseSender(ResponseSender s) {
		responseSender = s;
	}

private:
	log4cpp::Category& logger;

	wallaroo::Plug<IRequestProcessor, wallaroo::collection> requestProcessors;

	std::mutex requestsMutex;
	std::priority_queue<Request, std::vector<Request>, RequestValueComparer> requests;
	volatile long lastSerial = 0;

	std::unique_ptr<std::thread> requestProcessorExecutorThread;
	std::condition_variable cv;
	volatile bool finishCycleThread = false;

	Json::Reader jsonReader;
	Json::FastWriter jsonWriter;

	ResponseSender responseSender;

	void requestProcessorExecutorThreadFunction();

	long nextId();
};

} /* namespace processing */

#endif /* REQUESTQUEUER_HPP_ */
