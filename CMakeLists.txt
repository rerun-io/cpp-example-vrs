cmake_minimum_required(VERSION 3.16...3.27)

project(rerun_vrs_example LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
endif()

# Rerun:
include(FetchContent)
FetchContent_Declare(rerun_sdk URL https://github.com/rerun-io/rerun/releases/download/0.11.0/rerun_cpp_sdk.zip)
FetchContent_MakeAvailable(rerun_sdk)

# VRS:
include(FetchContent)
set(BUILD_TOOLS OFF) # VRS tools
set(BUILD_SAMPLES OFF) # VRS sample code and sample apps
FetchContent_Declare(vrslib URL https://github.com/facebookresearch/vrs/archive/refs/tags/v1.1.0.zip)
FetchContent_MakeAvailable(vrslib)

find_package(fmt REQUIRED)

add_executable(rerun_vrs_example src/main.cpp src/FramePlayer.cpp src/IMUPlayer.cpp src/utils.cpp)
target_link_libraries(rerun_vrs_example rerun_sdk vrslib vrs_utils fmt)
target_include_directories(rerun_vrs_example PRIVATE src)
