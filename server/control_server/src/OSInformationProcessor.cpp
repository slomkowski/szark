#include "OSInformationProcessor.hpp"

using namespace os;

WALLAROO_REGISTER(OSInformationProcessor);

os::OSInformationProcessor::OSInformationProcessor()
		: logger(log4cpp::Category::getInstance("OSInformationProcessor")),
		  config("config", RegistrationToken()),
		  wifiInfo("wifiInfo", RegistrationToken()) {
	logger.notice("Instance created.");
}

os::OSInformationProcessor::~OSInformationProcessor() {
	logger.notice("Instance destroyed.");
}

void os::OSInformationProcessor::process(Json::Value &request,
		boost::asio::ip::address address,
		Json::Value &response) {
	logger.info("Processing request.");

	response["wifi"]["b"] = wifiInfo->getBitrate();
	response["wifi"]["s"] = wifiInfo->getSignalLevel();
}
