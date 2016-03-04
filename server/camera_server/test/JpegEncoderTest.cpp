#include "JpegEncoder.hpp"

#include <boost/test/unit_test.hpp>
#include <wallaroo/catalog.h>

using namespace std;
using namespace camera;

const string TEST_IMAGE = "test_image.png";

constexpr int BUFFER_SIZE = 0x40000;

BOOST_AUTO_TEST_CASE(JpegEncoderTest_Performance) {
    wallaroo::Catalog catalog;
    catalog.Create("openCvJpegEncoder", "OpenCvJpegEncoder");
    catalog.Create("turboJpegEncoder", "TurboJpegEncoder");

    catalog.CheckWiring();

    unsigned char *inputBuffer;
    unsigned char *outputBuffer;

    if (posix_memalign(reinterpret_cast<void **>(&inputBuffer), 32, BUFFER_SIZE) != 0
        or posix_memalign(reinterpret_cast<void **>(&outputBuffer), 32, BUFFER_SIZE) != 0) {
        throw runtime_error("cannot allocate buffer");
    }

    cv::Mat inputImage = cv::imread(TEST_IMAGE);

    array<pair<string, shared_ptr<IJpegEncoder>>, 2> encoders = {
            make_pair("OpenCV", catalog["openCvJpegEncoder"]),
            make_pair("TurboJPEG", catalog["turboJpegEncoder"])
    };

    for (auto &enc : encoders) {
        BOOST_TEST_MESSAGE("Testing encoder " << enc.first);

        int size;

        for (int i = 0; i < 10; ++i) {
            size = enc.second->encodeImage(inputImage, outputBuffer, BUFFER_SIZE);
        }

        BOOST_TEST_MESSAGE("Encoded JPEG size: " << size << " B.");
    }

//    for (int i = 0; i < 10; i++) {
//        string fileName = "out.jpg";
//        auto result = imageGrabber->getImage(true);
//        cv::imwrite(fileName, result);
//        BOOST_TEST_MESSAGE("Frame combined and saved to " << fileName);
//    }

    BOOST_CHECK_EQUAL(1, 1);
}
