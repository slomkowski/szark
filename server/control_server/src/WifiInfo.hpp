#ifndef WIFIINFO_HPP_
#define WIFIINFO_HPP_

#include <string>
#include <stdexcept>

#include <log4cpp/Category.hh>
#include <wallaroo/registered.h>
#include <boost/asio/ip/address.hpp>

#include "Configuration.hpp"

namespace os {

    class WifiException : public std::runtime_error {
    public:
        WifiException(const std::string &message)
                : std::runtime_error(message) {
        }
    };

    class MacAddress {
    private:
        unsigned char mac[6];

    public:
        MacAddress(char *addr);

        std::string toString() const;

        unsigned char *getMac() {
            return mac;
        }

        bool operator<(const MacAddress &other) const;

        bool operator==(const MacAddress &other) const;

        bool operator!=(const MacAddress &other) const {
            return not (*this == other);
        }
    };

    class WifiLinkParams {
    private:
        double txBitrate;
        double rxBitrate;
        double signalStrength;

        MacAddress macAddress;

        std::string interfaceName;

    public:
        /*
         * Returns the transmit bitrate of the link if MBits/s.
         */
        double getTxBitrate() const {
            return txBitrate;
        }

        /*
         * Returns the receive bitrate of the link if MBits/s.
         */
        double getRxBitrate() const {
            return rxBitrate;
        }

        /*
         * Returns signal level in dBm.
         */
        double getSignalStrength() const {
            return signalStrength;
        }

        MacAddress const &getMacAddress() const {
            return macAddress;
        }

        std::string getInterfaceName() const {
            return interfaceName;
        }

        WifiLinkParams(double txBitrate, double rxBitrate, double signalStrength, MacAddress &macAddress,
                       std::string interfaceName)
                : txBitrate(txBitrate),
                  rxBitrate(rxBitrate),
                  signalStrength(signalStrength),
                  macAddress(macAddress),
                  interfaceName(interfaceName) {
        }

        std::string toString() const;
    };

    class IWifiInfo {
    public:

        virtual WifiLinkParams getWifiLinkParams(boost::asio::ip::address address) = 0;

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

        virtual WifiLinkParams getWifiLinkParams(boost::asio::ip::address address);

    private:
        log4cpp::Category &logger;
        wallaroo::Plug<common::config::Configuration> config;

        WifiInfoImpl *impl;

        void Init();

        void acquireNetworkInformationThreadFunction();

        void dumpStation();

        void dumpArp();

        void dumpCardAttributes();
    };


} /* namespace os */

#endif /* WIFIINFO_HPP_ */
