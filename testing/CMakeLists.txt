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




# Define paths
set(DATA_DIR ${CMAKE_SOURCE_DIR}/testing/data)
set(TEST_FENS_BUILD ${CMAKE_BINARY_DIR}/testing/fens.txt)
set(FENS_FILE ${DATA_DIR}/fens.txt)



# Move testfens binary only if it has changed
add_custom_command(
    OUTPUT ${TEST_FENS_BUILD}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FENS_FILE} ${TEST_FENS_BUILD}
    COMMENT "Test copy fens to build directory"
    DEPENDS ${FENS_FILE}
    )
    
add_custom_target(copyFens DEPENDS ${TEST_FENS_BUILD})
add_custom_target(runCopyFens ALL DEPENDS copyFens)