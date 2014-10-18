#include <chrono>
#include <future>

#include <log4cpp/PropertyConfigurator.hh>
#include <wallaroo/catalog.h>

#include "NetworkServer.hpp"
#include "Configuration.hpp"

using namespace std;

int main() {
	std::string initFileName = "logger.properties";
	log4cpp::PropertyConfigurator::configure(initFileName);

	wallaroo::Catalog catalog;
	catalog.Create("conf", "Configuration", initFileName);
	catalog.Create("imgGrabberLeft", "ImageGrabber", string("left"));
	catalog.Create("imgGrabberRight", "ImageGrabber", string("right"));
	catalog.Create("imgCombiner", "GripperImageSource");
	catalog.Create("hudPainter", "GripperHudPainter");

	wallaroo::use(catalog["conf"]).as("config").of(catalog["imgGrabberLeft"]);
	wallaroo::use(catalog["conf"]).as("config").of(catalog["imgGrabberRight"]);
	wallaroo::use(catalog["conf"]).as("config").of(catalog["imgCombiner"]);
	wallaroo::use(catalog["conf"]).as("config").of(catalog["hudPainter"]);

	wallaroo::use(catalog["imgGrabberLeft"]).as("leftCameraGrabber").of(catalog["imgCombiner"]);
	wallaroo::use(catalog["imgGrabberRight"]).as("rightCameraGrabber").of(catalog["imgCombiner"]);
	wallaroo::use(catalog["hudPainter"]).as("hudPainter").of(catalog["imgCombiner"]);

	catalog.Create("srv", "NetworkServer");
	wallaroo::use(catalog["conf"]).as("config").of(catalog["srv"]);
	wallaroo::use(catalog["imgCombiner"]).as("imageSource").of(catalog["srv"]);

	catalog.CheckWiring();
	catalog.Init();

	shared_ptr<camera::INetworkServer> srv = catalog["srv"];

	srv->run();
}

