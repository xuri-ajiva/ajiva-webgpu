cmake_minimum_required(VERSION 3.26)
project(
        Engine
        VERSION 0.1.0
        LANGUAGES CXX C
        DESCRIPTION "Ajiva Engine"
)

add_library(Engine STATIC
        #SHARED
        src/library.cpp
        src/Platform/Window.cpp
        src/Renderer/GpuContext.cpp
        src/Renderer/GpuContext.h
        src/Core/Logger.cpp
        src/Core/Clock.cpp
        src/Renderer/Buffer.cpp
        src/Renderer/Buffer.h
        src/Renderer/Texture.cpp
        src/Renderer/Texture.h
        src/Resource/Loader.cpp
        src/Resource/Loader.h
        src/Resource/tiny_obj_loader.h
        src/Application.cpp
        src/Application.h
        src/Renderer/Renderer.cpp
        src/Renderer/Renderer.h
        src/Resource/ResourceManager.h
        src/Platform/PlatformSystem.h
        src/Platform/PlatformSystem.cpp
        src/Renderer/Camera.cpp
        src/Renderer/Camera.h
        src/Core/EventSystem.cpp
        src/Core/EventSystem.h
        src/implementations.cpp
        src/Core/Layer.h
        src/Renderer/ImGuiLayer.cpp
        src/Renderer/ImGuiLayer.h
        src/Core/Layer.cpp
        src/Renderer/BindGroupBuilder.cpp
        src/Renderer/BindGroupBuilder.h
        src/Core/ThreadPool.cpp
        src/Core/ThreadPool.h
)

target_compile_definitions(Engine PRIVATE AJ_EXPORT)
target_compile_definitions(Engine PRIVATE AJ_LOG_IMGUI)

target_link_libraries(Engine PUBLIC glfw imgui plog glm glfw3webgpu PUBLIC webgpu)

include(GenerateExportHeader)
generate_export_header(Engine)

target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(Engine PROPERTIES LINK_FLAGS "/PROFILE")
