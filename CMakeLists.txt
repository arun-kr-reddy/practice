cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_MAKE_PROGRAM mingw32-make.exe)

project(test_project)

include_directories(include)

set(SOURCES main.cpp)
add_executable(main_exe ${SOURCES})

set(TEST_SOURCES test.cpp)
add_executable(test_exe ${TEST_SOURCES})