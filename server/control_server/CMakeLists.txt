#
# CMake configuration for SZARK Control Server
# (C) 2014, 2015 Michał Słomkowski
# License: GPL v3
#

cmake_minimum_required(VERSION 2.8.4)
project(SZARK_control_server)

INCLUDE("../common.cmake")

find_package(Boost COMPONENTS system unit_test_framework program_options filesystem REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

pkg_check_modules(LIBUSB "libusb-1.0")
pkg_check_modules(LOG4CPP "log4cpp")
pkg_check_modules(JSON "jsoncpp")
pkg_check_modules(NETLINK_GENL "libnl-genl-3.0")
pkg_check_modules(NETLINK_ROUTE "libnl-route-3.0")

include_directories(${LIBUSB_INCLUDE_DIRS})
include_directories(${LOG4CPP_INCLUDE_DIRS})
include_directories(${JSON_INCLUDE_DIRS})
include_directories(${NETLINK_GENL_INCLUDE_DIRS})
include_directories(${NETLINK_ROUTE_INCLUDE_DIRS})

include_directories("src")

set(SOURCES
        src/BridgeProcessor.cpp src/BridgeProcessor.hpp
        src/InterfaceManager.cpp src/InterfaceManager.hpp
        src/IRequestProcessor.hpp
        src/NetServer.cpp src/NetServer.hpp
        src/OSInformationProcessor.cpp src/OSInformationProcessor.hpp
        src/RequestQueuer.cpp src/RequestQueuer.hpp
        src/USBCommunicator.cpp src/USBCommunicator.hpp
        src/WifiInfo.cpp src/WifiInfo.hpp
        )

add_executable(szark_control_server ${SOURCES} src/main.cpp)

target_link_libraries(szark_control_server ${COMMON_LIB})
target_link_libraries(szark_control_server ${LIBUSB_LIBRARIES})
target_link_libraries(szark_control_server ${LOG4CPP_LIBRARIES})
target_link_libraries(szark_control_server ${JSON_LIBRARIES})
target_link_libraries(szark_control_server ${NETLINK_GENL_LIBRARIES})
target_link_libraries(szark_control_server ${NETLINK_ROUTE_LIBRARIES})
target_link_libraries(szark_control_server ${Boost_LIBRARIES})
target_link_libraries(szark_control_server ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(szark_control_server rt)

enable_testing()

set(TEST_SOURCES
        test/BridgeProcessorTest.cpp
        test/InterfaceManagerTest.cpp
        test/NetServerTest.cpp
        test/RequestQueuerTest.cpp
        test/USBCommunicationTest.cpp
        test/WifiInfoTest.cpp
        )

add_executable(szark_control_server_test ${SOURCES} ${TEST_SOURCES} test/main.cpp)

target_link_libraries(szark_control_server_test ${COMMON_LIB})
target_link_libraries(szark_control_server_test ${LIBUSB_LIBRARIES})
target_link_libraries(szark_control_server_test ${LOG4CPP_LIBRARIES})
target_link_libraries(szark_control_server_test ${JSON_LIBRARIES})
target_link_libraries(szark_control_server_test ${NETLINK_GENL_LIBRARIES})
target_link_libraries(szark_control_server_test ${NETLINK_ROUTE_LIBRARIES})
target_link_libraries(szark_control_server_test ${Boost_LIBRARIES})
target_link_libraries(szark_control_server_test ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(szark_control_server_test rt)

add_test(SZARKControlServerTest szark_control_server_test)
