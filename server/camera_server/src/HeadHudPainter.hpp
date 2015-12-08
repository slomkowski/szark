#pragma once

#include <wallaroo/device.h>
#include <log4cpp/Category.hh>

#include "Configuration.hpp"
#include "Painter.hpp"

namespace camera {
	class HeadHudPainter : public IPainter, public wallaroo::Device {
	public:
		HeadHudPainter();

		virtual ~HeadHudPainter();

		virtual cv::Mat drawContent(cv::Mat rawImage, boost::any additionalArg);

	private:
		log4cpp::Category &logger;
		wallaroo::Plug<common::config::Configuration> config;
	};
}
