#ifndef _GRIPPERHUDPAINTER_HPP_
#define _GRIPPERHUDPAINTER_HPP_

#include <wallaroo/device.h>
#include <log4cpp/Category.hh>

#include "Painter.hpp"

namespace camera {
	class GripperHudPainter : public IPainter, public wallaroo::Device {
	public:
		GripperHudPainter();

		virtual ~GripperHudPainter();

		virtual cv::Mat drawContent(cv::Mat rawImage, boost::any additionalArg);

	private:
		log4cpp::Category &logger;
	};
}

#endif