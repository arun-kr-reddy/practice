include_directories(cv)
include_directories(dsa)
include_directories("$ENV{CUDA_PATH}/include/")

set(SOURCES main.cpp)
add_executable(main_exe ${SOURCES})
target_link_libraries(main_exe "$ENV{CUDA_PATH}/lib/x64/OpenCL.lib")
add_custom_command(
        TARGET main_exe POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/cv/kernels.cl
                ${CMAKE_CURRENT_BINARY_DIR})
                
set(TEST_SOURCES test.cpp dsa/peak_finding.cpp dsa/document_distance.cpp)
add_executable(test_exe ${TEST_SOURCES})