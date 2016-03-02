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

void OSInformationProcessor::process(processing::Request &req, minijson::object_writer &response) {

    logger.info("Processing request.");

    try {
        auto linkParams = wifiInfo->getWifiLinkParams(req.ipAddress);

        auto wifiWriter = response.nested_object("w");
        wifiWriter.write("s", linkParams.getSignalStrength());
        wifiWriter.write("txb", linkParams.getTxBitrate());
        wifiWriter.write("rxb", linkParams.getRxBitrate());
        wifiWriter.close();

        logger.info("Wi-Fi params for %s: %2.0f dBm",
                    req.ipAddress.to_string().c_str(), linkParams.getSignalStrength());
        //TODO add isEnabled to WifiInfo
    } catch (WifiException &e) {
        logger.info("Error reading Wi-Fi information: %s.", e.what());

        auto wifiWriter = response.nested_object("w");
        wifiWriter.write("txb", 0);
        wifiWriter.write("rxb", 0);
        wifiWriter.write("s", 0);
        wifiWriter.close();
    }
}

