cmake_minimum_required(VERSION 3.20)

project(tabby_systemc CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SystemCLanguage CONFIG REQUIRED)

add_executable(tabby_tlm_model
    src/main.cc
    src/input_buffer.cc
    src/vertex_2d_io.cc
    src/command_queue.cc
    src/command_processor.cc
)

target_include_directories(tabby_tlm_model PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tabby
)

#target_link_libraries(tabby_tlm_model PRIVATE SystemC::systemc)
