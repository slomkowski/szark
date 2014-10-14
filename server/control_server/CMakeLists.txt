#
# CMake configuration for SZARK Control Server
# (C) 2014 Michał Słomkowski
# License: GPL v3
#

cmake_minimum_required(VERSION 2.6)
project(SZARK_control_server)

set(MAIN_EXECUTABLE szark_control_server)
set(TEST_EXECUTABLE szark_control_server_test)

set(EXTERNAL_LIBS_DIR "../../external-libs/")

list(APPEND CMAKE_CXX_FLAGS "-std=c++11 -Wall -Og -g ${CMAKE_CXX_FLAGS}")

INCLUDE(FindPkgConfig)

find_package(Boost COMPONENTS system unit_test_framework REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(LIBUSB "libusb-1.0")
pkg_check_modules(LOG4CPP "log4cpp")
pkg_check_modules(JSON "jsoncpp")

include_directories(${LIBUSB_INCLUDE_DIRS})
include_directories(${LOG4CPP_INCLUDE_DIRS})
include_directories(${JSON_INCLUDE_DIRS})

include_directories("../../firmware/common")
include_directories(${EXTERNAL_LIBS_DIR}/wallaroo)

include_directories("src")

file(GLOB src "src/*.hpp" "src/*.cpp")
list(REMOVE_ITEM src "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

file(GLOB main_file "src/main.cpp")
add_executable(${MAIN_EXECUTABLE} ${src} ${main_file})

target_link_libraries(${MAIN_EXECUTABLE} ${LIBUSB_LIBRARIES})
target_link_libraries(${MAIN_EXECUTABLE} ${LOG4CPP_LIBRARIES})
target_link_libraries(${MAIN_EXECUTABLE} ${JSON_LIBRARIES})
target_link_libraries(${MAIN_EXECUTABLE} ${Boost_LIBRARIES})

enable_testing()

file(GLOB test_src "test/*.hpp" "test/*.cpp")

add_executable(${TEST_EXECUTABLE} ${src} ${test_src})

target_link_libraries(${TEST_EXECUTABLE} ${LIBUSB_LIBRARIES})
target_link_libraries(${TEST_EXECUTABLE} ${LOG4CPP_LIBRARIES})
target_link_libraries(${TEST_EXECUTABLE} ${JSON_LIBRARIES})
target_link_libraries(${TEST_EXECUTABLE} ${Boost_LIBRARIES})

add_test(SZARKControlServerTest ${TEST_EXECUTABLE})
