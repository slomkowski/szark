#ifndef WIFIINFO_HPP_
#define WIFIINFO_HPP_

#include <string>
#include <stdexcept>

#include <log4cpp/Category.hh>
#include <wallaroo/registered.h>

#include "Configuration.hpp"

namespace os {

	class WifiException : public std::runtime_error {
	public:
		WifiException(const std::string &message)
				: std::runtime_error(message) {
		}
	};

	class IWifiInfo {
	public:
		/*
		 * Returns signal level in dBm.
		 */
		virtual double getSignalLevel() = 0;

		/*
		 * Returns the bitrate of the link if MBits/s.
		 */
		virtual double getBitrate() = 0;

		/*
		 * Returns wireless interface name.
		 */
		virtual std::string getInterfaceName() = 0;

		virtual ~IWifiInfo() = default;
	};

	struct WifiInfoImpl;

	class WifiInfo : public IWifiInfo, public wallaroo::Device {
	public:
		/*
		 * The constructor checks if the device is valid.
		 */
		WifiInfo(std::string iwName);

		/*
		 * Default constructor reads the device name from config space.
		 */
		WifiInfo();

		~WifiInfo();

		virtual double getSignalLevel();

		virtual double getBitrate();

		std::string getInterfaceName();

	private:
		log4cpp::Category &logger;
		wallaroo::Plug<common::config::Configuration> config;

		WifiInfoImpl *impl;

		void prepareStructs();

		void Init();
	};

} /* namespace os */

#endif /* WIFIINFO_HPP_ */
