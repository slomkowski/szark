#ifndef _INITIALIZATION_HPP_
#define _INITIALIZATION_HPP_

#include <string>
#include <vector>

namespace common {
	namespace init {
		std::vector<std::string> initializeProgram(int argc, char *argv[], std::string banner);
	}
}

#endif
