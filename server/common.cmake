set(EXTERNAL_LIBS_DIR "../../external-libs/")

list(APPEND CMAKE_CXX_FLAGS "-std=c++11 -Wall ${CMAKE_CXX_FLAGS}")

INCLUDE(FindPkgConfig)

include_directories("../common_lib/include")
include_directories("../../firmware/common")
include_directories(${EXTERNAL_LIBS_DIR}/wallaroo)

find_library(COMMON_LIB szark_common "../common_lib/build")

