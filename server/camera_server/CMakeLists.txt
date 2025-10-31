#
# CMake configuration for SZARK Camera Server
# (C) 2014, 2015 Michał Słomkowski
# License: GPL v3
#

cmake_minimum_required(VERSION 3.25)
project(SZARK_camera_server)

if (POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif ()

INCLUDE("../common.cmake")

find_package(Boost REQUIRED COMPONENTS unit_test_framework program_options filesystem)
find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

pkg_check_modules(LOG4CPP "log4cpp")
pkg_check_modules(DW "libdw")

include_directories(${LOG4CPP_INCLUDE_DIRS})
include_directories(${DW_INCLUDE_DIRS})

include_directories("src")

include_directories(../../lib/libpixfc/include)

include_directories(../../external-libs/backward-cpp)
include_directories(../../external-libs/minijson_reader)
include_directories(../../external-libs/minijson_writer)

link_directories(${LOG4CPP_LIBRARY_DIRS})
link_directories(${DW_LIBRARY_DIRS})

set(SOURCES
        src/CameraImageGrabber.cpp src/CameraImageGrabber.hpp
        src/Painter.hpp
        src/GripperHudPainter.cpp src/GripperHudPainter.hpp
        src/HeadHudPainter.cpp
        src/ImageSource.hpp
        src/GripperImageSource.cpp src/GripperImageSource.hpp
        src/HeadImageSource.cpp
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
target_link_libraries(szark_camserver_framegrabber ${DW_LIBRARIES})
target_link_libraries(szark_camserver_framegrabber rt)
target_link_libraries(szark_camserver_framegrabber turbojpeg)

add_executable(szark_camserver_gripper ${SOURCES} src/main_gripper.cpp)
target_link_libraries(szark_camserver_gripper ${COMMON_LIB})
target_link_libraries(szark_camserver_gripper pixfc-sse)
target_link_libraries(szark_camserver_gripper ${LOG4CPP_LIBRARIES})
target_link_libraries(szark_camserver_gripper ${Boost_LIBRARIES})
target_link_libraries(szark_camserver_gripper ${OpenCV_LIBRARIES})
target_link_libraries(szark_camserver_gripper ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(szark_camserver_gripper ${DW_LIBRARIES})
target_link_libraries(szark_camserver_gripper rt)
target_link_libraries(szark_camserver_gripper turbojpeg)

enable_testing()

set(TEST_SOURCES
        test/CameraImageGrabberTest.cpp
        test/GripperImageSourceTest.cpp
        test/NetworkServerTest.cpp
        test/JpegEncoderTest.cpp
        )

add_executable(szark_camserver_test ${SOURCES} ${TEST_SOURCES} test/main.cpp)
target_link_libraries(szark_camserver_test ${COMMON_LIB})
target_link_libraries(szark_camserver_test pixfc-sse)
target_link_libraries(szark_camserver_test ${LOG4CPP_LIBRARIES})
target_link_libraries(szark_camserver_test ${Boost_LIBRARIES})
target_link_libraries(szark_camserver_test ${OpenCV_LIBRARIES})
target_link_libraries(szark_camserver_test ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(szark_camserver_test ${DW_LIBRARIES})
target_link_libraries(szark_camserver_test rt)
target_link_libraries(szark_camserver_test turbojpeg)

add_test(SZARKCameraServerTest szark_camserver_test)
