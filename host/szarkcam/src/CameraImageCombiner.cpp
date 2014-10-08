#include "CameraImageCombiner.hpp"
#include "utils.hpp"
#include <opencv2/opencv.hpp>
#include <boost/format.hpp>

const int JPEG_QUALITY = 45;


using namespace camera;

WALLAROO_REGISTER(ImageCombiner);

camera::ImageCombiner::ImageCombiner()
		: logger(log4cpp::Category::getInstance("ImageCombiner")),
		  leftCameraGrabber("leftCameraGrabber", RegistrationToken()),
		  rightCameraGrabber("rightCameraGrabber", RegistrationToken()) {

	jpegEncoderParameters.push_back(CV_IMWRITE_JPEG_QUALITY);
	jpegEncoderParameters.push_back(JPEG_QUALITY);

	logger.notice("Instance created.");
}

camera::ImageCombiner::~ImageCombiner() {
	logger.notice("Instance destroyed.");
}

//TODO pewnie osobny wątek
cv::Mat camera::ImageCombiner::getCombinedImage(bool drawHud) {
	auto leftFrame = leftCameraGrabber->getFrame(true).second;
	auto rightFrame = rightCameraGrabber->getFrame(false).second;

	cv::Mat result;

	int us = utils::measureTime<std::chrono::microseconds>([&]() {
		using namespace cv;

		flip(leftFrame, leftFrame, 0);
		transpose(leftFrame, leftFrame);

		flip(leftFrame, leftFrame, 0);
		transpose(rightFrame, rightFrame);

		Size sizeLeft = leftFrame.size();
		Size sizeRight = rightFrame.size();

		Mat im3(sizeLeft.height, sizeLeft.width + sizeRight.width, CV_8UC3);

		Mat left(im3, Rect(0, 0, sizeLeft.width, sizeLeft.height));
		leftFrame.copyTo(left);

		Mat right(im3, Rect(sizeLeft.width, 0, sizeRight.width, sizeRight.height));
		rightFrame.copyTo(right);

		result = im3;
	});

	logger.info((boost::format("Combined image in %u us.") % us).str());

	return result;
}

void ImageCombiner::getEncodedImage(bool drawHud, EncodedImageProcessor processor) {

	auto img = getCombinedImage(drawHud);

	cv::vector<unsigned char> buffer;

	int us = utils::measureTime<std::chrono::microseconds>([&]() {
		cv::imencode(".jpg", img, buffer, jpegEncoderParameters);
	});

	logger.info((boost::format("Converted image to JPEG in %u us.") % us).str());

	processor(&buffer[0], buffer.size());
}
