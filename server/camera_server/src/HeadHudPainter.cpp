#include "HeadHudPainter.hpp"

#include <opencv2/opencv.hpp>
#include <wallaroo/registered.h>

using namespace std;
using namespace boost;
using namespace camera;

WALLAROO_REGISTER(HeadHudPainter);

camera::HeadHudPainter::HeadHudPainter()
        : logger(log4cpp::Category::getInstance("ImageGrabber")),
          config("config", RegistrationToken()) {
    logger.notice("Instance created.");
}

camera::HeadHudPainter::~HeadHudPainter() {
    logger.notice("Instance destroyed.");
}

cv::Mat camera::HeadHudPainter::drawContent(cv::Mat rawImage, boost::any additionalArg) {
    long frameNo = any_cast<long>(additionalArg);

    using namespace cv;
    //TODO draw hud

    putText(rawImage, to_string(frameNo).c_str(), Point(30, 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 200, 20), 4);

    return rawImage;
}

