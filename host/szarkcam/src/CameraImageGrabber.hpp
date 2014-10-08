#ifndef _CAMERAIMAGEGRABBER_HPP_
#define _CAMERAIMAGEGRABBER_HPP_

#include <boost/noncopyable.hpp>
#include <opencv2/opencv.hpp>
#include <log4cpp/Category.hh>
#include <wallaroo/device.h>
#include <wallaroo/registered.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <utility>

namespace camera {
	class ImageGrabberException : std::runtime_error {
	public:
		ImageGrabberException(const std::string &message)
				: std::runtime_error(message) {
		}
	};

	class IImageGrabber : boost::noncopyable {
	public:
		virtual ~IImageGrabber() = default;

		virtual std::pair<long, cv::Mat> getFrame(bool wait) = 0;
	};

	class ImageGrabber : public IImageGrabber, public wallaroo::Device {
	public:
		ImageGrabber(const std::string &prefix);

		virtual ~ImageGrabber();

		virtual std::pair<long, cv::Mat> getFrame(bool wait);

	private:
		log4cpp::Category &logger;

		std::unique_ptr<cv::VideoCapture> videoCapture;

		std::unique_ptr<std::thread> grabberThread;

		std::mutex mutex;
		std::condition_variable cond;

		cv::Mat currentFrame;
		int currentFrameNo;

		volatile bool finishThread = false;

		void grabberThreadFunction();
	};
}

#endif