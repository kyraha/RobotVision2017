cmake_minimum_required (VERSION 2.8)

project(optical_flow)

set(CUDA_USE_STATIC_CUDA_RUNTIME OFF)
set(CMAKE_CXX_STANDARD 11)

find_package(OpenCV 3 REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS} ./)

add_executable(optical_flow optical_flow.cpp)

target_link_libraries(optical_flow ${OpenCV_LIBS})

