#include <opencv2/opencv.hpp>
#include <boost/format.hpp>
#include "CameraImageGrabber.hpp"
#include "utils.hpp"
#include "Configuration.hpp"
#include <pthread.h>
#include <cstring>

using namespace std;
using namespace boost;
using namespace camera;

const int FRAMERATE_AVG_FRAMES = 5;

WALLAROO_REGISTER(ImageGrabber, string);

camera::ImageGrabber::ImageGrabber(const std::string &prefix) :
		prefix(prefix),
		logger(log4cpp::Category::getInstance("ImageGrabber")),
		captureTimesAvgBuffer(FRAMERATE_AVG_FRAMES),
		currentFrameNo(1) {

	int videoDevice = config::getInt(getFullConfigPath("device"));

	videoCapture.reset(new cv::VideoCapture(videoDevice));

	if (not videoCapture->isOpened()) {
		throw ImageGrabberException((format("cannot open device %d.") % videoDevice).str());
	}

	setVideoCaptureProperty(CV_CAP_PROP_FRAME_WIDTH, "width");
	setVideoCaptureProperty(CV_CAP_PROP_FRAME_HEIGHT, "height");

	logger.notice((format("Set '%s' frame size to %dx%d.") % prefix
			% videoCapture->get(CV_CAP_PROP_FRAME_WIDTH)
			% videoCapture->get(CV_CAP_PROP_FRAME_HEIGHT)).str());

	grabberThread.reset(new std::thread(&ImageGrabber::grabberThreadFunction, this));
	int result = pthread_setname_np(grabberThread->native_handle(), (prefix + "ImgGrab").c_str());
	if (result != 0) {
		logger.error((format("Cannot set thread name: %s.") % strerror(result)).str());
	}

	logger.notice("Instance created.");
}

std::tuple<long, double, cv::Mat>  camera::ImageGrabber::getFrame(bool wait) {
	std::unique_lock<std::mutex> lk(dataMutex);

	if (not wait) {
		cond.wait(lk);
		logger.info("Got frame.");
	} else {
		logger.info("Got frame without waiting.");
	}

	return std::tuple<long, double, cv::Mat>(currentFrameNo, currentFps, currentFrame);
}

camera::ImageGrabber::~ImageGrabber() {
	dataMutex.lock();
	finishThread = true;
	dataMutex.unlock();

	grabberThread->join();

	videoCapture->release();

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

		dataMutex.lock();
		this->currentFrame = frame;
		this->currentFrameNo++;
		this->currentFps = fps;
		dataMutex.unlock();

		cond.notify_all();
	}
}

std::string ImageGrabber::getFullConfigPath(std::string property) {
	return "szark.server.camera.ImageGrabber." + prefix + "." + property;
}

void ImageGrabber::setVideoCaptureProperty(int prop, std::string confName) {
	auto path = getFullConfigPath(confName);
	int val;

	try {
		val = config::getInt(path);
	} catch (config::ConfigException &e) {
		throw ImageGrabberException((format("failed to set property: %s") % e.what()).str());
	}

	videoCapture->set(prop, val);

//	if (readVal != val) {
//		throw ImageGrabberException((format("failed to set property '%s' to value %d, previous value: %d")
//				% path % val % readVal).str());
//	}

	logger.info((format("Set property '%s' to value %d.") % path % val).str());
}
