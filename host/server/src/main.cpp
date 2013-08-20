/*
 * main.cpp
 *
 *  Created on: 02-08-2013
 *      Author: michal
 */

#include <iostream>
#include <boost/timer.hpp>
#include <chrono>
#include <vector>
#include "USBRawCommunicator.hpp"
#include "Interface.hpp"

using namespace std;
using std::chrono::duration_cast;
using std::chrono::microseconds;

int main(int argc, char *argv[]) {
	bridge::Interface i;

	i.setLCDText("twoja star");
	i.setKillSwitch(true);
	i.setKillSwitch(false);

	i.sendChanges();
}

/*
int main(int argc, char *argv[]) {
	auto comm = new USB::RawCommunicator();

	string test = "hello world ala ma kota";
	typedef std::chrono::high_resolution_clock Clock;

	vector<uint8_t> buffer;

	buffer.push_back(USBCommands::BRIDGE_GET_STATE);

	buffer.push_back(USBCommands::BRIDGE_SET_KILLSWITCH);
	buffer.push_back(USBCommands::bridge::INACTIVE);

	buffer.push_back(USBCommands::BRIDGE_LCD_SET);
	buffer.push_back(test.length());

	for (int i = 0; i <= test.length(); i++) {
		buffer.push_back(test[i]);
	}

	auto t1 = Clock::now();
	comm->sendMessage(USBCommands::USB_WRITE, (uint8_t*) &buffer[0], buffer.size());
	auto t2 = Clock::now();

	std::cout << "timer: " << duration_cast<microseconds>(t2 - t1).count() << " us\n";

	uint8_t buff[100];
	t1 = Clock::now();
	auto length = comm->recvMessage(USBCommands::USB_READ, buff, 100);
	t2 = Clock::now();
	std::cout << "timer: " << duration_cast<microseconds>(t2 - t1).count() << " us\n";
	auto bState = reinterpret_cast<USBCommands::bridge::State*>(buff);
	cout << "volt: " << bState->rawVoltage << ", curr: " << bState->rawCurrent << endl;
	cout << "buttons: " << bState->buttonDown << ", " << bState->buttonUp << ", " << bState->buttonEnter << endl;
	cout << "killswitch: " << (bState->killSwitch == USBCommands::bridge::ACTIVE ? "active" : "inactive") << endl;

	delete comm;

	bridge::Interface interface;

	interface.motor[bridge::Motor::LEFT].getMotor();
}*/

