#ifndef _PAINTER_HPP_
#define _PAINTER_HPP_

#include <vector>
#include <stdexcept>
#include <opencv2/opencv.hpp>
#include <boost/noncopyable.hpp>
#include <boost/any.hpp>

namespace camera {
    class PainterException : public std::runtime_error {
    public:
        PainterException(const std::string &message)
                : std::runtime_error(message) {
        }
    };

    class IPainter : boost::noncopyable {
    public:
        virtual cv::Mat drawContent(cv::Mat rawImage, boost::any additionalArg) = 0;

        virtual ~IPainter() = default;
    };
}

#endif