#include "ImageSource.hpp"
#include "Painter.hpp"
#include "CameraImageGrabber.hpp"

#include "utils.hpp"

#include <wallaroo/part.h>
#include <log4cpp/Category.hh>

#include <opencv2/opencv.hpp>
#include <boost/format.hpp>

using namespace camera;

namespace camera {

    class HeadImageSource : public IImageSource, public wallaroo::Part {
    private:
        log4cpp::Category &logger;

        wallaroo::Collaborator<common::config::Configuration> config;
        wallaroo::Collaborator<IImageGrabber> cameraGrabber;

        wallaroo::Collaborator<IPainter> hudPainter;

        int gripperInputNo;
        int backInputNo;

    public:
        HeadImageSource()
                : logger(log4cpp::Category::getInstance("HeadImageSource")),
                  config("config", RegistrationToken()),
                  cameraGrabber("cameraGrabber", RegistrationToken()),
                  hudPainter("hudPainter", RegistrationToken()) {
        }

        virtual void Init() {
            if (not cv::useOptimized()) {
                logger.warn("OpenCV doesn't use optimized functions.");
            }

            gripperInputNo = config->getInt("HeadImageSource.input_gripper_no");
            backInputNo = config->getInt("HeadImageSource.input_back_no");

            logger.notice("Instance created.");
        }

        virtual ~HeadImageSource() {
            logger.notice("Instance destroyed.");
        }

        virtual cv::Mat getImage(std::string &videoInput, bool drawHud) {

            FlipParams flipParams;
            int input = 0;

            if (videoInput == "back") {
                input = backInputNo;
                flipParams = FlipParams::ROTATE_180;
                logger.info("Taking frame from back camera, rotate 180.");
            } else {
                input = gripperInputNo;
                flipParams = FlipParams::NONE;
                logger.info("Taking frame from gripper camera.");
            }

            long frameNo;
            double fps;
            cv::Mat frame;

            cameraGrabber->setVideoParams(input, flipParams);
            std::tie(frameNo, fps, frame) = cameraGrabber->getFrame(true);

            if (drawHud) {
                frame = hudPainter->drawContent(frame, std::make_pair(frameNo, fps));
            }

            return frame;
        }
    };
}

WALLAROO_REGISTER(HeadImageSource);
