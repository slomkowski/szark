#ifndef _GRIPPERIMAGESOURCE_HPP_
#define _GRIPPERIMAGESOURCE_HPP_

#include <opencv2/opencv.hpp>
#include <wallaroo/device.h>
#include <log4cpp/Category.hh>
#include <Configuration.hpp>

#include "ImageSource.hpp"
#include "Painter.hpp"
#include "CameraImageGrabber.hpp"

namespace camera {

	class GripperImageSource : public IImageSource, public wallaroo::Device {
	public:
		GripperImageSource();

		virtual ~GripperImageSource();

		virtual cv::Mat getImage(bool drawHud);

		virtual void getEncodedImage(bool drawHud, EncodedImageProcessor processor);

	private:
		log4cpp::Category &logger;

		bool leftCameraIsFaster;

		std::vector<int> jpegEncoderParameters;

		wallaroo::Plug<common::config::Configuration> config;
		wallaroo::Plug<IImageGrabber> leftCameraGrabber;
		wallaroo::Plug<IImageGrabber> rightCameraGrabber;

		wallaroo::Plug<IPainter> hudPainter;
	};
}

#endif