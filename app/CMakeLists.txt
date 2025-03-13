add_executable(engine main.cpp)
target_link_libraries(engine PRIVATE core)

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)  # Add this line to find OpenGL
target_link_libraries(engine PRIVATE imgui glfw OpenGL::GL)
target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(engine PRIVATE ${CMAKE_SOURCE_DIR}/external/stb)