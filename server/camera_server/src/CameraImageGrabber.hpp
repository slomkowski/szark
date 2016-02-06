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

    class IImageGrabber : boost::noncopyable {
    public:
        virtual ~IImageGrabber() = default;

        // tuple: frame no, fps, image data
        virtual std::tuple<long, double, cv::Mat> getFrame(bool wait) = 0;
    };

    class ImageGrabber : public IImageGrabber, public wallaroo::Part {
    public:
        ImageGrabber(const std::string &prefix);

        virtual ~ImageGrabber();

        virtual std::tuple<long, double, cv::Mat> getFrame(bool wait);

    private:
        std::string prefix;

        log4cpp::Category &logger;

        wallaroo::Collaborator<common::config::Configuration> config;

        boost::circular_buffer<double> captureTimesAvgBuffer;

        std::unique_ptr<cv::VideoCapture> videoCapture;

        std::unique_ptr<std::thread> grabberThread;

        std::mutex dataMutex;
        std::condition_variable cond;

        cv::Mat currentFrame;
        int currentFrameNo;
        double currentFps;

        volatile bool finishThread = false;

        virtual void Init();

        void grabberThreadFunction();

        std::string getFullConfigPath(std::string property);

        void setVideoCaptureProperty(int prop, std::string confName);
    };
}
