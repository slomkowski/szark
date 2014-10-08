#include <iostream>
#include <chrono>

#include <future>
#include <wallaroo/catalog.h>
#include "NetworkServer.hpp"
#include "Configuration.hpp"

#include <log4cpp/PropertyConfigurator.hh>

using namespace std;

int main() {
	std::string initFileName = "logger.properties";
	log4cpp::PropertyConfigurator::configure(initFileName);
	config::Configuration::create();

	wallaroo::Catalog catalog;
	catalog.Create("imgGrabberLeft", "ImageGrabber", string("left"));
	catalog.Create("imgGrabberRight", "ImageGrabber", string("right"));
	catalog.Create("imgCombiner", "ImageCombiner");

	wallaroo::use(catalog["imgGrabberLeft"]).as("leftCameraGrabber").of(catalog["imgCombiner"]);
	wallaroo::use(catalog["imgGrabberRight"]).as("rightCameraGrabber").of(catalog["imgCombiner"]);

	catalog.Create("srv", "NetworkServer");
	wallaroo::use(catalog["imgCombiner"]).as("imageSource").of(catalog["srv"]);

	catalog.CheckWiring();

	shared_ptr<camera::INetworkServer> srv = catalog["srv"];

	srv->run();
}

