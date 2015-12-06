#include <vector>
#include <cstdint>
#include <chrono>
#include <vector>
#include <algorithm>
#include <sstream>
#include <memory>

#include <boost/test/unit_test.hpp>
#include <wallaroo/catalog.h>

#include "InterfaceManager.hpp"
#include "USBCommunicator.hpp"

BOOST_AUTO_TEST_CASE(InterfaceManager_Operation) {
	using std::chrono::duration_cast;
	using std::chrono::microseconds;
	typedef std::chrono::high_resolution_clock Clock;

	using namespace bridge;
	using namespace common::bridge;

	USBCommunicator comm;

	wallaroo::Catalog catalog;
	catalog.Create("config", "Configuration");
	catalog.Create("mgr", "InterfaceManager");
	wallaroo_within(catalog) {
		wallaroo::use("config").as("config").of("mgr");
	};

	auto interfaceManager = std::shared_ptr<IInterfaceManager>(catalog["mgr"]);
	Interface &i = interfaceManager->iface();

	auto syncWithDeviceFunc = [&](std::vector<uint8_t> r) {
		comm.sendData(r);
		return comm.receiveData();
	};

	auto t1 = Clock::now();

	i.setLCDText("SZARK Test");
	i.setKillSwitch(true);
	i.setKillSwitch(false);
	i.isKillSwitchActive();

	i.motor[Motor::LEFT].setSpeed(8);
	i.motor[Motor::LEFT].setDirection(Direction::STOP);
	i.expander[ExpanderDevice::LIGHT_LEFT].setEnabled(false);
	i.arm[Joint::ELBOW].setDirection(Direction::BACKWARD);
	i.arm[Joint::ELBOW].setSpeed(70);
	i.setLCDText("hello world");
	i.arm.calibrate();

	std::vector<double> timings;

	for (int t = 0; t < 500; t++) {
		auto tstart = Clock::now();

		i.expander[ExpanderDevice::LIGHT_LEFT].setEnabled(true);

		if (t % 3 == 1)
			i.motor[Motor::LEFT].setSpeed(9);
		if (t % 3 == 2)
			i.motor[Motor::LEFT].setSpeed(9);

		if (t % 7 == 4)
			i.setLCDText("hi" + std::to_string(t));

		if (t % 60 == 30) {
			i.setLCDText("this is a very long text to test maximal transfer time");
			//i.arm[Joint::ELBOW].setDirection(Direction::STOP);
			//i.arm[Joint::ELBOW].setSpeed(3);
			i.arm[Joint::GRIPPER].setDirection(Direction::STOP);
			i.arm[Joint::GRIPPER].setSpeed(3);
			i.motor[Motor::LEFT].setSpeed(3);
			i.motor[Motor::RIGHT].setSpeed(4);
			i.expander[ExpanderDevice::LIGHT_CAMERA].setEnabled(true);
		}

		i.motor[Motor::LEFT].setSpeed(t % 10);

		BOOST_CHECK_EQUAL(i.motor[Motor::LEFT].getSpeed(), t % 10);

		i.arm[Joint::ELBOW].setDirection(Direction::BACKWARD);
		i.arm[Joint::ELBOW].setSpeed(t % 5);

		if (t % 2)
			i.motor[Motor::LEFT].setDirection(Direction::FORWARD);
		else
			i.motor[Motor::LEFT].setDirection(Direction::BACKWARD);

		interfaceManager->syncWithDevice(syncWithDeviceFunc);

		auto tstop = Clock::now();
		timings.push_back(duration_cast<microseconds>(tstop - tstart).count());
	}

	i.arm.brake();

	interfaceManager->syncWithDevice(syncWithDeviceFunc);

	auto t2 = Clock::now();

	std::stringstream avgTime;
	avgTime << "avg time: " << (std::accumulate(timings.begin(), timings.end(), 0.0) / timings.size()) << " us\n";
	BOOST_TEST_MESSAGE(avgTime.str());

	std::stringstream totalTime;
	totalTime << "total time: " << duration_cast<microseconds>(t2 - t1).count() << " us\n";
	BOOST_TEST_MESSAGE(totalTime.str());
}
