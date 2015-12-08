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

find_package(Boost COMPONENTS system unit_test_framework program_options filesystem REQUIRED)
find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

pkg_check_modules(LOG4CPP "log4cpp")

include_directories(${LOG4CPP_INCLUDE_DIRS})

include_directories("src")

set(SRC
        src/CameraImageGrabber.cpp src/CameraImageGrabber.hpp
        src/Painter.hpp
        src/GripperHudPainter.cpp src/GripperHudPainter.hpp
        src/HeadHudPainter.cpp src/HeadHudPainter.hpp
        src/ImageSource.hpp
        src/GripperImageSource.cpp src/GripperImageSource.hpp
        src/HeadImageSource.cpp src/HeadImageSource.hpp
        src/NetworkServer.cpp src/NetworkServer.hpp
        )

add_executable(${MAIN_EXECUTABLE} ${SRC} src/main.cpp)
target_link_libraries(${MAIN_EXECUTABLE} ${COMMON_LIB})
target_link_libraries(${MAIN_EXECUTABLE} ${LOG4CPP_LIBRARIES})
target_link_libraries(${MAIN_EXECUTABLE} ${Boost_LIBRARIES})
target_link_libraries(${MAIN_EXECUTABLE} ${OpenCV_LIBRARIES})
target_link_libraries(${MAIN_EXECUTABLE} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(${MAIN_EXECUTABLE} rt)

add_executable(szark_camera_server_framegrabber ${SRC} src/main_framegrabber.cpp)
target_link_libraries(szark_camera_server_framegrabber ${COMMON_LIB})
target_link_libraries(szark_camera_server_framegrabber ${LOG4CPP_LIBRARIES})
target_link_libraries(szark_camera_server_framegrabber ${Boost_LIBRARIES})
target_link_libraries(szark_camera_server_framegrabber ${OpenCV_LIBRARIES})
target_link_libraries(szark_camera_server_framegrabber ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(szark_camera_server_framegrabber rt)

enable_testing()

file(GLOB test_src "test/*.hpp" "test/*.cpp")

add_executable(${TEST_EXECUTABLE} ${SRC} ${test_src})

target_link_libraries(${TEST_EXECUTABLE} ${COMMON_LIB})
target_link_libraries(${TEST_EXECUTABLE} ${LOG4CPP_LIBRARIES})
target_link_libraries(${TEST_EXECUTABLE} ${Boost_LIBRARIES})
target_link_libraries(${TEST_EXECUTABLE} ${OpenCV_LIBRARIES})
target_link_libraries(${TEST_EXECUTABLE} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(${TEST_EXECUTABLE} rt)

add_test(SZARKCameraServerTest ${TEST_EXECUTABLE})
