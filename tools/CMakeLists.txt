# tools/CMakeLists.txt

add_executable(moveArrayGen moveArrayGen.cpp)
target_link_libraries(moveArrayGen PRIVATE core)

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)  # Add this line to find OpenGL
target_link_libraries(moveArrayGen PRIVATE imgui glfw OpenGL::GL tools_common)
target_include_directories(moveArrayGen PRIVATE ${CMAKE_SOURCE_DIR}/external/stb)

target_include_directories(moveArrayGen PRIVATE ${CMAKE_SOURCE_DIR}/tools/common)