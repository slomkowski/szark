#pragma once

#include <opencv2/opencv.hpp>
#include <boost/noncopyable.hpp>

#include <stdexcept>
#include <functional>

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

        virtual cv::Mat getImage(std::string &videoInput, bool drawHud) = 0;
    };
}
