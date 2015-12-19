#pragma once

#include <string>
#include <vector>

namespace common {
    namespace init {
        std::vector<std::string> initializeProgram(int argc, char *argv[], std::string banner);
    }
}
