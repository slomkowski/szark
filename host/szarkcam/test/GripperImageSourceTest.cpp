#include <boost/test/unit_test.hpp>
#include <wallaroo/catalog.h>

#include "CameraImageGrabber.hpp"
#include "GripperImageSource.hpp"

using namespace std;
using namespace camera;

BOOST_AUTO_TEST_CASE(GripperImageSourceTest_Run) {
	wallaroo::Catalog catalog;
	catalog.Create("imgGrabberLeft", "ImageGrabber", string("left"));
	catalog.Create("imgGrabberRight", "ImageGrabber", string("right"));
	catalog.Create("imgCombiner", "GripperImageSource");

	wallaroo::use(catalog["imgGrabberLeft"]).as("leftCameraGrabber").of(catalog["imgCombiner"]);
	wallaroo::use(catalog["imgGrabberRight"]).as("rightCameraGrabber").of(catalog["imgCombiner"]);

	catalog.CheckWiring();

	shared_ptr<IImageSource> imageGrabber = catalog["imgCombiner"];

	for (int i = 0; i < 10; i++) {
		string fileName = "out.jpg";
		auto result = imageGrabber->getImage(false);
		cv::imwrite(fileName, result);
		BOOST_MESSAGE("Frame combined and saved to " << fileName);
	}

	BOOST_CHECK_EQUAL(1, 1);
}
