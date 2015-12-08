#include "utils.hpp"
#include "GripperImageSource.hpp"

#include <opencv2/opencv.hpp>
#include <boost/format.hpp>

#include  <utility>

using namespace camera;

WALLAROO_REGISTER(GripperImageSource);

camera::GripperImageSource::GripperImageSource()
        : logger(log4cpp::Category::getInstance("GripperImageSource")),
          config("config", RegistrationToken()),
          leftCameraGrabber("leftCameraGrabber", RegistrationToken()),
          rightCameraGrabber("rightCameraGrabber", RegistrationToken()),
          hudPainter("hudPainter", RegistrationToken()) {

    logger.notice("Instance created.");
}

camera::GripperImageSource::~GripperImageSource() {
    logger.notice("Instance destroyed.");
}

cv::Mat camera::GripperImageSource::getImage(bool drawHud) {
    long leftFrameNo, rightFrameNo;
    double leftFps, rightFps;
    cv::Mat leftFrame, rightFrame;

    std::tie(leftFrameNo, leftFps, leftFrame) = leftCameraGrabber->getFrame(leftCameraIsFaster);
    std::tie(rightFrameNo, rightFps, rightFrame) = rightCameraGrabber->getFrame(not leftCameraIsFaster);

    bool newLeftIsFaster = leftFps > rightFps;

    if (newLeftIsFaster != leftCameraIsFaster) {
        logger.debug((boost::format("Set %s camera as faster.") % (newLeftIsFaster ? "left" : "right")).str());
    }

    leftCameraIsFaster = newLeftIsFaster;

    cv::Mat result;

    int us = common::utils::measureTime<std::chrono::microseconds>([&]() {
        using namespace cv;

        Mat transLeft;
        transpose(leftFrame, transLeft);
        flip(transLeft, transLeft, 0);

        Mat transRight;
        transpose(rightFrame, transRight);
        flip(transRight, transRight, 1);

        Size sizeLeft = transLeft.size();
        Size sizeRight = transRight.size();

        Mat im3(sizeLeft.height, sizeLeft.width + sizeRight.width, leftFrame.type());

        Mat left(im3, Rect(0, 0, sizeLeft.width, sizeLeft.height));
        transLeft.copyTo(left);

        Mat right(im3, Rect(sizeLeft.width, 0, sizeRight.width, sizeRight.height));
        transRight.copyTo(right);

        result = im3;
    });

    logger.info((boost::format("Combined image in %u us.") % us).str());

    if (drawHud) {
        result = hudPainter->drawContent(result, std::make_pair(leftFrameNo, rightFrameNo));
    }

    return result;
}

