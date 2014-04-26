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

BOOST_AUTO_TEST_CASE(InterfaceManager_Operation) {
	using std::chrono::duration_cast;
	using std::chrono::microseconds;
	using namespace wallaroo;
	typedef std::chrono::high_resolution_clock Clock;

	wallaroo::Catalog catalog;
	catalog.Create("comm", "USBCommunicator");
	catalog.Create("InterfaceManager", "InterfaceManager");

	wallaroo::use(catalog["comm"]).as("communicator").of(catalog["InterfaceManager"]);

	catalog.CheckWiring();

	std::shared_ptr<bridge::InterfaceManager> i = catalog["InterfaceManager"];

	auto t1 = Clock::now();

	i->setLCDText("SZARK Test");
	i->setKillSwitch(true);
	i->setKillSwitch(false);
	i->isKillSwitchActive();

	i->motor[bridge::Motor::LEFT].setSpeed(8);
	i->motor[bridge::Motor::LEFT].setDirection(bridge::Direction::STOP);
	i->expander[bridge::ExpanderDevice::LIGHT_LEFT].setEnabled(false);
	i->arm[bridge::Joint::ELBOW].setDirection(bridge::Direction::BACKWARD);
	i->arm[bridge::Joint::ELBOW].setSpeed(70);
	i->setLCDText("hello world");
	i->arm.calibrate();

	std::vector<double> timings;

	for (int t = 0; t < 500; t++) {
		auto tstart = Clock::now();

		i->expander[bridge::ExpanderDevice::LIGHT_LEFT].setEnabled(true);

		if (t % 3 == 1)
			i->motor[bridge::Motor::LEFT].setSpeed(9);
		if (t % 3 == 2)
			i->motor[bridge::Motor::LEFT].setSpeed(9);

		if (t % 7 == 4)
			i->setLCDText("hi" + std::to_string(t));

		if (t % 60 == 30) {
			i->setLCDText("this is a very long text to test maximal transfer time");
			//i->arm[bridge::Joint::ELBOW].setDirection(bridge::Direction::STOP);
			//i->arm[bridge::Joint::ELBOW].setSpeed(3);
			i->arm[bridge::Joint::GRIPPER].setDirection(bridge::Direction::STOP);
			i->arm[bridge::Joint::GRIPPER].setSpeed(3);
			i->arm[bridge::Joint::WRIST].setSpeed(5);
			i->motor[bridge::Motor::LEFT].setSpeed(3);
			i->motor[bridge::Motor::RIGHT].setSpeed(4);
			i->expander[bridge::ExpanderDevice::LIGHT_CAMERA].setEnabled(true);
		}

		i->motor[bridge::Motor::LEFT].setSpeed(t % 10);

		BOOST_CHECK_EQUAL(i->motor[bridge::Motor::LEFT].getSpeed(), t % 10);

		i->arm[bridge::Joint::ELBOW].setDirection(bridge::Direction::BACKWARD);
		i->arm[bridge::Joint::ELBOW].setSpeed(t % 5);

		if (t % 2)
			i->motor[bridge::Motor::LEFT].setDirection(bridge::Direction::FORWARD);
		else
			i->motor[bridge::Motor::LEFT].setDirection(bridge::Direction::BACKWARD);

		i->stageChanges();
		auto tstop = Clock::now();
		timings.push_back(duration_cast<microseconds>(tstop - tstart).count());

		//std::cout << t << ": timer single: " << duration_cast<microseconds>(tstop - tstart).count() << " us\n";

		//std::cout << "voltage: " << i->getVoltage() << std::endl;
	}

	i->arm.brake();

	i->stageChanges();

	auto t2 = Clock::now();

	std::stringstream avgTime;
	avgTime << "avg time: " << (std::accumulate(timings.begin(), timings.end(), 0.0) / timings.size()) << " us\n";
	BOOST_MESSAGE(avgTime.str());

	std::stringstream totalTime;
	totalTime << "total time: " << duration_cast<microseconds>(t2 - t1).count() << " us\n";
	BOOST_MESSAGE(totalTime.str());
}
