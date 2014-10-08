#ifndef _CAMERAIMAGEGRABBER_HPP_
#define _CAMERAIMAGEGRABBER_HPP_

#include <boost/noncopyable.hpp>
#include <vips/vips>
#include <log4cpp/Category.hh>
#include <wallaroo/device.h>
#include <wallaroo/registered.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <utility>

namespace camera {
	class IImageGrabber : boost::noncopyable {
	public:
		virtual ~IImageGrabber() = default;

		virtual std::pair<long, vips::VImage> getFrame(bool wait) = 0;
	};

	class ImageGrabber : public IImageGrabber, public wallaroo::Device {
	public:
		ImageGrabber(const std::string &prefix);

		virtual ~ImageGrabber();

		virtual std::pair<long, vips::VImage> getFrame(bool wait);

	private:
		log4cpp::Category &logger;

		std::unique_ptr<std::thread> grabberThread;

		std::mutex mutex;
		std::condition_variable cond;
		volatile bool finishThread = false;

		void grabberThreadFunction();
	};
}

#endif