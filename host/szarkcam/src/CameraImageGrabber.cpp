#include <opencv2/opencv.hpp>
#include <boost/format.hpp>
#include "CameraImageGrabber.hpp"
#include "utils.hpp"
#include "Configuration.hpp"

using namespace camera;
using namespace boost;

WALLAROO_REGISTER(ImageGrabber, std::string);

camera::ImageGrabber::ImageGrabber(const std::string &prefix) :
		logger(log4cpp::Category::getInstance("ImageGrabber")) {

	std::string videoDevice = config::getString("szark.server.camera.ImageGrabber." + prefix + ".device");

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

	long no = 1;
	auto img = cv::imread("k.png", CV_LOAD_IMAGE_COLOR);

	return std::make_pair(no, img);
}

camera::ImageGrabber::~ImageGrabber() {
	mutex.lock();
	finishThread = true;
	mutex.unlock();

	grabberThread->join();
	logger.notice("Instance destroyed.");
}

void ImageGrabber::grabberThreadFunction() {
	long frameNo = 1;

	while (!finishThread) {
		auto elapsedTime = utils::measureTime<std::chrono::milliseconds>([&]() {
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		});

		logger.info((format("Captured frame no %d in %d ms.") % frameNo % elapsedTime).str());

		mutex.lock();
		// TODO załadować do zmiennej
		mutex.unlock();

		cond.notify_all();

		frameNo++;
	}
}
