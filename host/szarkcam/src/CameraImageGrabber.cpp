#include <opencv2/opencv.hpp>
#include <boost/format.hpp>
#include "CameraImageGrabber.hpp"
#include "utils.hpp"
#include "Configuration.hpp"

using namespace camera;
using namespace boost;

const int FRAMERATE_AVG_FRAMES = 5;

WALLAROO_REGISTER(ImageGrabber, std::string);

camera::ImageGrabber::ImageGrabber(const std::string &prefix) :
		logger(log4cpp::Category::getInstance("ImageGrabber")),
		captureTimesAvgBuffer(FRAMERATE_AVG_FRAMES),
		currentFrameNo(1) {

	int videoDevice = config::getInt("szark.server.camera.ImageGrabber." + prefix + ".device");

	videoCapture.reset(new cv::VideoCapture(videoDevice));

	if (not videoCapture->isOpened()) {
		throw ImageGrabberException((boost::format("cannot open device %d.") % videoDevice).str());
	}

	videoCapture->set(CV_CAP_PROP_FRAME_WIDTH, 352);
	videoCapture->set(CV_CAP_PROP_FRAME_HEIGHT, 288);

	grabberThread.reset(new std::thread(&ImageGrabber::grabberThreadFunction, this));

	logger.notice("Instance created.");
}

std::tuple<long, double, cv::Mat>  camera::ImageGrabber::getFrame(bool wait) {
	std::unique_lock<std::mutex> lock(mutex);

	if (not wait) {
		cond.wait(lock);
		logger.info("Got frame.");
	} else {
		logger.info("Got frame without waiting.");
	}

	return std::tuple<long, double, cv::Mat>(currentFrameNo, currentFps, currentFrame);
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
		bool validFrame;
		int elapsedTime = utils::measureTime<std::chrono::milliseconds>([&]() {
			validFrame = videoCapture->read(frame);
		});

		if (not validFrame) {
			throw ImageGrabberException("invalid frame");
		}

		captureTimesAvgBuffer.push_back(elapsedTime);

		double fps = captureTimesAvgBuffer.size() * 1000.0 / (std::accumulate(captureTimesAvgBuffer.begin(), captureTimesAvgBuffer.end(), 0));

		logger.info((format("Captured frame no %d in %d ms (%2.1f fps).") % currentFrameNo % elapsedTime % fps).str());

		mutex.lock();
		this->currentFrame = frame;
		this->currentFrameNo++;
		this->currentFps = fps;
		mutex.unlock();

		cond.notify_all();
	}
}

