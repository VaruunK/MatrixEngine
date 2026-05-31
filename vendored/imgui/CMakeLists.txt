cmake_minimum_required(VERSION 3.21)
project(imgui)

add_library(imgui STATIC
    imgui.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    imgui_demo.cpp

    # SDL3 + SDL3 GPU backend
    backends/imgui_impl_sdl3.cpp
    backends/imgui_impl_sdlgpu3.cpp
)

add_library(imgui::imgui ALIAS imgui)

target_include_directories(imgui
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/backends
)

# ImGui needs SDL3 headers — SDL3::SDL3 must already exist when this runs
target_link_libraries(imgui PUBLIC SDL3::SDL3)
