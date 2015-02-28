cmake_minimum_required(VERSION 2.8.4)
project(catFILE)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -g -O0")

set(SOURCE_FILES catFILE.c)
add_executable(catFILE ${SOURCE_FILES})

project(catLowLevel)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

set(SOURCE_FILES catLowLevel.c)
add_executable(catLowLevel ${SOURCE_FILES})
