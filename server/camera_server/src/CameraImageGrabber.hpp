#pragma once

#include "Configuration.hpp"

#include <boost/noncopyable.hpp>
#include <boost/circular_buffer.hpp>
#include <opencv2/opencv.hpp>
#include <log4cpp/Category.hh>
#include <wallaroo/part.h>
#include <wallaroo/registered.h>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <utility>

namespace camera {
    class ImageGrabberException : public std::runtime_error {
    public:
        ImageGrabberException(const std::string &message)
                : std::runtime_error(message) {
        }
    };

    enum class FlipParams {
        NONE,
        FLIP_VERTICALLY,
        FLIP_HORIZONTALLY,
        ROTATE_180
    };

    class IImageGrabber : boost::noncopyable {
    public:
        virtual ~IImageGrabber() = default;

        virtual void setVideoParams(int input, FlipParams flipParams) = 0;

        // tuple: frame no, fps, image data
        virtual std::tuple<long, double, cv::Mat> getFrame(bool wait) = 0;
    };
}
