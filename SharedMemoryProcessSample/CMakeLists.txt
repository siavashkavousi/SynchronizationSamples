cmake_minimum_required(VERSION 3.3)
project(SynchronizationSamples)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

link_libraries(-lrt)

add_executable(INCREMENTER incrementer.cpp)
add_executable(DECREMENTER decrementer.cpp)