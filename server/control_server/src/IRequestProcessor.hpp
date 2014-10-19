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
		virtual void process(Json::Value &request, Json::Value &response) = 0;

		virtual ~IRequestProcessor() = default;
	};

} /* namespace processing */

#endif /* IREQUESTPROCESSOR_HPP_ */
