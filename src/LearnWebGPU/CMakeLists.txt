cmake_minimum_required(VERSION 3.26)
project(
        App
        VERSION 0.1.0
        LANGUAGES CXX C
        DESCRIPTION "Learn WebGPU (ajiva-webgpu)"
)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(libs/glfw)
add_subdirectory(libs/webgpu)
add_subdirectory(libs/glfw3webgpu)


add_executable(App
        Main.cpp
)

target_link_libraries(App PRIVATE glfw webgpu glfw3webgpu)

target_copy_webgpu_binaries(App)