#ifndef _GRIPPERIMAGESOURCE_HPP_
#define _GRIPPERIMAGESOURCE_HPP_

#include <opencv2/opencv.hpp>
#include <wallaroo/device.h>
#include <log4cpp/Category.hh>

#include "ImageSource.hpp"
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

		wallaroo::Plug<IImageGrabber> leftCameraGrabber;
		wallaroo::Plug<IImageGrabber> rightCameraGrabber;
	};
}

#endif