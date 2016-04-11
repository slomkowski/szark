#include "utils.hpp"
#include "GripperImageSource.hpp"

using namespace camera;

WALLAROO_REGISTER(GripperImageSource);

camera::GripperImageSource::GripperImageSource()
        : logger(log4cpp::Category::getInstance("GripperImageSource")),
          config("config", RegistrationToken()),
          leftCameraGrabber("leftCameraGrabber", RegistrationToken()),
          rightCameraGrabber("rightCameraGrabber", RegistrationToken()),
          hudPainter("hudPainter", RegistrationToken()) {

    if (not cv::useOptimized()) {
        logger.warn("OpenCV doesn't use optimized functions.");
    }

    logger.notice("Instance created.");
}

camera::GripperImageSource::~GripperImageSource() {
    logger.notice("Instance destroyed.");
}

cv::Mat camera::GripperImageSource::getImage(std::string &videoInput, bool drawHud) {
    long leftFrameNo, rightFrameNo;
    double leftFps, rightFps;
    cv::Mat leftFrame, rightFrame;

    videoInput = "default";

    std::tie(leftFrameNo, leftFps, leftFrame) = leftCameraGrabber->getFrame(leftCameraIsFaster);
    std::tie(rightFrameNo, rightFps, rightFrame) = rightCameraGrabber->getFrame(not leftCameraIsFaster);

    bool newLeftIsFaster = leftFps > rightFps;

    if (newLeftIsFaster != leftCameraIsFaster) {
        logger.debug("Set %s camera as faster.", (newLeftIsFaster ? "left" : "right"));
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

    logger.info("Combined image in %u us.", us);

    if (drawHud) {
        result = hudPainter->drawContent(result, std::make_pair(leftFrameNo, rightFrameNo));
    }

    return result;
}

