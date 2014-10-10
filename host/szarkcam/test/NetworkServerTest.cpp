#include <boost/test/unit_test.hpp>
#include <wallaroo/catalog.h>
#include <thread>
#include "NetworkServer.hpp"
#include <opencv2/opencv.hpp>

using namespace std;
using namespace camera;

class DummyImageSource : public camera::IImageSource, public wallaroo::Device {
public:
	cv::Mat getImage(bool drawHud) {
		this_thread::sleep_for(chrono::milliseconds(100));
		return cv::imread("test.jpg", CV_LOAD_IMAGE_COLOR);
	}

	void getEncodedImage(bool drawHud, EncodedImageProcessor processor) {
		auto img = getImage(drawHud);

		cv::vector<unsigned char> buffer;
		cv::imencode(".jpg", img, buffer);

		processor(&buffer[0], buffer.size());
	}
};

WALLAROO_REGISTER(DummyImageSource);

BOOST_AUTO_TEST_CASE(NetworkServerTest_Run) {
	wallaroo::Catalog catalog;
	catalog.Create("dummyImageSource", "DummyImageSource");
	catalog.Create("srv", "NetworkServer");
	wallaroo::use(catalog["dummyImageSource"]).as("imageSource").of(catalog["srv"]);

	catalog.CheckWiring();

	shared_ptr<INetworkServer> srv = catalog["srv"];

	srv->run();

	BOOST_CHECK_EQUAL(1, 1);
}