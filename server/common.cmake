set(EXTERNAL_LIBS_DIR "../../external-libs/")

#set(WARNINGS "-Wall -Wextra -pedantic -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wundef -Wno-unused -Wno-variadic-macros -Wno-parentheses -fdiagnostics-show-option")

set(WARNINGS "-Wall -g")

list(APPEND CMAKE_CXX_FLAGS "-std=c++11 ${WARNINGS} ${CMAKE_CXX_FLAGS}")

INCLUDE(FindPkgConfig)

include_directories("../common_lib/include")
include_directories("../../firmware/common")
include_directories(${EXTERNAL_LIBS_DIR}/wallaroo)

find_library(COMMON_LIB szark_common "../common_lib/build")

#SET(COMMON_LIB -Wl,--whole-archive ${COMMON_LIB_PRIVATE} -Wl,--no-whole-archive)

