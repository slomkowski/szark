#include "WifiInfo.hpp"

#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/attr.h>
#include <linux/nl80211.h>

#include <pthread.h>

#include <cmath>
#include <chrono>
#include <map>
#include <mutex>
#include <thread>
#include <functional>

#include <boost/format.hpp>

#include "utils.hpp"

constexpr std::chrono::milliseconds DUMP_STATION_INTERVAL(100);

using namespace os;
using namespace boost;
using namespace std;

WALLAROO_REGISTER(WifiInfo);

namespace os {
	struct WifiInfoImpl {
		bool enabled = true;

		string iwName;

		struct nl_sock *nl_sock;
		int nl80211_id;

		map<asio::ip::address, MacAddress> ipToMacMap;
		mutex ipToMacMapMutex;

		map<MacAddress, WifiLinkParams> wifiLinkParamsMap;
		mutex wifiLinkParamsMapMutex;

		unique_ptr<thread> acquireNetworkInformationThread;
		volatile bool acquireNetworkInformationThreadStop = false;

		log4cpp::Category *logger;
	};

}

os::MacAddress::MacAddress(char *addr) {
	memcpy(this->mac, addr, 6);
}

bool os::MacAddress::operator==(const MacAddress &other) const {
	return memcmp(this->mac, other.mac, 6) == 0;
}

bool os::MacAddress::operator<(const MacAddress &other) const {
	return memcmp(this->mac, other.mac, 6) < 0;
}

string os::MacAddress::toString() const {
	format fmt("%02x:%02x:%02x:%02x:%02x:%02x");

	for (int i = 0; i != 6; ++i) {
		fmt % static_cast<unsigned int>(mac[i]);
	}
	return fmt.str();
}

string os::WifiLinkParams::toString() const {
	return (format("%s: sig: %2.0f dBm, tx: %2.0f, Mb/s, rx: %2.0f Mb/s")
			% macAddress.toString() % signalStrength % txBitrate % rxBitrate).str();
}

os::WifiInfo::WifiInfo(string iwName)
		: logger(log4cpp::Category::getInstance("WifiInfo")),
		  config("config", RegistrationToken()),
		  impl(new WifiInfoImpl()) {
	impl->logger = &logger;
	impl->iwName = iwName;
	Init();
}

os::WifiInfo::WifiInfo()
		: logger(log4cpp::Category::getInstance("WifiInfo")),
		  config("config", RegistrationToken()),
		  impl(new WifiInfoImpl()) {
	impl->logger = &logger;
}

void os::WifiInfo::Init() {

	impl->nl_sock = nl_socket_alloc();
	if (not impl->nl_sock) {
		throw WifiException("failed to allocate netlink socket");
	}

	if (if_nametoindex(impl->iwName.c_str()) == 0) {
		throw WifiException(string("invalid interface name: ") + impl->iwName);
	}

	nl_socket_set_buffer_size(impl->nl_sock, 8192, 8192);

	if (genl_connect(impl->nl_sock)) {
		nl_socket_free(impl->nl_sock);
		throw WifiException("failed to connect to generic netlink");
	}

	impl->nl80211_id = genl_ctrl_resolve(impl->nl_sock, "nl80211");
	if (impl->nl80211_id < 0) {
		nl_socket_free(impl->nl_sock);
		throw WifiException("nl80211 not found");
	}

	impl->acquireNetworkInformationThread.reset(new thread(&WifiInfo::acquireNetworkInformationThreadFunction, this));
	int result = pthread_setname_np(impl->acquireNetworkInformationThread->native_handle(), "wifiInfo");
	if (result != 0) {
		logger.error((format("Cannot set thread name: %s.") % strerror(result)).str());
	}

	logger.notice("Instance created.");
}

os::WifiInfo::~WifiInfo() {
	impl->acquireNetworkInformationThreadStop = true;

	impl->acquireNetworkInformationThread->join();

	nl_socket_free(impl->nl_sock);

	delete impl;

	logger.notice("Instance destroyed.");
}

WifiLinkParams os::WifiInfo::getWifiLinkParams(asio::ip::address address) {
	unique_lock<mutex> iplk(impl->ipToMacMapMutex);

	if (impl->ipToMacMap.find(address) == impl->ipToMacMap.end()) {
		throw WifiException((format("could not find client with IP: %s") % address.to_string()).str());
	}

	MacAddress mac = impl->ipToMacMap.at(address);

	iplk.unlock();

	unique_lock<mutex> wplk(impl->wifiLinkParamsMapMutex);

	if (impl->wifiLinkParamsMap.find(mac) == impl->wifiLinkParamsMap.end()) {
		throw WifiException((format("could not find link parameters for client %s") % mac.toString()).str());
	}

	logger.info((format("Got link parameters for %s(%s).") % address.to_string() % mac.toString()).str());

	return impl->wifiLinkParamsMap.at(mac);
}


static double parseBitrate(struct nlattr *bitrate_attr) {
	struct nlattr *rinfo[NL80211_RATE_INFO_MAX + 1];
	static struct nla_policy rate_policy[NL80211_RATE_INFO_MAX + 1] = {
			[NL80211_RATE_INFO_BITRATE] = {NLA_U16},
			[NL80211_RATE_INFO_BITRATE32] = {NLA_U32},
	};

	if (nla_parse_nested(rinfo, NL80211_RATE_INFO_MAX, bitrate_attr, rate_policy)) {
		throw WifiException("failed to parse nested bitrate attributes");
	}

	if (rinfo[NL80211_RATE_INFO_BITRATE32]) {
		return 0.1 * nla_get_u32(rinfo[NL80211_RATE_INFO_BITRATE32]);
	} else if (rinfo[NL80211_RATE_INFO_BITRATE]) {
		return 0.1 * nla_get_u16(rinfo[NL80211_RATE_INFO_BITRATE]);
	} else {
		throw WifiException("could not read bitrate");
	}
}

static int dumpStationHandler(struct nl_msg *msg, void *arg) {
	auto impl = reinterpret_cast<WifiInfoImpl *>(arg);
	nlattr *tb[NL80211_ATTR_MAX + 1];
	genlmsghdr *gnlh = reinterpret_cast<genlmsghdr *>(nlmsg_data(nlmsg_hdr(msg)));
	nlattr *sinfo[NL80211_STA_INFO_MAX + 1];

	static nla_policy stats_policy[NL80211_STA_INFO_MAX + 1] = {
			[NL80211_STA_INFO_SIGNAL] = {NLA_U8},
			[NL80211_STA_INFO_SIGNAL_AVG] = {NLA_U8},
			[NL80211_STA_INFO_TX_BITRATE] = {NLA_NESTED},
			[NL80211_STA_INFO_RX_BITRATE] = {NLA_NESTED}
	};

	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), nullptr);

	if (!tb[NL80211_ATTR_STA_INFO]) {
		throw WifiException("sta stats missing");
	}

	if (nla_parse_nested(sinfo, NL80211_STA_INFO_MAX, tb[NL80211_ATTR_STA_INFO], stats_policy)) {
		throw WifiException("failed to parse nested attributes");
	}

	MacAddress macAddress(reinterpret_cast<char *>(nla_data(tb[NL80211_ATTR_MAC])));

	if (not sinfo[NL80211_STA_INFO_SIGNAL]) {
		throw WifiException("could not read signal strength");
	}

	if (not sinfo[NL80211_STA_INFO_TX_BITRATE]) {
		throw WifiException("could not read TX bitrate");
	}

	if (not sinfo[NL80211_STA_INFO_RX_BITRATE]) {
		throw WifiException("could not read RX bitrate");
	}

	if (not sinfo[NL80211_STA_INFO_SIGNAL_AVG]) {
		throw WifiException("could not read average signal strength");
	}

	double signal = (int8_t) nla_get_u8(sinfo[NL80211_STA_INFO_SIGNAL]);
	double signalAvg = (int8_t) nla_get_u8(sinfo[NL80211_STA_INFO_SIGNAL_AVG]);
	double txBitrate = parseBitrate(sinfo[NL80211_STA_INFO_TX_BITRATE]);
	double rxBitrate = parseBitrate(sinfo[NL80211_STA_INFO_RX_BITRATE]);

	char dev[30];
	if_indextoname(nla_get_u32(tb[NL80211_ATTR_IFINDEX]), dev);

	WifiLinkParams p(txBitrate, rxBitrate, signal, macAddress, string(dev));

	impl->logger->debug((format("Read client info: %s, avg: %2.0f dBm.") % p.toString() % signalAvg).str());

	return NL_SKIP;
}

static int errorHandler(struct sockaddr_nl *nla, struct nlmsgerr *err, void *arg) {
	int *ret = reinterpret_cast<int *>(arg);
	*ret = err->error;
	return NL_STOP;
}

static int finishHandler(struct nl_msg *msg, void *arg) {
	int *ret = reinterpret_cast<int *>(arg);
	*ret = 0;
	return NL_SKIP;
}

static int ackHandler(struct nl_msg *msg, void *arg) {
	int *ret = reinterpret_cast<int *>(arg);
	*ret = 0;
	return NL_STOP;
}

void os::WifiInfo::dumpStation() {
	int err;

	nl_msg *msg = nlmsg_alloc();
	if (!msg) {
		throw WifiException("failed to allocate netlink message");
	}

	struct nl_cb *cb = nl_cb_alloc(NL_CB_DEFAULT);
	struct nl_cb *s_cb = nl_cb_alloc(NL_CB_DEFAULT);

	if (!cb || !s_cb) {
		throw WifiException("failed to allocate netlink callbacks");
	}

	genlmsg_put(msg, 0, 0, impl->nl80211_id, 0, NLM_F_DUMP, NL80211_CMD_GET_STATION, 0);

	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, if_nametoindex(impl->iwName.c_str()));

	namespace ph = placeholders;
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, &dumpStationHandler, impl);

	nl_socket_set_cb(impl->nl_sock, s_cb);

	err = nl_send_auto_complete(impl->nl_sock, msg);

	if (err < 0) {
		nl_cb_put(cb);
		nlmsg_free(msg);
		throw WifiException("failed to execute station dump");
	}

	nl_cb_err(cb, NL_CB_CUSTOM, errorHandler, &err);
	nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, finishHandler, &err);
	nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ackHandler, &err);

	while (err > 0) {
		nl_recvmsgs(impl->nl_sock, cb);
	}

	return;

	nla_put_failure:
	throw WifiException((format("failed to execute station dump: %d") % err).str());
}

void os::WifiInfo::acquireNetworkInformationThreadFunction() {
	while (not impl->acquireNetworkInformationThreadStop) {
		using namespace chrono;

		auto m = common::utils::measureTime<microseconds>(bind(&WifiInfo::dumpStation, this));

		logger.debug((format("Station dump completed in %d us.") % m).str());

		this_thread::sleep_for(DUMP_STATION_INTERVAL);
	}
}
