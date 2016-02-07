#define BACKWARD_HAS_DW 1

#include "NetServer.hpp"
#include "IoServiceProvider.hpp"
#include "initialization.hpp"

#include <backward.hpp>
#include <wallaroo/catalog.h>

using namespace wallaroo;

int main(int argc, char *argv[]) {
    backward::SignalHandling sh;

    const char *banner = "SZARK Control Server\n(C) Michał Słomkowski\nCompilation: " __DATE__;

    auto configFiles = common::init::initializeProgram(argc, argv, banner);

    Catalog c;
    c.Create("conf", "Configuration", configFiles);
    c.Create("comm", "USBCommunicator");
    c.Create("ifaceProvider", "InterfaceProvider", true);
    c.Create("ifaceMgr", "InterfaceManager");
    c.Create("wifiInfo", "WifiInfo");
    c.Create("ioServiceProvider", "IoServiceProvider");
    c.Create("netServer", "NetServer");
    c.Create("reqQueuer", "RequestQueuer");
    c.Create("bridgeProc", "BridgeProcessor");
    c.Create("osProc", "OSInformationProcessor");

    wallaroo_within(c) {
        use("conf").as("config").of("wifiInfo");
        use("conf").as("config").of("netServer");
        use("conf").as("config").of("osProc");
        use("conf").as("config").of("ifaceMgr");
        use("ioServiceProvider").as("ioServiceProvider").of("netServer");
        use("ifaceProvider").as("interfaceProvider").of("ifaceMgr");
        use("ifaceMgr").as("interfaceManager").of("bridgeProc");
        use("wifiInfo").as("wifiInfo").of("osProc");
        use("comm").as("communicator").of("bridgeProc");
        use("reqQueuer").as("requestQueuer").of("netServer");
        use("bridgeProc").as("requestProcessors").of("reqQueuer");
        use("osProc").as("requestProcessors").of("reqQueuer");
    }

    c.CheckWiring();
    c.Init();

    std::shared_ptr<common::IoServiceProvider>(c["ioServiceProvider"])->run();

    return 0;
}

