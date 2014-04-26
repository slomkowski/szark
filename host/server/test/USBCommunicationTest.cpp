#include <cstdint>
#include <vector>
#include <boost/test/unit_test.hpp>

#include "USBCommunicator.hpp"

#include "usb-commands.hpp"
#include "usb-settings.hpp"

BOOST_AUTO_TEST_CASE(USBCommunicationTest_Connection) {
	bridge::USBCommunicator comm;

	std::vector<uint8_t> getStateData = { USBCommands::BRIDGE_GET_STATE, USBCommands::MESSAGE_END };

	for (int i = 0; i < 100; i++) {
		comm.sendData(getStateData);
		auto returned = comm.receiveData();

		BOOST_CHECK(returned.size() == USB_SETTINGS_DEVICE_TO_HOST_DATAPACKET_SIZE);

		BOOST_CHECK_MESSAGE(returned.at(8) == USBCommands::MESSAGE_END, "real message length");
	}
}
