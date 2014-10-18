cmake_minimum_required(VERSION 2.8.4)
project(SZARK_common_lib)

set(MAIN_LIB szark_common)
set(TEST_EXECUTABLE szark_common_test)

INCLUDE("../common.cmake")

INCLUDE(FindPkgConfig)

find_package(Boost COMPONENTS system unit_test_framework REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(LOG4CPP "log4cpp")

include_directories(${LOG4CPP_INCLUDE_DIRS})

include_directories("src")

file(GLOB src "include/*.hpp" "src/*.cpp")

add_library(${MAIN_LIB} ${src})

target_link_libraries(${MAIN_LIB} ${LOG4CPP_LIBRARIES})
target_link_libraries(${MAIN_LIB} ${Boost_LIBRARIES})

enable_testing()

file(GLOB test_src "test/*.hpp" "test/*.cpp")

add_executable(${TEST_EXECUTABLE} ${src} ${test_src})

target_link_libraries(${TEST_EXECUTABLE} ${LOG4CPP_LIBRARIES})
target_link_libraries(${TEST_EXECUTABLE} ${Boost_LIBRARIES})

add_test(SZARKCommonLibTest ${TEST_EXECUTABLE})
