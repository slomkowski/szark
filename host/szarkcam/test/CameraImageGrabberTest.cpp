#include <boost/test/unit_test.hpp>
#include <wallaroo/catalog.h>

#include "CameraImageGrabber.hpp"

using namespace std;
using namespace camera;

BOOST_AUTO_TEST_CASE(CameraImageGrabberTest_Run) {
	wallaroo::Catalog catalog;
	catalog.Create("imgGrabber", "ImageGrabber", string("left"));

	catalog.CheckWiring();

	shared_ptr<IImageGrabber> imageGrabber = catalog["imgGrabber"];

	for (int i = 0; i < 10; i++) {
		auto result = imageGrabber->getFrame(true);
		BOOST_MESSAGE("Frame wait no. " << result.first);

		result = imageGrabber->getFrame(false);
		BOOST_MESSAGE("Frame nowait no. " << result.first);
	}

	BOOST_CHECK_EQUAL(1, 1);
}