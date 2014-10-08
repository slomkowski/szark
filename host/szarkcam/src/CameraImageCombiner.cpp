#include "CameraImageCombiner.hpp"
#include "utils.hpp"
#include <vips/vips.h>
#include <boost/format.hpp>

using namespace camera;

WALLAROO_REGISTER(ImageCombiner);

camera::ImageCombiner::ImageCombiner()
		: logger(log4cpp::Category::getInstance("ImageCombiner")),
		  leftCameraGrabber("leftCameraGrabber", RegistrationToken()),
		  rightCameraGrabber("rightCameraGrabber", RegistrationToken()) {
	logger.notice("Instance created.");
}

camera::ImageCombiner::~ImageCombiner() {
	logger.notice("Instance destroyed.");
}

//TODO pewnie osobny wątek
vips::VImage camera::ImageCombiner::getCombinedImage(bool drawHud) {
	auto leftPair = leftCameraGrabber->getFrame(true);
	auto rightPair = rightCameraGrabber->getFrame(false);

	return leftPair.second.lrjoin(rightPair.second);
}

void ImageCombiner::getEncodedImage(bool drawHud, EncodedImageProcessor processor) {
	auto img = getCombinedImage(drawHud);
	void *bufferPointer;
	size_t length;

	int milliseconds = utils::measureTime<std::chrono::milliseconds>([&]() {
		vips_jpegsave_buffer(img.image(), &bufferPointer, &length, nullptr);
	});

	logger.info((boost::format("Converted image to JPEG in %u ms.") % milliseconds).str());

	processor(bufferPointer, length);

	g_free(bufferPointer);
}
