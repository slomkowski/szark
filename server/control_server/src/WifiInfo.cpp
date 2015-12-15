#include "WifiInfo.hpp"
#include "utils.hpp"

#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/attr.h>
#include <netlink/route/neighbour.h>
#include <linux/nl80211.h>
#include <pthread.h>

#include <boost/format.hpp>

#include <cmath>
#include <chrono>
#include <map>
#include <mutex>
#include <thread>
#include <functional>
#include <unordered_map>

constexpr std::chrono::milliseconds DUMP_STATION_INTERVAL(100);

using namespace os;
using namespace boost;
using namespace std;

WALLAROO_REGISTER(WifiInfo);

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

const static std::map<nl80211_iftype, std::string> IFACE_TYPE_NAMES = {
        {NL80211_IFTYPE_ADHOC,       "ad-hoc"},
        {NL80211_IFTYPE_STATION,     "managed"},
        {NL80211_IFTYPE_AP,          "AP"},
        {NL80211_IFTYPE_UNSPECIFIED, "unspecified"}
};

namespace os {
    struct WifiInfoImpl {
        bool enabled = true;

        string iwName;

        // wifi info
        struct nl_sock *wifiSock;
        int nl80211_id;

        // arp
        struct nl_sock *arpSock;
        struct rtnl_neigh *neigh;
        struct nl_cache *neigh_cache;

        map<asio::ip::address, MacAddress> ipToMacMap;
        mutex ipToMacMapMutex;

        map<MacAddress, WifiLinkParams> wifiLinkParamsMap;
        mutex wifiLinkParamsMapMutex;

        unique_ptr<thread> acquireNetworkInformationThread;
        volatile bool acquireNetworkInformationThreadStop = false;

        log4cpp::Category *logger;

        volatile nl80211_iftype interfaceType = NL80211_IFTYPE_UNSPECIFIED;
    };

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

    if (impl->iwName.length() == 0) {
        impl->iwName = config->getString("WifiInfo.device");
        impl->enabled = config->getBool("WifiInfo.enabled");
    }

    if (not impl->enabled) {
        logger.warn("Monitoring is disabled. Will throw when at read.");
        return;
    }

    impl->wifiSock = nl_socket_alloc();
    if (not impl->wifiSock) {
        throw WifiException("failed to allocate netlink wifi socket");
    }

    if (if_nametoindex(impl->iwName.c_str()) == 0) {
        throw WifiException(string("invalid interface name: ") + impl->iwName);
    }

    nl_socket_set_buffer_size(impl->wifiSock, 8192, 8192);

    if (genl_connect(impl->wifiSock)) {
        nl_socket_free(impl->wifiSock);
        throw WifiException("failed to connect to generic netlink");
    }

    impl->nl80211_id = genl_ctrl_resolve(impl->wifiSock, "nl80211");
    if (impl->nl80211_id < 0) {
        nl_socket_free(impl->wifiSock);
        throw WifiException("nl80211 not found");
    }

    impl->arpSock = nl_socket_alloc();
    if (not impl->arpSock) {
        throw WifiException("failed to allocate netlink arp socket");
    }

    if (nl_connect(impl->arpSock, NETLINK_ROUTE) < 0) {
        throw WifiException("failed connect netlink arp socket");
    }

    if (rtnl_neigh_alloc_cache(impl->arpSock, &(impl->neigh_cache)) < 0) {
        throw WifiException("failed alloc neighbour cache");
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

    if (impl->acquireNetworkInformationThread.get() != nullptr) {
        impl->acquireNetworkInformationThread->join();

        nl_socket_free(impl->wifiSock);
    }
    delete impl;

    logger.notice("Instance destroyed.");
}

WifiLinkParams os::WifiInfo::getWifiLinkParams(asio::ip::address address) {
    if (not impl->enabled) {
        throw WifiException("WifiInfo is disabled");
    }

    if (impl->interfaceType == NL80211_IFTYPE_STATION) {
        // in managed mode there's only one station (the AP) we return the parameters for it

        unique_lock<mutex> wplk(impl->wifiLinkParamsMapMutex);

        if (impl->wifiLinkParamsMap.size() == 0) {
            throw WifiException("wifiLinkParamsMap is empty");
        }

        auto params = impl->wifiLinkParamsMap.begin();

        logger.info((format("Got link parameters for access point (%s).") % params->first.toString()).str());

        return params->second;

    } else {
        // in case of Ad-Hoc mode, we return statistics for the peer

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
}


static double parseBitrate(struct nlattr *bitrate_attr) {
    struct nlattr *rinfo[NL80211_RATE_INFO_MAX + 1];
    static struct nla_policy rate_policy[NL80211_RATE_INFO_MAX + 1];
    rate_policy[NL80211_RATE_INFO_BITRATE] = {NLA_U16};
    rate_policy[NL80211_RATE_INFO_BITRATE32] = {NLA_U32};

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

    static nla_policy stats_policy[NL80211_STA_INFO_MAX + 1];
    stats_policy[NL80211_STA_INFO_SIGNAL] = {NLA_U8};
    stats_policy[NL80211_STA_INFO_SIGNAL_AVG] = {NLA_U8};
    stats_policy[NL80211_STA_INFO_TX_BITRATE] = {NLA_NESTED};
    stats_policy[NL80211_STA_INFO_RX_BITRATE] = {NLA_NESTED};

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

    impl->logger->info((format("Read client info: %s, avg: %2.0f dBm.") % p.toString() % signalAvg).str());

    unique_lock<mutex> iplk(impl->wifiLinkParamsMapMutex);

    if (impl->wifiLinkParamsMap.find(macAddress) == impl->wifiLinkParamsMap.end()) {
        impl->logger->notice("New station added: " + macAddress.toString() + ".");
    }

    impl->wifiLinkParamsMap.erase(macAddress);
    impl->wifiLinkParamsMap.insert(make_pair(macAddress, p));

    return NL_SKIP;
}

static int dumpCardInfoHandler(struct nl_msg *msg, void *arg) {
    auto impl = reinterpret_cast<WifiInfoImpl *>(arg);
    nlattr *tb[NL80211_ATTR_MAX + 1];
    genlmsghdr *gnlh = reinterpret_cast<genlmsghdr *>(nlmsg_data(nlmsg_hdr(msg)));

    nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), nullptr);

    std::stringstream report;

    report << "Read interface info: ";

    if (tb[NL80211_ATTR_IFNAME]) {
        report << nla_get_string(tb[NL80211_ATTR_IFNAME]);
    }

    if (tb[NL80211_ATTR_MAC]) {
        MacAddress macAddress((char *) nla_data(tb[NL80211_ATTR_MAC]));
        report << " (MAC: " << macAddress.toString() << ")";
    }

    if (tb[NL80211_ATTR_SSID]) {
        report << ", SSID: " << nla_data(tb[NL80211_ATTR_SSID]);
    }

    if (tb[NL80211_ATTR_IFTYPE]) {
        nl80211_iftype type = static_cast<nl80211_iftype>(nla_get_u32(tb[NL80211_ATTR_IFTYPE]));

        impl->interfaceType = type;

        report << ", mode: " << IFACE_TYPE_NAMES.at(type);
    }

    report << ".";

    impl->logger->info(report.str());

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

void os::WifiInfo::dumpCardAttributes() {
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

    genlmsg_put(msg, 0, 0, impl->nl80211_id, 0, 0, NL80211_CMD_GET_INTERFACE, 0);

    NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, if_nametoindex(impl->iwName.c_str()));

    namespace ph = placeholders;
    nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, &dumpCardInfoHandler, impl);

    nl_socket_set_cb(impl->wifiSock, s_cb);

    err = nl_send_auto(impl->wifiSock, msg);

    if (err < 0) {
        nl_cb_put(cb);
        nlmsg_free(msg);
        throw WifiException("failed to execute station dump");
    }

    nl_cb_err(cb, NL_CB_CUSTOM, errorHandler, &err);
    nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, finishHandler, &err);
    nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ackHandler, &err);

    while (err > 0) {
        nl_recvmsgs(impl->wifiSock, cb);
    }

    nlmsg_free(msg);

    return;

    nla_put_failure:
    throw WifiException((format("failed to execute station dump: %d") % err).str());
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

    nl_socket_set_cb(impl->wifiSock, s_cb);

    err = nl_send_auto(impl->wifiSock, msg);

    if (err < 0) {
        nl_cb_put(cb);
        nlmsg_free(msg);
        throw WifiException("failed to execute station dump");
    }

    nl_cb_err(cb, NL_CB_CUSTOM, errorHandler, &err);
    nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, finishHandler, &err);
    nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ackHandler, &err);

    while (err > 0) {
        nl_recvmsgs(impl->wifiSock, cb);
    }

    nlmsg_free(msg);

    return;

    nla_put_failure:
    throw WifiException((format("failed to execute station dump: %d") % err).str());
}

void os::WifiInfo::acquireNetworkInformationThreadFunction() {
    while (not impl->acquireNetworkInformationThreadStop) {
        using namespace chrono;

        int m = common::utils::measureTime<microseconds>(bind(&WifiInfo::dumpCardAttributes, this));

        logger.debug((format("Card attributes dump completed in %d us.") % m).str());

        m = common::utils::measureTime<microseconds>(bind(&WifiInfo::dumpArp, this));

        logger.debug((format("ARP table filled in %d us.") % m).str());

        m = common::utils::measureTime<microseconds>(bind(&WifiInfo::dumpStation, this));

        logger.debug((format("Station dump completed in %d us.") % m).str());

        this_thread::sleep_for(DUMP_STATION_INTERVAL);
    }
}

static void arpTableDumpHanlder(struct nl_object *obj, void *arg) {
    auto impl = reinterpret_cast<WifiInfoImpl *>(arg);
    auto neigh = reinterpret_cast<rtnl_neigh *>(obj);

    nl_addr *macRawAddr = rtnl_neigh_get_lladdr(neigh);
    nl_addr *ipRawAddr = rtnl_neigh_get_dst(neigh);

    if (macRawAddr == nullptr or ipRawAddr == nullptr) {
        return;
    }

    if (nl_addr_get_family(ipRawAddr) != AF_INET) {
        return;
    }

    if (nl_addr_get_len(ipRawAddr) != 4) {
        throw WifiException("returned IP address has invalid length");
    }

    if (nl_addr_get_len(macRawAddr) != 6) {
        throw WifiException("returned MAC address has invalid length");
    }

    asio::ip::address_v4::bytes_type addrBytes;
    for (int i = 0; i < 4; i++) {
        addrBytes[i] = reinterpret_cast<char *>(nl_addr_get_binary_addr(ipRawAddr))[i];
    }
    asio::ip::address_v4 ip(addrBytes);

    MacAddress mac(reinterpret_cast<char *>(nl_addr_get_binary_addr(macRawAddr)));

    impl->logger->info((format("Got ARP table entry: %s -> %s.") % ip.to_string() % mac.toString()).str());

    unique_lock<mutex> iplk(impl->ipToMacMapMutex);
    if (impl->ipToMacMap.find(ip) == impl->ipToMacMap.end()) {
        impl->logger->notice("New IP address found: " + ip.to_string() + ".");
    }
    impl->ipToMacMap.erase(ip);
    impl->ipToMacMap.insert(make_pair(ip, mac));

}

void os::WifiInfo::dumpArp() {

    if (nl_cache_refill(impl->arpSock, impl->neigh_cache) < 0) {
        throw WifiException("nf_cache_fill");
    }

    nl_cache_foreach(impl->neigh_cache, arpTableDumpHanlder, impl);
}