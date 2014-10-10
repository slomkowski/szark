#include  <utility>

#include <opencv2/opencv.hpp>
#include <boost/format.hpp>

#include "utils.hpp"
#include "GripperImageSource.hpp"

const int JPEG_QUALITY = 45;

using namespace camera;

WALLAROO_REGISTER(GripperImageSource);

camera::GripperImageSource::GripperImageSource()
		: logger(log4cpp::Category::getInstance("GripperImageSource")),
		  leftCameraGrabber("leftCameraGrabber", RegistrationToken()),
		  rightCameraGrabber("rightCameraGrabber", RegistrationToken()),
		  hudPainter("hudPainter", RegistrationToken()) {

	jpegEncoderParameters.push_back(CV_IMWRITE_JPEG_QUALITY);
	jpegEncoderParameters.push_back(JPEG_QUALITY);

	logger.notice("Instance created.");
}

camera::GripperImageSource::~GripperImageSource() {
	logger.notice("Instance destroyed.");
}

cv::Mat camera::GripperImageSource::getImage(bool drawHud) {
	long leftFrameNo, rightFrameNo;
	double leftFps, rightFps;
	cv::Mat leftFrame, rightFrame;

	std::tie(leftFrameNo, leftFps, leftFrame) = leftCameraGrabber->getFrame(leftCameraIsFaster);
	std::tie(rightFrameNo, rightFps, rightFrame) = rightCameraGrabber->getFrame(not leftCameraIsFaster);

	bool newLeftIsFaster = leftFps > rightFps;

	if (newLeftIsFaster != leftCameraIsFaster) {
		logger.debug((boost::format("Set %s camera as faster.") % (newLeftIsFaster ? "left" : "right")).str());
	}

	leftCameraIsFaster = newLeftIsFaster;

	cv::Mat result;

	int us = utils::measureTime<std::chrono::microseconds>([&]() {
		using namespace cv;

		//TODO obracanie
		//flip(leftFrame, leftFrame, 0);
		transpose(leftFrame, leftFrame);
		//flip(leftFrame, leftFrame, 0);

		transpose(rightFrame, rightFrame);

		Size sizeLeft = leftFrame.size();
		Size sizeRight = rightFrame.size();

		Mat im3(sizeLeft.height, sizeLeft.width + sizeRight.width, leftFrame.type());

		Mat left(im3, Rect(0, 0, sizeLeft.width, sizeLeft.height));
		leftFrame.copyTo(left);

		Mat right(im3, Rect(sizeLeft.width, 0, sizeRight.width, sizeRight.height));
		rightFrame.copyTo(right);

		result = im3;
	});

	logger.info((boost::format("Combined image in %u us.") % us).str());

	if (drawHud) {
		result = hudPainter->drawContent(result, std::make_pair(leftFrameNo, rightFrameNo));
	}

	return result;
}

void GripperImageSource::getEncodedImage(bool drawHud, EncodedImageProcessor processor) {

	auto img = getImage(drawHud);

	cv::vector<unsigned char> buffer(30000);

	int us = utils::measureTime<std::chrono::microseconds>([&]() {
		cv::imencode(".jpg", img, buffer, jpegEncoderParameters);
	});

	logger.info((boost::format("Converted image to JPEG in %u us.") % us).str());

	processor(&buffer[0], buffer.size());
}
