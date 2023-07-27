cmake_minimum_required(VERSION 3.26)

project(
        App
        VERSION 0.1.0
        LANGUAGES CXX C
        DESCRIPTION "Learn WebGPU (ajiva-webgpu)"
)
option(DEV_MODE "Set up development helper settings" ON)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(libs/glfw)
add_subdirectory(libs/webgpu)
add_subdirectory(libs/glfw3webgpu)


add_executable(App
        Main.cpp
        GpuContext.hpp
        Window.hpp
        magic_enum.hpp
)

if(DEV_MODE)
    target_compile_definitions(App PRIVATE
            RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources"
    )
else()
    target_compile_definitions(App PRIVATE
            RESOURCE_DIR="./resources"
    )
endif()

target_link_libraries(App PRIVATE glfw webgpu glfw3webgpu)

target_copy_webgpu_binaries(App)
