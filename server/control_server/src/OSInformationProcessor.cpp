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
                                         minijson::object_writer &response) {
    logger.info("Processing request.");

    try {
        auto linkParams = wifiInfo->getWifiLinkParams(address);

        auto wifiWriter = response.nested_object("wifi");
        wifiWriter.write("b", linkParams.getSignalStrength());
        wifiWriter.write("s", 0);
        wifiWriter.close();

        logger.info("Wi-Fi params for %s: %2.0f dBm",
                    address.to_string().c_str(), linkParams.getSignalStrength());
        //TODO add isEnabled to WifiInfo
    } catch (WifiException &e) {
        logger.info(std::string("Error reading Wi-Fi information: ") + e.what());

        auto wifiWriter = response.nested_object("wifi");
        wifiWriter.write("b", 0);
        wifiWriter.write("s", 0);
        wifiWriter.close();
    }
}
