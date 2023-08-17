cmake_minimum_required(VERSION 3.26)

project(AjivaEngine)

set(CMAKE_CXX_STANDARD 20)
option(DEV_MODE "Set up development helper settings" ON)
set(LIBS ${CMAKE_CURRENT_SOURCE_DIR}/libs)
set(BIN ${CMAKE_CURRENT_SOURCE_DIR}/bin)

add_subdirectory(libs/glfw)
add_subdirectory(libs/webgpu)
add_subdirectory(libs/glfw3webgpu)
add_subdirectory(libs/plog)
add_subdirectory(libs/glm)
add_subdirectory(libs/imgui)

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

