#
# CMake configuration for SZARK Camera Server
# (C) 2014, 2015 Michał Słomkowski
# License: GPL v3
#

cmake_minimum_required(VERSION 2.8.4)
project(SZARK_camera_server)

INCLUDE("../common.cmake")

find_package(Boost COMPONENTS system unit_test_framework program_options filesystem REQUIRED)
find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

pkg_check_modules(LOG4CPP "log4cpp")

include_directories(${LOG4CPP_INCLUDE_DIRS})

include_directories("src")

add_library(szark_camserver_lib STATIC
        src/CameraImageGrabber.cpp src/CameraImageGrabber.hpp
        src/Painter.hpp
        src/GripperHudPainter.cpp src/GripperHudPainter.hpp
        src/HeadHudPainter.cpp src/HeadHudPainter.hpp
        src/ImageSource.hpp
        src/GripperImageSource.cpp src/GripperImageSource.hpp
        src/HeadImageSource.cpp src/HeadImageSource.hpp
        src/NetworkServer.cpp src/NetworkServer.hpp
        )

target_link_libraries(szark_camserver_lib ${LOG4CPP_LIBRARIES})
target_link_libraries(szark_camserver_lib ${Boost_LIBRARIES})
target_link_libraries(szark_camserver_lib ${OpenCV_LIBRARIES})
target_link_libraries(szark_camserver_lib ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(szark_camserver_lib rt)

SET(CAMSERVER_LIB -Wl,--whole-archive szark_camserver_lib -Wl,--no-whole-archive)

add_executable(szark_camserver_gripper src/main_gripper.cpp)
target_link_libraries(szark_camserver_gripper ${CAMSERVER_LIB})
target_link_libraries(szark_camserver_gripper ${COMMON_LIB})


add_executable(szark_camserver_framegrabber src/main_framegrabber.cpp)
target_link_libraries(szark_camserver_framegrabber ${CAMSERVER_LIB})
target_link_libraries(szark_camserver_framegrabber ${COMMON_LIB})


enable_testing()

add_library(szark_camserver_testlib STATIC
        test/CameraImageGrabberTest.cpp
        test/GripperImageSourceTest.cpp
        test/NetworkServerTest.cpp
        )

SET(CAMSERVER_TESTLIB -Wl,--whole-archive szark_camserver_testlib -Wl,--no-whole-archive)

add_executable(szark_camserver_test test/main.cpp)
target_link_libraries(szark_camserver_test ${CAMSERVER_LIB})
target_link_libraries(szark_camserver_test ${COMMON_LIB})
target_link_libraries(szark_camserver_test ${CAMSERVER_TESTLIB})


add_test(SZARKCameraServerTest szark_camserver_test)
