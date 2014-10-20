#include <memory>
#include <wallaroo/catalog.h>

#include "initialization.hpp"

#include "NetServer.hpp"

using namespace wallaroo;

int main(int argc, char *argv[]) {

	const char *banner = "SZARK Control Server\n(C) Michał Słomkowski\nCompilation: " __DATE__;

	auto configFiles = common::init::initializeProgram(argc, argv, banner);

	Catalog c;
	c.Create("conf", "Configuration", configFiles);
	c.Create("comm", "USBCommunicator");
	c.Create("wifiInfo", "WifiInfo");
	c.Create("netServer", "NetServer");
	c.Create("reqQueuer", "RequestQueuer");
	c.Create("bridgeProc", "BridgeProcessor");

	wallaroo_within(c) {
		use("conf").as("config").of("wifiInfo");
		use("conf").as("config").of("netServer");
		use("comm").as("communicator").of("bridgeProc");
		use("reqQueuer").as("requestQueuer").of("netServer");
		use("bridgeProc").as("requestProcessors").of("reqQueuer");
		// TODO napisać i dodać os processor - min. wifi
	}

	c.CheckWiring();
	c.Init();

	auto netServer = std::shared_ptr<processing::INetServer>(c["netServer"]);

	netServer->run();

	return 0;
}

