add_executable(app main.cpp)
target_link_libraries(app PRIVATE core)

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)  # Add this line to find OpenGL
target_link_libraries(app PRIVATE imgui glfw OpenGL::GL)
target_include_directories(app PRIVATE ${CMAKE_SOURCE_DIR}/external/stb)