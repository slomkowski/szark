#include <memory>
#include <chrono>
#include <thread>

#include <boost/test/unit_test.hpp>
#include <wallaroo/catalog.h>

#include "RequestQueuer.hpp"
#include "IRequestProcessor.hpp"

class RequestProcessorMock : public processing::IRequestProcessor, public wallaroo::Device {
public:
	virtual void process(Json::Value &request, boost::asio::ip::address address, Json::Value &response);
};

void RequestProcessorMock::process(Json::Value &request, boost::asio::ip::address address, Json::Value &response) {

	response["lights"]["led"] = false;
	response["lights"]["camera"] = true;

	std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

WALLAROO_REGISTER(RequestProcessorMock);

static std::string requestWithNoSerial =
		R"(
{
     "timestamp" : "12:34:56,123",
     "control" : "stop"
}
)";

static std::string validRequest =
		R"(
{
     "serial" : 55555555,
     "timestamp" : "02:34:56,123",
     "killswitch" : false,
     "motor" :
     {
	  "left" : { "speed" : 12, "direction" : "forward" },
	  "right" : { "speed" : 12, "direction" : "forward" }
     },
     "arm" :
     {
	  "wrist" : { "speed" : 12, "position" : 45 },
	  "gripper" : { "speed" : 12, "direction" : "backward" },
	  "elbow" : { "speed" : 12, "direction" : "backward" },
	  "shoulder" : { "speed" : 12, "direction" : "stop" }
     },
     "lights" :
     {
	  "led" : true,
	  "camera" : false
     }
}
)";

static std::string validRequestWithLowerSerial =
		R"(
{
     "serial" : 11111111,
     "timestamp" : "12:34:56,123",
     "control" : "stop",
     "motors" :
     {
	  "left" : { "speed" : 12, "direction" : "forward" },
	  "right" : { "speed" : 12, "direction" : "forward" }
     }
})";

static std::string validRequestWithZeroSerial =
		R"(
{
     "serial" : 0,
     "timestamp" : "12:34:56,123",
     "control" : "stop"
})";

static std::string validRequestWithHigherSerial =
		R"(
{
     "serial" : 55555556,
     "timestamp" : "12:34:56,123",
     "control" : "stop",
     "motors" :
     {
	  "left" : { "speed" : 12, "direction" : "forward" },
	  "right" : { "speed" : 12, "direction" : "forward" }
     }
})";

static std::shared_ptr<processing::RequestQueuer> prepareBindings(wallaroo::Catalog &catalog) {
	catalog.Create("rq", "RequestQueuer");
	catalog.Create("mock1", "RequestProcessorMock");
	wallaroo::use(catalog["mock1"]).as("requestProcessors").of(catalog["rq"]);

	catalog.CheckWiring();

	return catalog["rq"];
}

BOOST_AUTO_TEST_CASE(RequestQueuerTest_addRequest) {
	wallaroo::Catalog catalog;
	auto rq = prepareBindings(catalog);

	BOOST_CHECK_EQUAL(rq->getNumOfProcessors(), 1);

	std::pair<std::string, bool> tests[] = {
			std::make_pair("invalid string", false),
			std::make_pair(validRequest, true),
			std::make_pair(requestWithNoSerial, false),
			std::make_pair(validRequest, false),
			std::make_pair(validRequestWithLowerSerial, false),
			std::make_pair(validRequestWithHigherSerial, true),
			std::make_pair(validRequestWithHigherSerial, false),

			std::make_pair(validRequestWithZeroSerial, true),
			std::make_pair(validRequestWithZeroSerial, true),
			std::make_pair(validRequestWithZeroSerial, true),

			std::make_pair(requestWithNoSerial, false),
			std::make_pair(validRequest, true),
			std::make_pair(validRequestWithLowerSerial, false),
			std::make_pair(validRequestWithHigherSerial, true),
			std::make_pair(validRequestWithHigherSerial, false)
	};

	for (auto t : tests) {
		BOOST_CHECK_EQUAL(rq->addRequest(t.first, boost::asio::ip::address()), t.second);
		std::this_thread::sleep_for(std::chrono::milliseconds(15));
	}

	catalog.Create("mock2", "RequestProcessorMock");
	wallaroo::use(catalog["mock2"]).as("requestProcessors").of(catalog["rq"]);

	catalog.CheckWiring();

	BOOST_CHECK_EQUAL(rq->getNumOfProcessors(), 2);
}

BOOST_AUTO_TEST_CASE(RequestQueuerTest_requestProcessor) {
	wallaroo::Catalog catalog;
	auto rq = prepareBindings(catalog);

	Json::Value req;
	Json::Value resp;

	req["serial"] = 123;

	BOOST_CHECK_EQUAL(req["serial"].asInt(), resp["serial"].asInt());
}
