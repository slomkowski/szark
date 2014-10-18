#ifndef _GRIPPERHUDPAINTER_HPP_
#define _GRIPPERHUDPAINTER_HPP_

#include <wallaroo/device.h>
#include <log4cpp/Category.hh>

#include "Configuration.hpp"
#include "Painter.hpp"

namespace camera {
	class GripperHudPainter : public IPainter, public wallaroo::Device {
	public:
		GripperHudPainter();

		virtual ~GripperHudPainter();

		virtual cv::Mat drawContent(cv::Mat rawImage, boost::any additionalArg);

	private:
		log4cpp::Category &logger;
		wallaroo::Plug<common::config::Configuration> config;
	};
}

#endif