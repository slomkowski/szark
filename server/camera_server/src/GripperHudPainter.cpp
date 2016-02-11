#include "GripperHudPainter.hpp"

#include <opencv2/opencv.hpp>
#include <wallaroo/registered.h>

using namespace std;
using namespace boost;
using namespace camera;

WALLAROO_REGISTER(GripperHudPainter)

camera::GripperHudPainter::GripperHudPainter()
        : logger(log4cpp::Category::getInstance("GripperHudPainter")),
          config("config", RegistrationToken()) {
    logger.notice("Instance created.");
}

camera::GripperHudPainter::~GripperHudPainter() {
    logger.notice("Instance destroyed.");
}

cv::Mat camera::GripperHudPainter::drawContent(cv::Mat rawImage, boost::any additionalArg) {
    long leftFrameNo, rightFrameNo;

    tie(leftFrameNo, rightFrameNo) = any_cast<pair<long, long>>(additionalArg);

    using namespace cv;
    //TODO draw hud

    putText(rawImage, to_string(leftFrameNo).c_str(), Point(30, 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 200, 20), 4);
    putText(rawImage, to_string(rightFrameNo).c_str(), Point(300, 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 200, 20), 4);

    return rawImage;
}

