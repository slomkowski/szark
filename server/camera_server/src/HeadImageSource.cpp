#include  <utility>

#include <opencv2/opencv.hpp>
#include <boost/format.hpp>

#include "utils.hpp"
#include "HeadImageSource.hpp"

const int JPEG_QUALITY = 45;

using namespace camera;

WALLAROO_REGISTER(HeadImageSource);

camera::HeadImageSource::HeadImageSource()
        : logger(log4cpp::Category::getInstance("HeadImageSource")),
          config("config", RegistrationToken()),
          cameraGrabber("cameraGrabber", RegistrationToken()),
          hudPainter("hudPainter", RegistrationToken()) {

    jpegEncoderParameters.push_back(CV_IMWRITE_JPEG_QUALITY);
    jpegEncoderParameters.push_back(JPEG_QUALITY);

    logger.notice("Instance created.");
}

camera::HeadImageSource::~HeadImageSource() {
    logger.notice("Instance destroyed.");
}

cv::Mat camera::HeadImageSource::getImage(bool drawHud) {
    long frameNo;
    double fps;
    cv::Mat frame;

    std::tie(frameNo, fps, frame) = cameraGrabber->getFrame(leftCameraIsFaster);

    if (drawHud) {
        frame = hudPainter->drawContent(frame, frameNo);
    }

    return frame;
}

void HeadImageSource::getEncodedImage(bool drawHud, EncodedImageProcessor processor) {

    auto img = getImage(drawHud);

    cv::vector<unsigned char> buffer(30000);

    int us = common::utils::measureTime<std::chrono::microseconds>([&]() {
        cv::imencode(".jpg", img, buffer, jpegEncoderParameters);
    });

    logger.info((boost::format("Converted image to JPEG in %u us.") % us).str());

    processor(&buffer[0], buffer.size());
}
