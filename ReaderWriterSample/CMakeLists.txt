cmake_minimum_required(VERSION 3.3)
project(ReaderWriterSample)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

set(SOURCE_FILES main.cpp)
link_libraries(-lrt)
add_executable(ReaderWriterSample ${SOURCE_FILES})