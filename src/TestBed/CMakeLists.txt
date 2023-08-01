cmake_minimum_required(VERSION 3.26)
project(TestBed)

set(CMAKE_CXX_STANDARD 20)

add_executable(TestBed main.cpp)

target_link_libraries(TestBed PRIVATE Engine)
target_copy_webgpu_binaries(TestBed)

