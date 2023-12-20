message(STATUS "MNN Depends:" ${MNN_DEPS})
cmake_minimum_required(VERSION 3.9)
project(pmnn)
set(SDK_INC /Users/jim/workspace/MNN/build)
set(CMAKE_CXX_STANDARD 11)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

#find_library(OpenCV)
find_package(OpenCV)

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(inc  /usr/local/include)
set(CMAKE_CXX_STANDARD 11)

link_libraries(${SDK_INC}/libMNN.dylib)
add_executable(pmnn src/file_set_table.cpp src/pre_process.cpp src/core.cpp src/get_file_size.cpp)
add_executable(smnn src/inference.cpp)
add_executable(fmnn src/file_set_table.cpp src/pre_process.cpp src/fork.cpp src/get_file_size.cpp)
target_link_libraries(pmnn ${OpenCV_LIBS})
target_link_libraries(smnn ${OpenCV_LIBS})
target_link_libraries(fmnn ${OpenCV_LIBS})
