#ifndef _CAMERAIMAGECOMBINER_HPP_
#define _CAMERAIMAGECOMBINER_HPP_

#include <boost/noncopyable.hpp>
#include <vips/vips>
#include <wallaroo/device.h>
#include <log4cpp/Category.hh>
#include <functional>

#include "CameraImageGrabber.hpp"

namespace camera {
	class ImageCombinerException : std::runtime_error {
	public:
		ImageCombinerException(const std::string &message)
				: std::runtime_error(message) {
		}
	};

	typedef std::function<void(void *, int)> EncodedImageProcessor;

	class IImageCombiner : boost::noncopyable {
	public:
		virtual ~IImageCombiner() = default;

		virtual vips::VImage getCombinedImage(bool drawHud) = 0;

		virtual void getEncodedImage(bool drawHud, EncodedImageProcessor processor) = 0;
	};

	class ImageCombiner : public IImageCombiner, public wallaroo::Device {
	public:
		ImageCombiner();

		virtual ~ImageCombiner();

		virtual vips::VImage getCombinedImage(bool drawHud);

		virtual void getEncodedImage(bool drawHud, EncodedImageProcessor processor);

	private:
		log4cpp::Category &logger;

		wallaroo::Plug<IImageGrabber> leftCameraGrabber;
		wallaroo::Plug<IImageGrabber> rightCameraGrabber;
	};
}

#endif