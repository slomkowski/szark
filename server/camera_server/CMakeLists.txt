#
# CMake configuration for SZARK Camera Server
# (C) 2014 Michał Słomkowski
# License: GPL v3
#

cmake_minimum_required(VERSION 2.6)
project(SZARK_camera_server)

set(MAIN_EXECUTABLE szark_camera_server)
set(TEST_EXECUTABLE szark_camera_server_test)

INCLUDE("../common.cmake")

find_package(Boost COMPONENTS system unit_test_framework REQUIRED)
find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(LOG4CPP "log4cpp")

include_directories(${LOG4CPP_INCLUDE_DIRS})

include_directories("src")

file(GLOB src "src/*.hpp" "src/*.cpp")
list(REMOVE_ITEM src "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

file(GLOB main_file "src/main.cpp")
add_executable(${MAIN_EXECUTABLE} ${src} ${main_file})

target_link_libraries(${MAIN_EXECUTABLE} ${COMMON_LIB})
target_link_libraries(${MAIN_EXECUTABLE} ${LOG4CPP_LIBRARIES})
target_link_libraries(${MAIN_EXECUTABLE} ${Boost_LIBRARIES})
target_link_libraries(${MAIN_EXECUTABLE} ${OpenCV_LIBRARIES})

enable_testing()

file(GLOB test_src "test/*.hpp" "test/*.cpp")

add_executable(${TEST_EXECUTABLE} ${src} ${test_src})

target_link_libraries(${TEST_EXECUTABLE} ${COMMON_LIB})
target_link_libraries(${TEST_EXECUTABLE} ${LOG4CPP_LIBRARIES})
target_link_libraries(${TEST_EXECUTABLE} ${Boost_LIBRARIES})
target_link_libraries(${TEST_EXECUTABLE} ${OpenCV_LIBRARIES})

add_test(SZARKCameraServerTest ${TEST_EXECUTABLE})
