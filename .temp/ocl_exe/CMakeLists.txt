cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
set(CMAKE_VERBOSE_MAKEFILE true)

#
# Include the basic cmake build script
#
include(BuildUtils)

#
# Set the project name
#
set_project_name("_NAME_AUTO_")

#
# List with modules we depend on
#
add_module_dependencies(
)

#
# Include extra directories relative to the module source folder
#
include_directories("${OCV_PATH}")

#
# List of public module header files, relative to the module source folder
#
add_public_headers(
)

#
# List of source files, relative to the module source folder
#
add_sources(
    "main.cpp"
)

#
# List of libs
#
add_libs_to_exe("$ENV{SDK_PATH}/apps/qnx_ap/prebuilt/aarch64le/usr/lib/graphics/qc/libOpenCL.so")
add_libs_to_exe("$ENV{SDK_PATH}/apps/qnx_ap/prebuilt/aarch64le/usr/lib/graphics/qc/libOSUser.so")
add_libs_to_exe("$ENV{SDK_PATH}/apps/qnx_ap/prebuilt/aarch64le/usr/lib/graphics/qc/libGSLUser.so")

#
# Build and install library
#
install_module_exe()

# include(application_ghs)
#
# Macro to enable dummy section between bss & data to reduce binary size
#
# include_dummy_section(true)
