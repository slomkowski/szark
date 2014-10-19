#include <vector>
#include <algorithm>
#include <iostream>
#include <cstdlib>

#include <wallaroo/catalog.h>
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>

#include "initialization.hpp"

#include "logging.hpp"
#include "Configuration.hpp"

using namespace std;
namespace po = boost::program_options;

static string DEFAULT_CONFIG_FILE = "config.ini";

std::vector<std::string> common::init::initializeProgram(int argc, char *argv[], std::string banner) {
	bool displayColorLog;
	string logLevel;
	vector<string> configFiles;

	cout << banner << endl;

	po::options_description desc((
			boost::format("\nUsage: %s [options] [config file 1] ...\nAllowed options:") % argv[0]).str());

	desc.add_options()
			("help", "produce help message")
			("color", po::value<bool>(&displayColorLog)->default_value(true), "display color log")
			("config-file", po::value<vector<string>>(), "config file")
			("log-level", po::value<string>(&logLevel)->default_value("info"), "set log level. Available values: debug, info, notice, warn, error");

	po::positional_options_description co;
	co.add("config-file", -1);

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(desc).positional(co).run(), vm);
	po::notify(vm);

	if (vm.count("help")) {
		cout << desc << endl;
		exit(1);
	}

	if (vm.count("config-file") == 0) {
		configFiles.push_back(DEFAULT_CONFIG_FILE);
	} else {
		configFiles = vm["config-file"].as<vector<string>>();
	}

	common::logger::configureLogger(configFiles, logLevel, displayColorLog);

	return configFiles;
}