#include <opencv2/opencv.hpp>
#include <boost/format.hpp>
#include "CameraImageGrabber.hpp"
#include "utils.hpp"
#include "Configuration.hpp"

using namespace camera;
using namespace boost;

WALLAROO_REGISTER(ImageGrabber, std::string);

camera::ImageGrabber::ImageGrabber(const std::string &prefix) :
		logger(log4cpp::Category::getInstance("ImageGrabber")),
		currentFrameNo(1) {

	int videoDevice = config::getInt("szark.server.camera.ImageGrabber." + prefix + ".device");

	videoCapture.reset(new cv::VideoCapture(videoDevice));

	videoCapture->set(CV_CAP_PROP_FRAME_WIDTH, 352);
	videoCapture->set(CV_CAP_PROP_FRAME_HEIGHT, 288);

	grabberThread.reset(new std::thread(&ImageGrabber::grabberThreadFunction, this));

	logger.notice("Instance created.");
}

std::pair<long, cv::Mat> camera::ImageGrabber::getFrame(bool wait) {
	std::unique_lock<std::mutex> lock(mutex);

	if (not wait) {
		cond.wait(lock);
		logger.info("Got frame.");
	} else {
		logger.info("Got frame without waiting.");
	}

	return std::make_pair(currentFrameNo, currentFrame);
}

camera::ImageGrabber::~ImageGrabber() {
	mutex.lock();
	finishThread = true;
	mutex.unlock();

	grabberThread->join();
	logger.notice("Instance destroyed.");
}

void ImageGrabber::grabberThreadFunction() {
	while (!finishThread) {
		cv::Mat frame;
		auto elapsedTime = utils::measureTime<std::chrono::milliseconds>([&]() {
			videoCapture->read(frame);
		});

		logger.info((format("Captured frame no %d in %d ms.") % currentFrameNo % elapsedTime).str());

		mutex.lock();
		this->currentFrame = frame;
		this->currentFrameNo++;
		mutex.unlock();

		cond.notify_all();
	}
}
