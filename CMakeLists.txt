cmake_minimum_required(VERSION 3.9)
project(main)
 
set(CMAKE_CXX_STANDARD 11)
 
#find_library(OpenCV)
find_package(OpenCV)
 
include_directories(${OpenCV_INCLUDE_DIRS})
set(CMAKE_CXX_STANDARD 11)
 
add_executable(main pre_process.cpp)
target_link_libraries(main ${OpenCV_LIBS})
