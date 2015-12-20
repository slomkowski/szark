#include "NetworkServer.hpp"

#include <boost/test/unit_test.hpp>
#include <wallaroo/catalog.h>
#include <opencv2/opencv.hpp>

#include <thread>

using namespace std;
using namespace camera;

class DummyImageSource : public camera::IImageSource, public wallaroo::Device {
public:
    cv::Mat getImage(bool drawHud) {
        this_thread::sleep_for(chrono::milliseconds(100));
        return cv::imread("test.jpg", CV_LOAD_IMAGE_COLOR);
    }
};

WALLAROO_REGISTER(DummyImageSource);

BOOST_AUTO_TEST_CASE(NetworkServerTest_Run) {
    wallaroo::Catalog catalog;
    catalog.Create("dummyImageSource", "DummyImageSource");
    catalog.Create("srv", "NetworkServer");
    catalog.Create("ioServiceProvider", "IoServiceProvider");

    wallaroo::use(catalog["ioServiceProvider"]).as("ioServiceProvider").of(catalog["srv"]);
    wallaroo::use(catalog["dummyImageSource"]).as("imageSource").of(catalog["srv"]);

    catalog.CheckWiring();

    std::shared_ptr<common::IoServiceProvider>(catalog["ioServiceProvider"])->run();

    BOOST_CHECK_EQUAL(1, 1);
}