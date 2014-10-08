cmake_minimum_required(VERSION 2.8.4)
project(SZARK_camera_server)

set(MAIN_EXECUTABLE szark_camera_server)
set(TEST_EXECUTABLE szark_camera_server_test)

set(EXTERNAL_LIBS_DIR "../../external-libs/")

list(APPEND CMAKE_CXX_FLAGS "-std=c++11 -Wall -Og -g ${CMAKE_CXX_FLAGS}")

INCLUDE(FindPkgConfig)

find_package(Boost COMPONENTS system unit_test_framework REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(LOG4CPP "log4cpp")
pkg_check_modules(VIPSCC "vipsCC")
pkg_check_modules(VIPS "vips")

include_directories(${LOG4CPP_INCLUDE_DIRS})
include_directories(${VIPSCC_INCLUDE_DIRS})

include_directories(${EXTERNAL_LIBS_DIR}/wallaroo)

include_directories("src")

file(GLOB src "src/*.hpp" "src/*.cpp")
list(REMOVE_ITEM src "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

file(GLOB main_file "src/main.cpp")
add_executable(${MAIN_EXECUTABLE} ${src} ${main_file})

target_link_libraries(${MAIN_EXECUTABLE} ${LOG4CPP_LIBRARIES})
target_link_libraries(${MAIN_EXECUTABLE} ${Boost_LIBRARIES})
target_link_libraries(${MAIN_EXECUTABLE} ${VIPS_LIBRARIES})
target_link_libraries(${MAIN_EXECUTABLE} ${VIPSCC_LIBRARIES})

enable_testing()

file(GLOB test_src "test/*.hpp" "test/*.cpp")

add_executable(${TEST_EXECUTABLE} ${src} ${test_src})

target_link_libraries(${TEST_EXECUTABLE} ${LOG4CPP_LIBRARIES})
target_link_libraries(${TEST_EXECUTABLE} ${Boost_LIBRARIES})
target_link_libraries(${TEST_EXECUTABLE} ${VIPS_LIBRARIES})
target_link_libraries(${TEST_EXECUTABLE} ${VIPSCC_LIBRARIES})

add_test(SZARKCameraServerTest ${TEST_EXECUTABLE})
