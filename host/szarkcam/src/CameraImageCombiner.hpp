#ifndef _CAMERAIMAGECOMBINER_HPP_
#define _CAMERAIMAGECOMBINER_HPP_

#include <boost/noncopyable.hpp>
#include <opencv2/opencv.hpp>
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

	typedef std::function<void(void *, size_t)> EncodedImageProcessor;

	class IImageCombiner : boost::noncopyable {
	public:
		virtual ~IImageCombiner() = default;

		virtual cv::Mat getCombinedImage(bool drawHud) = 0;

		virtual void getEncodedImage(bool drawHud, EncodedImageProcessor processor) = 0;
	};

	class ImageCombiner : public IImageCombiner, public wallaroo::Device {
	public:
		ImageCombiner();

		virtual ~ImageCombiner();

		virtual cv::Mat getCombinedImage(bool drawHud);

		virtual void getEncodedImage(bool drawHud, EncodedImageProcessor processor);

	private:
		log4cpp::Category &logger;

		bool leftCameraIsFaster;

		std::vector<int> jpegEncoderParameters;

		wallaroo::Plug<IImageGrabber> leftCameraGrabber;
		wallaroo::Plug<IImageGrabber> rightCameraGrabber;
	};
}

#endif