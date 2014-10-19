#ifndef _COLORPATTERNLAYOUT_HPP_
#define  _COLORPATTERNLAYOUT_HPP_

#include <log4cpp/PatternLayout.hh>

namespace common {
	namespace logger {
		class ColorPatternLayout : public log4cpp::PatternLayout {
		public:
			ColorPatternLayout();

			virtual ~ColorPatternLayout();
		};
	}
}

#endif