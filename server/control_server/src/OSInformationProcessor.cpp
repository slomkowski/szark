#include "OSInformationProcessor.hpp"

#include <boost/format.hpp>

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

	try {
		auto linkParams = wifiInfo->getWifiLinkParams(address);

		response["wifi"]["s"] = linkParams.getSignalStrength();
		logger.info((boost::format("Wi-Fi params for %s: %2.0f dBm") % address.to_string() % linkParams.getSignalStrength()).str());
//TODO add isEnabled to WifiInfo
	} catch (WifiException &e) {
		logger.info(std::string("Error reading Wi-Fi information: ") + e.what());
		response["wifi"]["b"] = 0;
		response["wifi"]["s"] = 0;
	}
}
