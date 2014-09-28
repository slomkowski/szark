cmake_minimum_required(VERSION 2.8.4)
project(szarkcam)

find_package( OpenCV REQUIRED )
find_package(Boost COMPONENTS system REQUIRED)

include_directories( ${OpenCV_INCLUDE_DIRS} )

set(SOURCE_FILES main.cpp)
add_executable(szarkcam ${SOURCE_FILES})

target_link_libraries(szarkcam ${OpenCV_LIBS})
target_link_libraries(szarkcam ${Boost_LIBRARIES})

add_definitions(-std=c++11)