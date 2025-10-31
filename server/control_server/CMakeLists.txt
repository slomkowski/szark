#
# CMake configuration for SZARK Control Server
# (C) 2014, 2015, 2016 Michał Słomkowski
# License: GPL v3
#

cmake_minimum_required(VERSION 3.25)
project(SZARK_control_server)

if (POLICY CMP0167)
        cmake_policy(SET CMP0167 NEW)
endif ()

INCLUDE("../common.cmake")

find_package(Boost REQUIRED COMPONENTS unit_test_framework program_options filesystem)
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

pkg_check_modules(LIBUSB "libusb-1.0")
pkg_check_modules(LOG4CPP "log4cpp")
pkg_check_modules(NETLINK_GENL "libnl-genl-3.0")
pkg_check_modules(NETLINK_ROUTE "libnl-route-3.0")
pkg_check_modules(DW "libdw")


include_directories(${LIBUSB_INCLUDE_DIRS})
include_directories(${LOG4CPP_INCLUDE_DIRS})
include_directories(${NETLINK_GENL_INCLUDE_DIRS})
include_directories(${NETLINK_ROUTE_INCLUDE_DIRS})
include_directories(${DW_INCLUDE_DIRS})

include_directories(../../external-libs/backward-cpp)
include_directories(../../external-libs/minijson_writer)
include_directories(../../external-libs/minijson_reader)

include_directories("src")

link_directories(${LIBUSB_LIBRARY_DIRS})
link_directories(${LOG4CPP_LIBRARY_DIRS})
link_directories(${NETLINK_GENL_LIBRARY_DIRS})
link_directories(${NETLINK_ROUTE_LIBRARY_DIRS})
link_directories(${DW_LIBRARY_DIRS})

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
target_link_libraries(szark_control_server ${NETLINK_GENL_LIBRARIES})
target_link_libraries(szark_control_server ${NETLINK_ROUTE_LIBRARIES})
target_link_libraries(szark_control_server ${DW_LIBRARIES})
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
target_link_libraries(szark_control_server_test ${NETLINK_GENL_LIBRARIES})
target_link_libraries(szark_control_server_test ${NETLINK_ROUTE_LIBRARIES})
target_link_libraries(szark_control_server_test ${Boost_LIBRARIES})
target_link_libraries(szark_control_server_test ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(szark_control_server_test rt)

add_test(SZARKControlServerTest szark_control_server_test)
