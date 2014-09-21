cmake_minimum_required(VERSION 2.6)
project(SZARK_Server)

set(MAIN_EXECUTABLE szark_server)

list(APPEND CMAKE_CXX_FLAGS "-std=c++11 -Wall ${CMAKE_CXX_FLAGS}")

INCLUDE(FindPkgConfig)

find_package(Boost COMPONENTS system REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(LIBUSB "libusb-1.0")
pkg_check_modules(LOG4CPP "log4cpp")
pkg_check_modules(JSON "jsoncpp")

include_directories(${LIBUSB_INCLUDE_DIRS})
include_directories(${LOG4CPP_INCLUDE_DIRS})
include_directories(${JSON_INCLUDE_DIRS})

set(EXTERNAL_LIBS_DIR "../../external-libs/")

include_directories("../../firmware/common")
include_directories(${EXTERNAL_LIBS_DIR}/wallaroo)

file(GLOB src "src/*.hpp" "src/*.cpp")
add_executable(${MAIN_EXECUTABLE} ${src})

target_link_libraries(${MAIN_EXECUTABLE} ${LIBUSB_LIBRARIES})
target_link_libraries(${MAIN_EXECUTABLE} ${LOG4CPP_LIBRARIES})
target_link_libraries(${MAIN_EXECUTABLE} ${JSON_LIBRARIES})
target_link_libraries(${MAIN_EXECUTABLE} ${Boost_LIBRARIES})


