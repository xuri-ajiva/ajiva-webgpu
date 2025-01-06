# Define an ImGui target that fits our use case

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui)

add_library(IMGUI STATIC
        # Among the different backends available, we are interested in connecting
        # the GUI to GLFW andWebGPU:
        ${IMGUI_DIR}/backends/imgui_impl_wgpu.h
        ${IMGUI_DIR}/backends/imgui_impl_wgpu.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.h
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp

        # Bonus to add some C++ specific features (the core ImGUi is a C library)
        ${IMGUI_DIR}/misc/cpp/imgui_stdlib.h
        ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp

        # The core ImGui files
        ${IMGUI_DIR}/imconfig.h
        ${IMGUI_DIR}/imgui.h
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_internal.h
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/imstb_rectpack.h
        ${IMGUI_DIR}/imstb_textedit.h
        ${IMGUI_DIR}/imstb_truetype.h
)

target_include_directories(IMGUI PUBLIC .)
target_link_libraries(IMGUI PUBLIC webgpu glfw)