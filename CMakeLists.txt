cmake_minimum_required(VERSION 3.10)

project(test_project)

include_directories(include)

set(SOURCES main.cpp)

add_executable(main_exe ${SOURCES})