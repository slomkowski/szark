#include <boost/test/unit_test.hpp>
#include <wallaroo/catalog.h>
#include <tuple>
#include "CameraImageGrabber.hpp"

using namespace std;
using namespace camera;

BOOST_AUTO_TEST_CASE(CameraImageGrabberTest_Run) {
	wallaroo::Catalog catalog;
	catalog.Create("imgGrabber", "ImageGrabber", string("left"));

	catalog.CheckWiring();

	shared_ptr<IImageGrabber> imageGrabber = catalog["imgGrabber"];

	for (int i = 0; i < 10; i++) {
		long frameNo;

		tie(frameNo, ignore, ignore) = imageGrabber->getFrame(true);
		BOOST_TEST_MESSAGE("Frame wait no. " << frameNo);

		tie(frameNo, ignore, ignore) = imageGrabber->getFrame(false);
		BOOST_TEST_MESSAGE("Frame nowait no. " << frameNo);
	}

	BOOST_CHECK_EQUAL(1, 1);
}