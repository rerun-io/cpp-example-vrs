cmake_minimum_required(VERSION 3.16)

project(vrs_example LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# if(NOT DEFINED CMAKE_CXX_STANDARD)
# endif()

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare(rerun_sdk URL https://build.rerun.io/commit/f3a5ae2/rerun_cpp_sdk.zip)
FetchContent_MakeAvailable(rerun_sdk)

include(FetchContent)
FetchContent_Declare(vrslib URL https://github.com/facebookresearch/vrs/archive/refs/tags/v1.1.0.zip)
FetchContent_MakeAvailable(vrslib)

find_package(Eigen3 REQUIRED)
find_package(OpenCV REQUIRED)

add_executable(log_vrs src/main.cpp src/FramePlayer.cpp src/utils.cpp)
target_link_libraries(log_vrs Eigen3::Eigen ${OpenCV_LIBS} rerun_sdk vrslib vrs_utils)
target_include_directories(log_vrs PRIVATE src)
