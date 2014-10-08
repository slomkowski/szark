#include <boost/test/unit_test.hpp>
#include <wallaroo/catalog.h>
#include <thread>
#include "NetworkServer.hpp"
#include <vips/vips.h>

using namespace std;
using namespace camera;

class DummyImageSource : public camera::IImageCombiner, public wallaroo::Device {
public:
	vips::VImage getCombinedImage(bool drawHud) {
		this_thread::sleep_for(chrono::milliseconds(100));
		return vips::VImage("test.jpg");
	}

	void getEncodedImage(bool drawHud, EncodedImageProcessor processor) {
		auto img = getCombinedImage(drawHud);

		void *bufferPointer;
		size_t length;
		vips_jpegsave_buffer(img.image(), &bufferPointer, &length, nullptr);

		processor(bufferPointer, length);

		g_free(bufferPointer);
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