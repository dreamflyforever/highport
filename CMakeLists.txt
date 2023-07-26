message(STATUS "MNN Depends:" ${MNN_DEPS})
cmake_minimum_required(VERSION 3.9)
project(pmnn)
set(SDK_INC /Users/jim/workspace/MNN/build)
set(CMAKE_CXX_STANDARD 11)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

#find_library(OpenCV)
find_package(OpenCV)

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(.  /usr/local/include)
set(CMAKE_CXX_STANDARD 11)

link_libraries(${SDK_INC}/libMNN.dylib)
add_executable(pmnn file_set_table.cpp pre_process.cpp core.cpp)
add_executable(smnn inference.cpp)
target_link_libraries(pmnn ${OpenCV_LIBS})
target_link_libraries(smnn ${OpenCV_LIBS})
