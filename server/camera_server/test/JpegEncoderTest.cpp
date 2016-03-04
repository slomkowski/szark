#include "JpegEncoder.hpp"

#include "utils.hpp"

#include <boost/test/unit_test.hpp>
#include <wallaroo/catalog.h>

#include <boost/algorithm/string/replace.hpp>

#include <fstream>
#include <chrono>

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
    string timestamp = common::utils::getTimestamp();

    array<pair<string, shared_ptr<IJpegEncoder>>, 2> encoders = {
            make_pair("OpenCV", catalog["openCvJpegEncoder"]),
            make_pair("TurboJPEG", catalog["turboJpegEncoder"])
    };

    for (auto &enc : encoders) {
        BOOST_TEST_MESSAGE("Testing encoder " << enc.first << " ...");

        int size;

        int ms = common::utils::measureTime<chrono::milliseconds>([&]() {
            for (int i = 0; i < 100; ++i) {
                size = enc.second->encodeImage(inputImage, outputBuffer, BUFFER_SIZE);
            }
        });

        BOOST_TEST_MESSAGE("Encoded JPEG size: " << size << " B, overall time: " << ms << " ms.");
        string tempPath = "/tmp/szark_testjpeg_" + timestamp + "_" + enc.first + ".jpg";
        boost::replace_all(tempPath, ":", "_");
        ofstream outputJpeg(tempPath, ios::binary | ios::out | ios::trunc);
        outputJpeg.write(reinterpret_cast<const char *>(outputBuffer), size);
        outputJpeg.close();
        BOOST_TEST_MESSAGE("JPEG saved as " << tempPath);
    }

    BOOST_CHECK_EQUAL(1, 1);
}
