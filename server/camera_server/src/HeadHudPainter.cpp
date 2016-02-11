#include "Configuration.hpp"
#include "Painter.hpp"
#include "SharedInterfaceProvider.hpp"

#include <opencv2/opencv.hpp>
#include <wallaroo/registered.h>
#include <boost/format.hpp>
#include <log4cpp/Category.hh>

using namespace cv;

namespace camera {
    const Scalar GREEN(0x55, 0xD4, 0x19);
    const Scalar RED(0x19, 0x1B, 0xD4);

    class HeadHudPainter : public IPainter, public wallaroo::Part {
    public:
        HeadHudPainter()
                : logger(log4cpp::Category::getInstance("HeadHudPainter")),
                  config("config", RegistrationToken()),
                  interfaceProvider("interfaceProvider", RegistrationToken()) {
            logger.notice("Instance created.");
        }

        virtual ~HeadHudPainter() {
            logger.notice("Instance destroyed.");
        }

        virtual cv::Mat drawContent(cv::Mat rawImage, boost::any additionalArg) {
            long frameNo;
            double fps;

            std::tie(frameNo, fps) = boost::any_cast<std::pair<long, double>>(additionalArg);

            putText(rawImage, std::to_string(frameNo).c_str(),
                    Point(30, 30), FONT_HERSHEY_SIMPLEX, 1, GREEN, 4);

            putText(rawImage, (boost::format("%.1f") % fps).str().c_str(),
                    Point(30, 63), FONT_HERSHEY_SIMPLEX, 1, RED, 4);

            if (interfaceProvider->isValid()) {
                drawMotorInfo(rawImage);
            }

            return rawImage;
        }

    private:
        log4cpp::Category &logger;
        wallaroo::Collaborator<common::config::Configuration> config;
        wallaroo::Collaborator<common::bridge::InterfaceProvider> interfaceProvider;

        void drawMotorInfo(cv::Mat &img) {
            using namespace common::bridge;
            const Point size(100, 20);

            const Point pivot(img.cols / 2, img.rows - 40);

            auto drawMotorSpeed = [&](Interface::MotorClass::SingleMotor &motor,
                                      Point offset,
                                      bool invert) {

                auto color = motor.getDirection() == Direction::BACKWARD ? RED : GREEN;

                drawProgressBar(img, pivot + offset, size, color,
                                static_cast<float>(motor.getSpeed()) / MOTOR_DRIVER_MAX_SPEED, invert);
            };

            drawMotorSpeed(interfaceProvider->getInterface()->motor.left, Point(-120, 0), true);
            drawMotorSpeed(interfaceProvider->getInterface()->motor.right, Point(20, 0), false);
        }

        void drawProgressBar(cv::Mat &img,
                             cv::Point topLeft,
                             cv::Point size,
                             cv::Scalar color,
                             float progress,
                             bool invert = false) {

            rectangle(img, topLeft, topLeft + size, color, 2);

            if (invert) {
                Point filledAreaOffset(size.x * (1.0 - progress), 0);
                rectangle(img, topLeft + filledAreaOffset, topLeft + size, color, CV_FILLED);
            } else {
                Point filledAreaSize(size.x * progress, size.y);
                rectangle(img, topLeft, topLeft + filledAreaSize, color, CV_FILLED);
            }
        }
    };

    WALLAROO_REGISTER(HeadHudPainter);
}
