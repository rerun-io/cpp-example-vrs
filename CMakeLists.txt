cmake_minimum_required(VERSION 3.16...3.27)

project(rerun_vrs_example LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
endif()

include(FetchContent)
FetchContent_Declare(rerun_sdk URL https://github.com/rerun-io/rerun/releases/download/0.10.0/rerun_cpp_sdk.zip)
FetchContent_MakeAvailable(rerun_sdk)

# Don't build the VRS tools
set(BUILD_TOOLS OFF)

include(FetchContent)
FetchContent_Declare(vrslib URL https://github.com/facebookresearch/vrs/archive/refs/tags/v1.1.0.zip)
FetchContent_MakeAvailable(vrslib)

find_package(Eigen3 REQUIRED)
find_package(OpenCV REQUIRED)
find_package(fmt REQUIRED)

add_executable(rerun_vrs_example src/main.cpp src/FramePlayer.cpp src/IMUPlayer.cpp src/utils.cpp)
target_link_libraries(rerun_vrs_example Eigen3::Eigen ${OpenCV_LIBS} rerun_sdk vrslib vrs_utils fmt)
target_include_directories(rerun_vrs_example PRIVATE src)
