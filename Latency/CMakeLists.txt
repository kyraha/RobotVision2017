cmake_minimum_required (VERSION 2.8)

project(latency)

set(CUDA_USE_STATIC_CUDA_RUNTIME OFF)
set(CMAKE_CXX_STANDARD 11)

find_package(OpenCV 3 REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS} ./)

add_executable(latency latency.cpp)

target_link_libraries(latency ${OpenCV_LIBS})

