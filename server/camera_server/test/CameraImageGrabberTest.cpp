#include "CameraImageGrabber.hpp"

#include "utils.hpp"

#include <boost/test/unit_test.hpp>
#include <wallaroo/catalog.h>

#include <boost/algorithm/string/replace.hpp>

#include <tuple>

using namespace std;
using namespace camera;

BOOST_AUTO_TEST_CASE(CameraImageGrabberTest_Run) {
    wallaroo::Catalog catalog;

    catalog.Create("conf", "Configuration");//, confVec);
    shared_ptr<common::config::Configuration> config = catalog["conf"];
    config->putInt("ImageGrabber.test_device", 0);
    config->putInt("ImageGrabber.test_width", 720);
    config->putInt("ImageGrabber.test_height", 480);
    config->putInt("ImageGrabber.test_input", 0);

    BOOST_CHECK_EQUAL(config->getInt("ImageGrabber.test_device"), 0);

    catalog.Create("imgGrabber", "Video4LinuxImageGrabber", string("test"));

    wallaroo_within(catalog) {
        wallaroo::use("conf").as("config").of("imgGrabber");
    };

    catalog.CheckWiring();
    catalog.Init();

    shared_ptr<IImageGrabber> imageGrabber = catalog["imgGrabber"];

    for (int i = 0; i < 10; i++) {
        long frameNo;
        double fps = 0;
        cv::Mat frame;

        for (int j = 0; j < 5; ++j) {
            tie(frameNo, fps, frame) = imageGrabber->getFrame(false);
            BOOST_TEST_MESSAGE("Frame wait no. " << frameNo << ", fps: " << fps);
        }

        string tempPath = "/tmp/szark_imgdump_" + common::utils::getTimestamp() + ".png";
        boost::replace_all(tempPath, ":", "_");
        cv::imwrite(tempPath, frame);

        BOOST_TEST_MESSAGE("Raw frame saved as " << tempPath);
    }

    BOOST_CHECK_EQUAL(1, 1);
}