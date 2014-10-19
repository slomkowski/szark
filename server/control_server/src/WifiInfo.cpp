#include <sys/ioctl.h>

#include <cstring>
#include <cmath>

#include "WifiInfo.hpp"

using namespace os;

WALLAROO_REGISTER(WifiInfo);

os::WifiInfo::WifiInfo(std::string iwName)
		: logger(log4cpp::Category::getInstance("WifiInfo")),
		  config("config", RegistrationToken()),
		  iwName(iwName) {
	Init();
}

os::WifiInfo::WifiInfo()
		: logger(log4cpp::Category::getInstance("WifiInfo")),
		  config("config", RegistrationToken()) {
}

void os::WifiInfo::Init() {
	if (iwName.length() == 0) {
		enabled = config->getBool("szark.WifiInfo.enabled");
		iwName = config->getString("szark.WifiInfo.device");
	}

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	logger.notice("opened wireless device: " + iwName);

	try {
		getSignalLevel();
		getBitrate();
	} catch (WifiException &e) {
		close(sockfd);
		throw WifiException(std::string("failed to open device ") + iwName);
	}
}

os::WifiInfo::~WifiInfo() {
	close(sockfd);
}

double os::WifiInfo::getSignalLevel() {
	prepareStructs();

	if (ioctl(sockfd, SIOCGIWSTATS, &req) == -1) {
		throw WifiException("failed to get signal level");
	}

	double sigLevel = -200.0;

	if (stats.qual.level != 0 or (stats.qual.updated & (IW_QUAL_DBM | IW_QUAL_RCPI))) {
		/*
		 * RCPI (IEEE 802.11k) statistics:
		 *    RCPI = int{(Power in dBm +110)*2}
		 *    for 0 dBm > Power > -110 dBm
		 */
		if (stats.qual.updated & IW_QUAL_RCPI) {
			if (not (stats.qual.updated & IW_QUAL_LEVEL_INVALID)) {
				sigLevel = (stats.qual.level / 2.0) - 110.0;
			}
		} else if (stats.qual.updated & IW_QUAL_DBM) {
			if (not (stats.qual.updated & IW_QUAL_LEVEL_INVALID)) {
				sigLevel = stats.qual.level > 63 ? stats.qual.level - 0x100 : stats.qual.level;
			}
		} else {
			/*
			 * Relative values (0 -> max)
			 */
			if (not (stats.qual.updated & IW_QUAL_LEVEL_INVALID)) {
				sigLevel = 10.0 * std::log10(stats.qual.level);
			}
		}
	} else {
		throw WifiException("signal level value invalid");
	}

	logger.info(std::string("current signal level: ") + std::to_string(sigLevel) + " dBm");

	return sigLevel;
}

double os::WifiInfo::getBitrate() {
	prepareStructs();

	if (ioctl(sockfd, SIOCGIWRATE, &req) == -1) {
		throw WifiException("failed to get bitrate");
	}

	double bitrate = req.u.bitrate.value / 1000000.0;

	logger.info(std::string("current bitrate: ") + std::to_string(bitrate) + " MBit/s");

	return bitrate;
}

void os::WifiInfo::prepareStructs() {
	if (not enabled) {
		throw WifiException("WifiInfo disabled");
	}

	std::strncpy(req.ifr_name, iwName.c_str(), IFNAMSIZ);
	req.u.data.pointer = &stats;
	req.u.data.length = sizeof(iw_statistics);
	req.u.data.flags = 1;

	bzero(&stats, sizeof(iw_statistics));
}

