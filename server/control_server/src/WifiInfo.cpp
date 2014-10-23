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
#include <boost/format.hpp>

using namespace os;
using namespace boost;

WALLAROO_REGISTER(WifiInfo);

namespace os {
	struct WifiInfoImpl {
		bool enabled = true;
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

	logger.notice("Instance created.");
}

os::WifiInfo::~WifiInfo() {
	delete impl;
	logger.notice("Instance destroyed.");
}

/*static char *get_chain_signal(struct nlattr *attr_list) {
	struct nlattr *attr;
	static char buf[64];
	char *cur = buf;
	int i = 0, rem;
	const char *prefix;

	if (!attr_list)
		return "";

	nla_for_each_nested(reinterpret_cast<void*>(attr), attr_list, rem) {
		if (i++ > 0)
			prefix = ", ";
		else
			prefix = "[";

		cur += snprintf(cur, sizeof(buf) - (cur - buf), "%s%d", prefix,
				(int8_t) nla_get_u8(attr));
	}

	if (i)
		snprintf(cur, sizeof(buf) - (cur - buf), "] ");

	return buf;
}*/

void parse_bitrate(struct nlattr *bitrate_attr, char *buf, int buflen) {
	int rate = 0;
	char *pos = buf;
	struct nlattr *rinfo[NL80211_RATE_INFO_MAX + 1];
	static struct nla_policy rate_policy[NL80211_RATE_INFO_MAX + 1] = {
			[NL80211_RATE_INFO_BITRATE] = {.type = NLA_U16},
			[NL80211_RATE_INFO_BITRATE32] = {.type = NLA_U32},
			[NL80211_RATE_INFO_MCS] = {.type = NLA_U8},
			[NL80211_RATE_INFO_40_MHZ_WIDTH] = {.type = NLA_FLAG},
			[NL80211_RATE_INFO_SHORT_GI] = {.type = NLA_FLAG},
	};

	if (nla_parse_nested(rinfo, NL80211_RATE_INFO_MAX,
			bitrate_attr, rate_policy)) {
		snprintf(buf, buflen, "failed to parse nested rate attributes!");
		return;
	}

	if (rinfo[NL80211_RATE_INFO_BITRATE32])
		rate = nla_get_u32(rinfo[NL80211_RATE_INFO_BITRATE32]);
	else if (rinfo[NL80211_RATE_INFO_BITRATE])
		rate = nla_get_u16(rinfo[NL80211_RATE_INFO_BITRATE]);
	if (rate > 0)
		pos += snprintf(pos, buflen - (pos - buf),
				"%d.%d MBit/s", rate / 10, rate % 10);

	if (rinfo[NL80211_RATE_INFO_MCS])
		pos += snprintf(pos, buflen - (pos - buf),
				" MCS %d", nla_get_u8(rinfo[NL80211_RATE_INFO_MCS]));
	if (rinfo[NL80211_RATE_INFO_VHT_MCS])
		pos += snprintf(pos, buflen - (pos - buf),
				" VHT-MCS %d", nla_get_u8(rinfo[NL80211_RATE_INFO_VHT_MCS]));
	if (rinfo[NL80211_RATE_INFO_40_MHZ_WIDTH])
		pos += snprintf(pos, buflen - (pos - buf), " 40MHz");
	if (rinfo[NL80211_RATE_INFO_80_MHZ_WIDTH])
		pos += snprintf(pos, buflen - (pos - buf), " 80MHz");
	if (rinfo[NL80211_RATE_INFO_80P80_MHZ_WIDTH])
		pos += snprintf(pos, buflen - (pos - buf), " 80P80MHz");
	if (rinfo[NL80211_RATE_INFO_160_MHZ_WIDTH])
		pos += snprintf(pos, buflen - (pos - buf), " 160MHz");
	if (rinfo[NL80211_RATE_INFO_SHORT_GI])
		pos += snprintf(pos, buflen - (pos - buf), " short GI");
	if (rinfo[NL80211_RATE_INFO_VHT_NSS])
		pos += snprintf(pos, buflen - (pos - buf),
				" VHT-NSS %d", nla_get_u8(rinfo[NL80211_RATE_INFO_VHT_NSS]));
}

static int print_sta_handler(struct nl_msg *msg, void *arg) {

	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = reinterpret_cast<genlmsghdr *>(nlmsg_data(nlmsg_hdr(msg)));
	struct nlattr *sinfo[NL80211_STA_INFO_MAX + 1];
	char mac_addr[20], state_name[10], dev[20];
	struct nl80211_sta_flag_update *sta_flags;
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
			[NL80211_STA_INFO_STA_FLAGS] =
					{.minlen = sizeof(struct nl80211_sta_flag_update)},
			[NL80211_STA_INFO_LOCAL_PM] = {.type = NLA_U32},
			[NL80211_STA_INFO_PEER_PM] = {.type = NLA_U32},
			[NL80211_STA_INFO_NONPEER_PM] = {.type = NLA_U32},
			[NL80211_STA_INFO_CHAIN_SIGNAL] = {.type = NLA_NESTED},
			[NL80211_STA_INFO_CHAIN_SIGNAL_AVG] = {.type = NLA_NESTED},
	};
	char *chain;

	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			genlmsg_attrlen(gnlh, 0), NULL);

	/*
	 * TODO: validate the interface and mac address!
	 * Otherwise, there's a race condition as soon as
	 * the kernel starts sending station notifications.
	 */

	if (!tb[NL80211_ATTR_STA_INFO]) {
		fprintf(stderr, "sta stats missing!\n");
		return NL_SKIP;
	}
	if (nla_parse_nested(sinfo, NL80211_STA_INFO_MAX,
			tb[NL80211_ATTR_STA_INFO],
			stats_policy)) {
		fprintf(stderr, "failed to parse nested attributes!\n");
		return NL_SKIP;
	}

	/*mac_addr_n2a(mac_addr, nla_data(tb[NL80211_ATTR_MAC]));
	if_indextoname(nla_get_u32(tb[NL80211_ATTR_IFINDEX]), dev);
	printf("Station %s (on %s)", mac_addr, dev);*/

	if (sinfo[NL80211_STA_INFO_INACTIVE_TIME])
		printf("\n\tinactive time:\t%u ms",
				nla_get_u32(sinfo[NL80211_STA_INFO_INACTIVE_TIME]));
	if (sinfo[NL80211_STA_INFO_RX_BYTES])
		printf("\n\trx bytes:\t%u",
				nla_get_u32(sinfo[NL80211_STA_INFO_RX_BYTES]));
	if (sinfo[NL80211_STA_INFO_RX_PACKETS])
		printf("\n\trx packets:\t%u",
				nla_get_u32(sinfo[NL80211_STA_INFO_RX_PACKETS]));
	if (sinfo[NL80211_STA_INFO_TX_BYTES])
		printf("\n\ttx bytes:\t%u",
				nla_get_u32(sinfo[NL80211_STA_INFO_TX_BYTES]));
	if (sinfo[NL80211_STA_INFO_TX_PACKETS])
		printf("\n\ttx packets:\t%u",
				nla_get_u32(sinfo[NL80211_STA_INFO_TX_PACKETS]));
	if (sinfo[NL80211_STA_INFO_TX_RETRIES])
		printf("\n\ttx retries:\t%u",
				nla_get_u32(sinfo[NL80211_STA_INFO_TX_RETRIES]));
	if (sinfo[NL80211_STA_INFO_TX_FAILED])
		printf("\n\ttx failed:\t%u",
				nla_get_u32(sinfo[NL80211_STA_INFO_TX_FAILED]));

	/*chain = get_chain_signal(sinfo[NL80211_STA_INFO_CHAIN_SIGNAL]);
	if (sinfo[NL80211_STA_INFO_SIGNAL])
		printf("\n\tsignal:  \t%d %sdBm",
				(int8_t)nla_get_u8(sinfo[NL80211_STA_INFO_SIGNAL]),
				chain);

	chain = get_chain_signal(sinfo[NL80211_STA_INFO_CHAIN_SIGNAL_AVG]);
	if (sinfo[NL80211_STA_INFO_SIGNAL_AVG])
		printf("\n\tsignal avg:\t%d %sdBm",
				(int8_t)nla_get_u8(sinfo[NL80211_STA_INFO_SIGNAL_AVG]),
				chain);*/

	if (sinfo[NL80211_STA_INFO_T_OFFSET])
		printf("\n\tToffset:\t%lld us",
				(unsigned long long) nla_get_u64(sinfo[NL80211_STA_INFO_T_OFFSET]));

	if (sinfo[NL80211_STA_INFO_TX_BITRATE]) {
		char buf[100];

		parse_bitrate(sinfo[NL80211_STA_INFO_TX_BITRATE], buf, sizeof(buf));
		printf("\n\ttx bitrate:\t%s", buf);
	}

	if (sinfo[NL80211_STA_INFO_RX_BITRATE]) {
		char buf[100];

		parse_bitrate(sinfo[NL80211_STA_INFO_RX_BITRATE], buf, sizeof(buf));
		printf("\n\trx bitrate:\t%s", buf);
	}

	if (sinfo[NL80211_STA_INFO_EXPECTED_THROUGHPUT]) {
		uint32_t thr;

		thr = nla_get_u32(sinfo[NL80211_STA_INFO_EXPECTED_THROUGHPUT]);
		/* convert in Mbps but scale by 1000 to save kbps units */
		thr = thr * 1000 / 1024;

		printf("\n\texpected throughput:\t%u.%uMbps",
				thr / 1000, thr % 1000);
	}

	if (sinfo[NL80211_STA_INFO_LLID])
		printf("\n\tmesh llid:\t%d",
				nla_get_u16(sinfo[NL80211_STA_INFO_LLID]));
	if (sinfo[NL80211_STA_INFO_PLID])
		printf("\n\tmesh plid:\t%d",
				nla_get_u16(sinfo[NL80211_STA_INFO_PLID]));

	printf("\n");

	return NL_SKIP;
}

enum id_input {
	II_NONE,
	II_NETDEV,
	II_PHY_NAME,
	II_PHY_IDX,
	II_WDEV,
};

struct nl80211_state {
	struct nl_sock *nl_sock;
	int nl80211_id;
};

static int nl80211_init(struct nl80211_state *state) {
	int err;

	state->nl_sock = nl_socket_alloc();
	if (!state->nl_sock) {
		fprintf(stderr, "Failed to allocate netlink socket.\n");
		return -ENOMEM;
	}

	nl_socket_set_buffer_size(state->nl_sock, 8192, 8192);

	if (genl_connect(state->nl_sock)) {
		fprintf(stderr, "Failed to connect to generic netlink.\n");
		err = -ENOLINK;
		goto out_handle_destroy;
	}

	state->nl80211_id = genl_ctrl_resolve(state->nl_sock, "nl80211");
	if (state->nl80211_id < 0) {
		fprintf(stderr, "nl80211 not found.\n");
		err = -ENOENT;
		goto out_handle_destroy;
	}

	return 0;

	out_handle_destroy:
	nl_socket_free(state->nl_sock);
	return err;
}

static void nl80211_cleanup(struct nl80211_state *state) {
	nl_socket_free(state->nl_sock);
}

static int error_handler(struct sockaddr_nl *nla, struct nlmsgerr *err,
		void *arg) {
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


double os::WifiInfo::getSignalLevel() {
	if (not impl->enabled) {
		return 0;
	}
	int sigLevel = 0;
	struct nl80211_state nlstate;

	int err = nl80211_init(&nlstate);

	//allocate a message
	nl_msg *msg = nlmsg_alloc();
	if (!msg) {
		fprintf(stderr, "failed to allocate netlink message\n");
		return 2;
	}
	struct nl_cb *cb;
	struct nl_cb *s_cb;

	cb = nl_cb_alloc(NL_CB_DEFAULT);
	s_cb = nl_cb_alloc(NL_CB_DEFAULT);

	if (!cb || !s_cb) {
		fprintf(stderr, "failed to allocate netlink callbacks\n");
		err = 2;
	}

	genlmsg_put(msg, 0, 0, nlstate.nl80211_id, 0, NLM_F_DUMP, NL80211_CMD_GET_STATION, 0);

	int ifIndex = if_nametoindex("wlan0");
	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, ifIndex);

	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, print_sta_handler, NULL);

	nl_socket_set_cb(nlstate.nl_sock, s_cb);

	err = nl_send_auto_complete(nlstate.nl_sock, msg);
	if (err < 0) {
		nl_cb_put(cb);
		nlmsg_free(msg);
		return 1;
	}

	nl_cb_err(cb, NL_CB_CUSTOM, error_handler, &err);
	nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &err);
	nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &err);

	while (err > 0)
		nl_recvmsgs(nlstate.nl_sock, cb);

	nl80211_cleanup(&nlstate);


	logger.info(std::string("Current signal level: ") + std::to_string(sigLevel) + " dBm.");


	return sigLevel;

	nla_put_failure:
	fprintf(stderr, "building message failed\n");
	return 2;

}

std::string os::WifiInfo::getInterfaceName() {
	return impl->iwName;
}

double os::WifiInfo::getBitrate() {
	if (not impl->enabled) {
		return 0;
	}

	int bitrate;

	logger.info(std::string("Current bitrate: ") + std::to_string(bitrate) + " MBit/s.");

	return bitrate;
}

void os::WifiInfo::prepareStructs() {
}

static std::string macToStr(char *mac) {
	return (boost::format("%x:%x:%x:%x:%x:%x")
			% mac[0] % mac[1] % mac[2] % mac[3] % mac[4] % mac[5]).str();
}

std::unique_ptr<char> os::WifiInfo::getMacAddress(boost::asio::ip::address address) {
	std::unique_ptr<char> mac(new char[6]);

	logger.info((format("MAC address for IP %s is %s.") % address.to_string() % macToStr(mac.get())).str());

	return mac;
}

