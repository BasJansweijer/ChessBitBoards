cmake_minimum_required(VERSION 3.10)
project(ChessBitboards)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()


set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")  # Debug: No optimization, full debug symbols
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")  # Release: Full optimizations, no debug

set(CMAKE_CXX_STANDARD 20)

# Add subdirectories
add_subdirectory(core)
add_subdirectory(app)
add_subdirectory(tools)
add_subdirectory(tools/common)
add_subdirectory(testing)

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


# Define paths
set(STOCKFISH_SRC_DIR ${CMAKE_SOURCE_DIR}/external/stockfish/src)
set(STOCKFISH_BINARY ${CMAKE_BINARY_DIR}/stockfish)
set(STOCKFISH_EXECUTABLE ${STOCKFISH_SRC_DIR}/stockfish)

# Compile stockfish only if the binary does not exist or dependencies change
add_custom_command(   
    OUTPUT ${STOCKFISH_EXECUTABLE}
    COMMAND make -C ${STOCKFISH_SRC_DIR} -j 4 build
    COMMENT "Compiling stockfish"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS ${STOCKFISH_SRC_DIR}/Makefile
)

# Define a target that triggers compilation
add_custom_target(stockfishCompile DEPENDS ${STOCKFISH_EXECUTABLE})

# Move stockfish binary only if it has changed
add_custom_command(
    OUTPUT ${STOCKFISH_BINARY}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${STOCKFISH_EXECUTABLE} ${STOCKFISH_BINARY}
    COMMENT "Moving stockfish binary to build directory"
    DEPENDS stockfishCompile
)

# Create a target that depends on the final binary
add_custom_target(moveStockfish ALL DEPENDS ${STOCKFISH_BINARY})