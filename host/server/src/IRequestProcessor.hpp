/*
 * IRequestProcessor.hpp
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
#ifndef IREQUESTPROCESSOR_HPP_
#define IREQUESTPROCESSOR_HPP_

#include <json/value.h>

namespace processing {

class IRequestProcessor {
public:
	/*
	 * Takes the JSON document tree and parses the request. Returns the response.
	 * The function is blocking till the values are gathered.
	 */
	virtual Json::Value process(Json::Value request) = 0;

	virtual ~IRequestProcessor() = default;
};

} /* namespace processing */

#endif /* IREQUESTPROCESSOR_HPP_ */
