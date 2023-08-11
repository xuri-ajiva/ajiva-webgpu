cmake_minimum_required(VERSION 3.26)

project(untitled)

set(CMAKE_CXX_STANDARD 20)
option(DEV_MODE "Set up development helper settings" ON)
set(LIBS ${CMAKE_CURRENT_SOURCE_DIR}/libs)
set(BIN ${CMAKE_CURRENT_SOURCE_DIR}/bin)

add_subdirectory(libs/glfw)
add_subdirectory(libs/webgpu)
add_subdirectory(libs/glfw3webgpu)
add_subdirectory(libs/plog)
add_subdirectory(libs/glm)

#=================== IMGUI ===================

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui)
add_library(IMGUI STATIC)

target_sources(IMGUI
        PRIVATE
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/imgui.cpp

        PRIVATE
        ${IMGUI_DIR}/backends/imgui_impl_wgpu.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
)

target_include_directories(IMGUI
        PUBLIC ${IMGUI_DIR}
        PUBLIC ${IMGUI_DIR}/backends
        PUBLIC ${SDL3_DIR}/include
)

target_link_libraries(IMGUI PUBLIC webgpu glfw3webgpu)

#================== ENGINE ==================

add_subdirectory(Engine)

#=================== APP ====================

add_subdirectory(TestBed)

if(DEV_MODE)
    target_compile_definitions(TestBed PRIVATE
            RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources"
    )
else()
    target_compile_definitions(TestBed PRIVATE
            RESOURCE_DIR="./resources"
    )
endif()

