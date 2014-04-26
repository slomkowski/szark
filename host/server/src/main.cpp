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
#include <algorithm>
#include <stdexcept>

#include <log4cpp/PropertyConfigurator.hh>

int main(int argc, char *argv[]) {
	std::string initFileName = "logger.properties";
	log4cpp::PropertyConfigurator::configure(initFileName);
}

