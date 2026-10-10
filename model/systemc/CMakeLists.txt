cmake_minimum_required(VERSION 3.20)

project(tabby_systemc CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SystemCLanguage CONFIG REQUIRED)

include(FetchContent)

set(YAML_CPP_BUILD_TESTS
    OFF
    CACHE BOOL "Build yaml-cpp tests")
set(YAML_CPP_BUILD_TOOLS
    OFF
    CACHE BOOL "Build yaml-cpp tools")
set(YAML_CPP_INSTALL
    OFF
    CACHE BOOL "Install yaml-cpp")
set(YAML_CPP_FORMAT_SOURCE
    OFF
    CACHE BOOL "Add yaml-cpp formatting target")

# yaml-cpp 0.8.0 declares an older policy baseline; CMake 4 requires at least
# 3.10.
set(CMAKE_POLICY_VERSION_MINIMUM 3.20)

FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG 0.8.0
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(yaml-cpp)

FetchContent_Declare(
    CLI11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG v2.4.2
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(CLI11)

FetchContent_Declare(
    lodepng
    GIT_REPOSITORY https://github.com/lvandeve/lodepng.git
    GIT_TAG e584a8490db1b76b81bf298c11c09d3b55d9c270)
FetchContent_MakeAvailable(lodepng)

# LodePNG provides source files rather than a CMake library target.
add_library(lodepng STATIC ${lodepng_SOURCE_DIR}/lodepng.cpp)
add_library(LodePNG::LodePNG ALIAS lodepng)
target_include_directories(lodepng SYSTEM PUBLIC ${lodepng_SOURCE_DIR})

add_executable(
    tabby_tlm_model
    src/main.cc
    src/config_loader.cc
    src/input_buffer.cc
    src/timing.cc
    src/vertex_2d_io.cc
    src/command_queue.cc
    src/command_processor.cc
    src/tabby_tlm_model.cc
    src/frame_buffer.cc
    src/triangle_rasterizer.cc
    src/rasterized_scanline.cc)

target_include_directories(tabby_tlm_model
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/tabby)

# Keep SystemC headers compatible with the library built using C++17.
target_compile_definitions(tabby_tlm_model PRIVATE SC_CPLUSPLUS=201703L)

target_link_libraries(
    tabby_tlm_model PRIVATE SystemC::systemc yaml-cpp::yaml-cpp CLI11::CLI11
                            LodePNG::LodePNG)
