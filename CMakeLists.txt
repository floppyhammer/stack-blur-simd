cmake_minimum_required(VERSION 3.16.1)
project(stack_blur_simd)

set(CMAKE_CXX_STANDARD 17)

# Set binary output directory. Using CMAKE_CURRENT_SOURCE_DIR is necessary.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

# Include third_party headers.
include_directories(third_party)

# Core library.
add_subdirectory(src)

add_executable(demo main.cpp)

target_link_libraries(demo libstackblursimd)
