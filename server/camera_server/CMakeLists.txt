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

include_directories(../../lib/libpixfc/include)

set(SOURCES
        src/CameraImageGrabber.cpp src/CameraImageGrabber.hpp
        src/Painter.hpp
        src/GripperHudPainter.cpp src/GripperHudPainter.hpp
        src/HeadHudPainter.cpp src/HeadHudPainter.hpp
        src/ImageSource.hpp
        src/GripperImageSource.cpp src/GripperImageSource.hpp
        src/HeadImageSource.cpp src/HeadImageSource.hpp
        src/NetworkServer.cpp src/NetworkServer.hpp
        src/JpegEncoder.cpp src/JpegEncoder.hpp
        )

if (${CMAKE_SIZEOF_VOID_P} STREQUAL "8")
    link_directories("../../lib/libpixfc/x86_64")
else ()
    link_directories("../../lib/libpixfc/x86")
endif ()

add_executable(szark_camserver_framegrabber ${SOURCES} src/main_framegrabber.cpp)
target_link_libraries(szark_camserver_framegrabber ${COMMON_LIB})
target_link_libraries(szark_camserver_framegrabber pixfc-sse)
target_link_libraries(szark_camserver_framegrabber ${LOG4CPP_LIBRARIES})
target_link_libraries(szark_camserver_framegrabber ${Boost_LIBRARIES})
target_link_libraries(szark_camserver_framegrabber ${OpenCV_LIBRARIES})
target_link_libraries(szark_camserver_framegrabber ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(szark_camserver_framegrabber rt)
target_link_libraries(szark_camserver_framegrabber turbojpeg)

add_executable(szark_camserver_gripper ${SOURCES} src/main_gripper.cpp)
target_link_libraries(szark_camserver_gripper ${COMMON_LIB})
target_link_libraries(szark_camserver_gripper pixfc-sse)
target_link_libraries(szark_camserver_gripper ${LOG4CPP_LIBRARIES})
target_link_libraries(szark_camserver_gripper ${Boost_LIBRARIES})
target_link_libraries(szark_camserver_gripper ${OpenCV_LIBRARIES})
target_link_libraries(szark_camserver_gripper ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(szark_camserver_gripper rt)
target_link_libraries(szark_camserver_gripper turbojpeg)

enable_testing()

set(TEST_SOURCES
        test/CameraImageGrabberTest.cpp
        test/GripperImageSourceTest.cpp
        test/NetworkServerTest.cpp
        )

add_executable(szark_camserver_test ${TEST_SOURCES} test/main.cpp)
target_link_libraries(szark_camserver_test ${COMMON_LIB})
target_link_libraries(szark_camserver_test pixfc-sse)
target_link_libraries(szark_camserver_test ${LOG4CPP_LIBRARIES})
target_link_libraries(szark_camserver_test ${Boost_LIBRARIES})
target_link_libraries(szark_camserver_test ${OpenCV_LIBRARIES})
target_link_libraries(szark_camserver_test ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(szark_camserver_test rt)
target_link_libraries(szark_camserver_test turbojpeg)

add_test(SZARKCameraServerTest szark_camserver_test)
