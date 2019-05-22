cmake_minimum_required(VERSION 3.12)
project(lab5)

set(CMAKE_CXX_STANDARD 17)

FIND_PACKAGE(Qt5Core)
FIND_PACKAGE(Qt5Concurrent)
find_package(Boost REQUIRED COMPONENTS locale filesystem system)
include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Concurrent_INCLUDE_DIRS})

get_target_property(QtCore_location Qt5::Core LOCATION)

add_executable(lab5 main.cpp)

target_link_libraries(lab5 Qt5::Core Qt5::Concurrent ${Boost_LIBRARIES})