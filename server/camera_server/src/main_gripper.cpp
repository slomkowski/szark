#define BACKWARD_HAS_DW 1

#include "IoServiceProvider.hpp"
#include "initialization.hpp"

#include <backward.hpp>
#include <wallaroo/catalog.h>

using namespace std;
using namespace wallaroo;

int main(int argc, char *argv[]) {
    backward::SignalHandling sh;

    const char *banner = "SZARK Camera Server\n(C) Michał Słomkowski\nCompilation: " __DATE__;

    auto configFiles = common::init::initializeProgram(argc, argv, banner);

    Catalog c;
    c.Create("conf", "Configuration", configFiles);
    c.Create("imgGrabberLeft", "Video4LinuxImageGrabber", string("left"));
    c.Create("imgGrabberRight", "Video4LinuxImageGrabber", string("right"));
    c.Create("ifaceProvider", "InterfaceProvider", false);
    c.Create("imgCombiner", "GripperImageSource");
    c.Create("hudPainter", "GripperHudPainter");
    c.Create("ioServiceProvider", "IoServiceProvider");
    c.Create("srv", "NetworkServer");
    c.Create("jpegEncoder", "OpenCvJpegEncoder");

    wallaroo_within(c) {
        use("conf").as("config").of("imgGrabberLeft");
        use("conf").as("config").of("imgGrabberRight");
        use("conf").as("config").of("imgCombiner");
        use("conf").as("config").of("hudPainter");
        use("conf").as("config").of("srv");

        use("ioServiceProvider").as("ioServiceProvider").of("srv");
        use("imgGrabberLeft").as("leftCameraGrabber").of("imgCombiner");
        use("imgGrabberRight").as("rightCameraGrabber").of("imgCombiner");
        use("hudPainter").as("hudPainter").of("imgCombiner");
        use("imgCombiner").as("imageSource").of("srv");
        use("jpegEncoder").as("jpegEncoder").of("srv");
    };

    c.CheckWiring();
    c.Init();

    std::shared_ptr<common::IoServiceProvider>(c["ioServiceProvider"])->run();

    return 0;
}

