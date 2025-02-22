cmake_minimum_required(VERSION 3.10)

# Create the testing executable
project(ChessTests)

add_executable(checkMoveGen verifyLegalMoves.cpp)
target_link_libraries(checkMoveGen PRIVATE core)

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)  # Add this line to find OpenGL
target_link_libraries(checkMoveGen PRIVATE imgui glfw OpenGL::GL)
target_link_libraries(checkMoveGen PRIVATE core)
target_link_libraries(checkMoveGen PRIVATE tools_common)
target_include_directories(checkMoveGen PRIVATE ${CMAKE_SOURCE_DIR}/external/stb)