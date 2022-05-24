cmake_minimum_required(VERSION 3.10)

project(test_project)

include_directories(include)

set(SOURCES main.cpp)
add_executable(main_exe ${SOURCES})

set(TEST_SOURCES test.cpp)
add_executable(test_exe ${TEST_SOURCES})
target_link_libraries(test_exe pthread)