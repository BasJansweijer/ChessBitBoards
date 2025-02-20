cmake_minimum_required(VERSION 3.10)
project(ChessBitboards)

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")  # Debug: No optimization, full debug symbols
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")  # Release: Full optimizations, no debug

set(CMAKE_CXX_STANDARD 20)

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


