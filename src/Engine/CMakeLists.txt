cmake_minimum_required(VERSION 3.26)
project(
        Engine
        VERSION 0.1.0
        LANGUAGES CXX C
        DESCRIPTION "Ajiva Engine"
)


add_library(Engine STATIC
        src/library.cpp
        src/Platform/Window.cpp
        src/Renderer/GpuContext.hpp
        src/Core/Logger.cpp
        src/Core/Clock.cpp
)

target_compile_definitions(Engine PRIVATE AJ_EXPORT)
target_compile_definitions(Engine PRIVATE AJ_LOG_IMGUI)

target_link_libraries(Engine PUBLIC glfw webgpu IMGUI plog PRIVATE glfw3webgpu)
target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
