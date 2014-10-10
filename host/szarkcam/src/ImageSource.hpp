#ifndef _IMAGESOURCE_HPP_
#define  _IMAGESOURCE_HPP_

#include <stdexcept>
#include <functional>
#include <opencv2/opencv.hpp>
#include <boost/noncopyable.hpp>

namespace camera {
	class ImageSourceException : public std::runtime_error {
	public:
		ImageSourceException(const std::string &message)
				: std::runtime_error(message) {
		}
	};

	typedef std::function<void(void *, size_t)> EncodedImageProcessor;

	class IImageSource : boost::noncopyable {
	public:
		virtual ~IImageSource() = default;

		virtual cv::Mat getImage(bool drawHud) = 0;

		virtual void getEncodedImage(bool drawHud, EncodedImageProcessor processor) = 0;
	};
}

#endif