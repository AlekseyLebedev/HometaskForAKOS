cmake_minimum_required(VERSION 2.8.4)
project(untitled)

set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -std=c99")

set(SOURCE_FILES main.c)
add_executable(untitled ${SOURCE_FILES})