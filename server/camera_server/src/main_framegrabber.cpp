#include "NetworkServer.hpp"
#include "initialization.hpp"
#include <Configuration.hpp>

#include <wallaroo/catalog.h>

using namespace std;
using namespace wallaroo;

int main(int argc, char *argv[]) {

    const char *banner = "SZARK Camera Server\n(C) Michał Słomkowski\nCompilation: " __DATE__;

    auto configFiles = common::init::initializeProgram(argc, argv, banner);

    Catalog c;
    c.Create("conf", "Configuration", configFiles);
//    c.Create("imgGrabber", "ImageGrabber", string("head"));
    c.Create("imgGrabber", "Video4LinuxImageGrabber", string("head"));
    c.Create("ifaceProvider", "InterfaceProvider", false);
    c.Create("imgCombiner", "HeadImageSource");
    c.Create("hudPainter", "HeadHudPainter");
    c.Create("srv", "NetworkServer");
//    c.Create("jpegEncoder", "OpenCvJpegEncoder");
    c.Create("jpegEncoder", "TurboJpegEncoder");

    wallaroo_within(c) {
        use("conf").as("config").of("imgGrabber");
        use("conf").as("config").of("imgCombiner");
        use("conf").as("config").of("hudPainter");
        use("conf").as("config").of("srv");

        use("imgGrabber").as("cameraGrabber").of("imgCombiner");
        use("hudPainter").as("hudPainter").of("imgCombiner");
        use("ifaceProvider").as("interfaceProvider").of("hudPainter");
        use("imgCombiner").as("imageSource").of("srv");
        use("jpegEncoder").as("jpegEncoder").of("srv");
    };

    c.CheckWiring();
    c.Init();

    shared_ptr<camera::INetworkServer> srv = c["srv"];

    srv->run();

    return 0;
}

