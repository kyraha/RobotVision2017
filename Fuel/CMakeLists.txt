cmake_minimum_required (VERSION 2.8)

project(fuel-tracker)

set(CUDA_USE_STATIC_CUDA_RUNTIME OFF)
set(CMAKE_CXX_STANDARD 11)

find_package(OpenCV 3 REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS} ./)

add_executable(fuel-tracker fuel-tracker.cpp)

target_link_libraries(fuel-tracker ${OpenCV_LIBS})

