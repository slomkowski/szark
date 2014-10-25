#include <chrono>
#include <thread>

#include <boost/test/unit_test.hpp>

#include "WifiInfo.hpp"

BOOST_AUTO_TEST_CASE(WifiInfoTest_Run) {
	const std::string INTERFACE_NAME = "wlan0";

	BOOST_CHECK_THROW(os::WifiInfo("invalid_iface"), os::WifiException);

	os::WifiInfo wifi(INTERFACE_NAME);

	for (int i = 0; i < 10; i++) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		BOOST_CHECK_THROW(wifi.getWifiLinkParams(boost::asio::ip::address_v4::from_string("1.1.1.1")), os::WifiException);

		auto linkParams = wifi.getWifiLinkParams(boost::asio::ip::address_v4::from_string("192.168.0.1"));

		BOOST_CHECK_EQUAL(INTERFACE_NAME, linkParams.getInterfaceName());

		auto rxBitrate = linkParams.getRxBitrate();
		auto txBitrate = linkParams.getTxBitrate();
		auto sigLevel = linkParams.getSignalStrength();

		BOOST_CHECK_GT(rxBitrate, 1.0);
		BOOST_CHECK_LT(rxBitrate, 300.0);

		BOOST_CHECK_GT(txBitrate, 1.0);
		BOOST_CHECK_LT(txBitrate, 300.0);

		BOOST_TEST_MESSAGE(std::string("wifi bitrate: ") + std::to_string(rxBitrate));

		BOOST_CHECK_GT(sigLevel, -150.0);
		BOOST_CHECK_LT(sigLevel, 0);

		BOOST_TEST_MESSAGE(std::string("wifi signal strength: ") + std::to_string(sigLevel));

		linkParams.getMacAddress();


	}
}
