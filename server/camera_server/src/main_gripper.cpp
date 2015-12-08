#include "NetworkServer.hpp"
#include "initialization.hpp"

#include <wallaroo/catalog.h>

using namespace std;
using namespace wallaroo;

int main(int argc, char *argv[]) {

    const char *banner = "SZARK Camera Server\n(C) Michał Słomkowski\nCompilation: " __DATE__;

    auto configFiles = common::init::initializeProgram(argc, argv, banner);

    Catalog c;
    c.Create("conf", "Configuration", configFiles);
    c.Create("imgGrabberLeft", "ImageGrabber", string("left"));
    c.Create("imgGrabberRight", "ImageGrabber", string("right"));
    c.Create("imgCombiner", "GripperImageSource");
    c.Create("hudPainter", "GripperHudPainter");
    c.Create("srv", "NetworkServer");

    wallaroo_within(c) {
        use("conf").as("config").of("imgGrabberLeft");
        use("conf").as("config").of("imgGrabberRight");
        use("conf").as("config").of("imgCombiner");
        use("conf").as("config").of("hudPainter");
        use("conf").as("config").of("srv");

        use("imgGrabberLeft").as("leftCameraGrabber").of("imgCombiner");
        use("imgGrabberRight").as("rightCameraGrabber").of("imgCombiner");
        use("hudPainter").as("hudPainter").of("imgCombiner");
        use("imgCombiner").as("imageSource").of("srv");
    };

    c.CheckWiring();
    c.Init();

    shared_ptr<camera::INetworkServer> srv = c["srv"];

    srv->run();

    return 0;
}

