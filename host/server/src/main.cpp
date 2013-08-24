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
#include "InterfaceManager.hpp"

using namespace std;
using std::chrono::duration_cast;
using std::chrono::microseconds;

int main(int argc, char *argv[]) {
	bridge::InterfaceManager i;

	typedef std::chrono::high_resolution_clock Clock;

	auto t1 = Clock::now();

	//i.setLCDText("twoja star");
	i.setKillSwitch(true);
	i.setKillSwitch(false);
	i.isKillSwitchActive();
	//i.arm.calibrate();
	i.motor[bridge::Motor::LEFT].setSpeed(8);
	i.motor[bridge::Motor::LEFT].setDirection(bridge::Direction::STOP);
	i.expander[bridge::ExpanderDevice::LIGHT_LEFT].setEnabled(false);
	i.arm[bridge::Joint::ELBOW].setDirection(bridge::Direction::BACKWARD);
	i.arm[bridge::Joint::ELBOW].setSpeed(70);
	i.setLCDText("hello world ala ma kota");
	i.arm.calibrate();

	for (int t = 0; t < 150; t++) {
		 auto tstart = Clock::now();

		i.stageChanges();
		 auto tstop = Clock::now();

		std::cout << "timer single: " << duration_cast<microseconds>(tstop - tstart).count() << " us\n";

		i.expander[bridge::ExpanderDevice::LIGHT_LEFT].setEnabled(true);

		std::cout << "voltage: " << i.getVoltage() << std::endl;
	}

	auto t2 = Clock::now();
	std::cout << "timer: " << duration_cast<microseconds>(t2 - t1).count() << " us\n";
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

