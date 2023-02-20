cmake_minimum_required(VERSION 3.0)

set ( CMAKE_CXX_FLAGS "-std=c++11 -pthread" )

project(threadPoolTest)

add_executable(threadPoolTest test.cpp TaskQueue.cpp threadPool.cpp)