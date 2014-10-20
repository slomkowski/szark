#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/wireless.h>

#include <cstring>
#include <cmath>

#include "WifiInfo.hpp"

using namespace os;

WALLAROO_REGISTER(WifiInfo);

namespace os {
	struct WifiInfoImpl {
		int sockfd;
		bool enabled = true;
		iw_statistics stats;
		iwreq req;

		std::string iwName;
	};
}

os::WifiInfo::WifiInfo(std::string iwName)
		: logger(log4cpp::Category::getInstance("WifiInfo")),
		  config("config", RegistrationToken()),
		  impl(new WifiInfoImpl()) {
	impl->iwName = iwName;
	Init();
}

os::WifiInfo::WifiInfo()
		: logger(log4cpp::Category::getInstance("WifiInfo")),
		  config("config", RegistrationToken()),
		  impl(new WifiInfoImpl()) {
}

void os::WifiInfo::Init() {
	if (impl->iwName.length() == 0) {
		impl->enabled = config->getBool("WifiInfo.enabled");
		impl->iwName = config->getString("WifiInfo.device");
	}

	if (impl->enabled) {
		impl->sockfd = socket(AF_INET, SOCK_DGRAM, 0);

		logger.notice("Opened wireless device: " + impl->iwName + ".");

		try {
			getSignalLevel();
			getBitrate();
		} catch (WifiException &e) {
			close(impl->sockfd);
			throw WifiException(std::string("failed to open device ") + impl->iwName);
		}
	} else {
		logger.warn("Monitoring is disabled. Will return dummy values.");
	}

	logger.notice("Instance created.");
}

os::WifiInfo::~WifiInfo() {
	close(impl->sockfd);
	delete impl;
	logger.notice("Instance destroyed.");
}

double os::WifiInfo::getSignalLevel() {
	if (not impl->enabled) {
		return 0;
	}

	prepareStructs();

	if (ioctl(impl->sockfd, SIOCGIWSTATS, &(impl->req)) == -1) {
		throw WifiException("failed to get signal level");
	}

	double sigLevel = -200.0;

	if (impl->stats.qual.level != 0 or (impl->stats.qual.updated & (IW_QUAL_DBM | IW_QUAL_RCPI))) {
		/*
		 * RCPI (IEEE 802.11k) statistics:
		 *    RCPI = int{(Power in dBm +110)*2}
		 *    for 0 dBm > Power > -110 dBm
		 */
		if (impl->stats.qual.updated & IW_QUAL_RCPI) {
			if (not (impl->stats.qual.updated & IW_QUAL_LEVEL_INVALID)) {
				sigLevel = (impl->stats.qual.level / 2.0) - 110.0;
			}
		} else if (impl->stats.qual.updated & IW_QUAL_DBM) {
			if (not (impl->stats.qual.updated & IW_QUAL_LEVEL_INVALID)) {
				sigLevel = impl->stats.qual.level > 63 ? impl->stats.qual.level - 0x100 : impl->stats.qual.level;
			}
		} else {
			/*
			 * Relative values (0 -> max)
			 */
			if (not (impl->stats.qual.updated & IW_QUAL_LEVEL_INVALID)) {
				sigLevel = 10.0 * std::log10(impl->stats.qual.level);
			}
		}
	} else {
		throw WifiException("signal level value invalid");
	}

	logger.info(std::string("Current signal level: ") + std::to_string(sigLevel) + " dBm.");

	return sigLevel;
}

std::string os::WifiInfo::getInterfaceName() {
	return impl->iwName;
}

double os::WifiInfo::getBitrate() {
	if (not impl->enabled) {
		return 0;
	}

	prepareStructs();

	if (ioctl(impl->sockfd, SIOCGIWRATE, &impl->req) == -1) {
		throw WifiException("failed to get bitrate");
	}

	double bitrate = impl->req.u.bitrate.value / 1000000.0;

	logger.info(std::string("Current bitrate: ") + std::to_string(bitrate) + " MBit/s.");

	return bitrate;
}

void os::WifiInfo::prepareStructs() {
	std::strncpy(impl->req.ifr_name, impl->iwName.c_str(), IFNAMSIZ);
	impl->req.u.data.pointer = &impl->stats;
	impl->req.u.data.length = sizeof(iw_statistics);
	impl->req.u.data.flags = 1;

	bzero(&impl->stats, sizeof(iw_statistics));
}

