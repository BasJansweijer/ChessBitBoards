cmake_minimum_required(VERSION 3.10)
project(ChessBitboards)

# Add subdirectories for core and app
add_subdirectory(core)
add_subdirectory(app)
add_subdirectory(tools)
add_subdirectory(tools/common)

# add imgui
file(GLOB IMGUI_SRC
    external/imgui/*.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)

add_library(imgui STATIC ${IMGUI_SRC})
target_include_directories(imgui PUBLIC external/imgui external/imgui/backends)


# add stb
file(GLOB STB_SRC
    external/stb/*.cpp
)


