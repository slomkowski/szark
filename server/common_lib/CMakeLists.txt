cmake_minimum_required(VERSION 3.25)
project(SZARK_common_lib)

set(MAIN_LIB szark_common)
set(TEST_EXECUTABLE szark_common_test)

if (POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif ()

INCLUDE("../common.cmake")

INCLUDE(FindPkgConfig)

find_package(Boost REQUIRED COMPONENTS unit_test_framework program_options filesystem)
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

pkg_check_modules(LOG4CPP "log4cpp")

include_directories(${LOG4CPP_INCLUDE_DIRS})
include_directories("src")

link_directories(${LOG4CPP_LIBRARY_DIRS})

set(src
        src/ColorPatternLayout.cpp include/ColorPatternLayout.hpp
        src/Configuration.cpp include/Configuration.hpp
        src/DataHolder.cpp include/DataHolder.hpp
        src/Interface.cpp include/Interface.hpp
        src/SharedInterfaceProvider.cpp include/SharedInterfaceProvider.hpp
        src/IoServiceProvider.cpp include/IoServiceProvider.hpp
        src/convert.cpp include/convert.hpp
        src/initialization.cpp include/initialization.hpp
        src/logging.cpp include/logging.hpp
        src/utils.cpp include/utils.hpp
)

add_library(${MAIN_LIB} STATIC ${src})

target_link_libraries(${MAIN_LIB}
        ${LOG4CPP_LIBRARIES}
        Boost::program_options
        Boost::filesystem
        Threads::Threads
        rt
)

enable_testing()

set(test_src
        test/ConfigurationTest.cpp
        test/SharedInterfaceProviderTest.cpp
)

add_executable(${TEST_EXECUTABLE} ${src} ${test_src} test/main.cpp)

target_link_libraries(${TEST_EXECUTABLE}
        ${LOG4CPP_LIBRARIES}
        Boost::unit_test_framework
        Boost::program_options
        Boost::filesystem
        Threads::Threads
        rt
)

add_test(SZARKCommonLibTest ${TEST_EXECUTABLE})
