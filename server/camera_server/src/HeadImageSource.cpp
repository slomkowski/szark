#include "utils.hpp"
#include "HeadImageSource.hpp"

#include <opencv2/opencv.hpp>
#include <boost/format.hpp>

#include  <utility>

using namespace camera;

WALLAROO_REGISTER(HeadImageSource);

camera::HeadImageSource::HeadImageSource()
        : logger(log4cpp::Category::getInstance("HeadImageSource")),
          config("config", RegistrationToken()),
          cameraGrabber("cameraGrabber", RegistrationToken()),
          hudPainter("hudPainter", RegistrationToken()) {

    logger.notice("Instance created.");
}

camera::HeadImageSource::~HeadImageSource() {
    logger.notice("Instance destroyed.");
}

cv::Mat camera::HeadImageSource::getImage(bool drawHud) {
    long frameNo;
    double fps;
    cv::Mat frame;

    std::tie(frameNo, fps, frame) = cameraGrabber->getFrame(true);

    if (drawHud) {
        frame = hudPainter->drawContent(frame, frameNo);
    }

    return frame;
}
