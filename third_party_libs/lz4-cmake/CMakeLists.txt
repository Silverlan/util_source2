cmake_minimum_required(VERSION 2.8)

project(lz4)

include_directories(../lz4/lib)

file(GLOB_RECURSE SRC ../lz4/lib/*.c)
file(GLOB_RECURSE INCLUDES ../lz4/lib/*.h)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3")

add_library(lz4 STATIC ${SRC})
