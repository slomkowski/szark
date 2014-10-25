#include "WifiInfo.hpp"

#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/attr.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/attr.h>

#include <linux/nl80211.h>

#include <cmath>
#include <chrono>
#include <map>
#include <mutex>
#include <thread>
#include <functional>

#include <boost/format.hpp>

#include "utils.hpp"

constexpr std::chrono::milliseconds DUMP_STATION_INTERVAL(0);

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

		unique_ptr<thread> stationDumpThread;
		volatile bool stationDumpThreadStop = false;

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
	return (format("%s: sig: %2.0f  dBm, tx: %2.0f, Mb/s, rx: %2.0f Mb/s")
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

	impl->stationDumpThread.reset(new thread(&WifiInfo::stationDumpThreadFunction, this));

	logger.notice("Instance created.");
}

os::WifiInfo::~WifiInfo() {
	impl->stationDumpThreadStop = true;

	impl->stationDumpThread->join();

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


static double parse_bitrate(struct nlattr *bitrate_attr) {
	struct nlattr *rinfo[NL80211_RATE_INFO_MAX + 1];
	static struct nla_policy rate_policy[NL80211_RATE_INFO_MAX + 1] = {
			[NL80211_RATE_INFO_BITRATE] = {.type = NLA_U16},
			[NL80211_RATE_INFO_BITRATE32] = {.type = NLA_U32},
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

	static struct nla_policy stats_policy[NL80211_STA_INFO_MAX + 1] = {
			[NL80211_STA_INFO_INACTIVE_TIME] = {.type = NLA_U32},
			[NL80211_STA_INFO_RX_BYTES] = {.type = NLA_U32},
			[NL80211_STA_INFO_TX_BYTES] = {.type = NLA_U32},
			[NL80211_STA_INFO_RX_PACKETS] = {.type = NLA_U32},
			[NL80211_STA_INFO_TX_PACKETS] = {.type = NLA_U32},
			[NL80211_STA_INFO_SIGNAL] = {.type = NLA_U8},
			[NL80211_STA_INFO_T_OFFSET] = {.type = NLA_U64},
			[NL80211_STA_INFO_TX_BITRATE] = {.type = NLA_NESTED},
			[NL80211_STA_INFO_RX_BITRATE] = {.type = NLA_NESTED},
			[NL80211_STA_INFO_LLID] = {.type = NLA_U16},
			[NL80211_STA_INFO_PLID] = {.type = NLA_U16},
			[NL80211_STA_INFO_PLINK_STATE] = {.type = NLA_U8},
			[NL80211_STA_INFO_TX_RETRIES] = {.type = NLA_U32},
			[NL80211_STA_INFO_TX_FAILED] = {.type = NLA_U32},
			[NL80211_STA_INFO_STA_FLAGS] = {.minlen = sizeof(struct nl80211_sta_flag_update)},
			[NL80211_STA_INFO_LOCAL_PM] = {.type = NLA_U32},
			[NL80211_STA_INFO_PEER_PM] = {.type = NLA_U32},
			[NL80211_STA_INFO_NONPEER_PM] = {.type = NLA_U32},
			[NL80211_STA_INFO_CHAIN_SIGNAL] = {.type = NLA_NESTED},
			[NL80211_STA_INFO_CHAIN_SIGNAL_AVG] = {.type = NLA_NESTED},
	};

	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), nullptr);

	if (!tb[NL80211_ATTR_STA_INFO]) {
		throw WifiException("sta stats missing");
	}

	if (
			nla_parse_nested(sinfo, NL80211_STA_INFO_MAX, tb[NL80211_ATTR_STA_INFO], stats_policy
			)) {
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
	double txBitrate = parse_bitrate(sinfo[NL80211_STA_INFO_TX_BITRATE]);
	double rxBitrate = parse_bitrate(sinfo[NL80211_STA_INFO_RX_BITRATE]);

	char dev[30];
	if_indextoname(nla_get_u32(tb[NL80211_ATTR_IFINDEX]), dev);

	WifiLinkParams p(txBitrate, rxBitrate, signal, macAddress, string(dev));

	impl->logger->debug((format("Read client info: %s, avg: %2.0f dBm.") % p.toString() % signalAvg).str());

	return NL_SKIP;
}

static int error_handler(struct sockaddr_nl *nla, struct nlmsgerr *err, void *arg) {
	int *ret = reinterpret_cast<int *>(arg);
	*ret = err->error;
	return NL_STOP;
}

static int finish_handler(struct nl_msg *msg, void *arg) {
	int *ret = reinterpret_cast<int *>(arg);
	*ret = 0;
	return NL_SKIP;
}

static int ack_handler(struct nl_msg *msg, void *arg) {
	int *ret = reinterpret_cast<int *>(arg);
	*ret = 0;
	return NL_STOP;
}

void os::WifiInfo::stationDumpThreadFunction() {
	while (not impl->stationDumpThreadStop) {
		using namespace chrono;

		auto m = common::utils::measureTime<microseconds>([this]() {
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

			nl_cb_err(cb, NL_CB_CUSTOM, error_handler, &err);
			nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &err);
			nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &err);

			while (err > 0) {
				nl_recvmsgs(impl->nl_sock, cb);
			}

			nla_put_failure:
			throw WifiException((format("failed to execute station dump: %d") % err).str());
		});

		logger.debug((format("Station dump completed in %d us.") % m).str());

		this_thread::sleep_for(DUMP_STATION_INTERVAL);
	}
}
