cmake_minimum_required(VERSION 3.10)

# Create the testing executable
project(ChessTests)

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)  # Add this line to find OpenGL

add_executable(checkMoveGen verifyLegalMoves.cpp)
target_link_libraries(checkMoveGen PRIVATE core)
target_link_libraries(checkMoveGen PRIVATE imgui glfw OpenGL::GL)
target_link_libraries(checkMoveGen PRIVATE core)
target_link_libraries(checkMoveGen PRIVATE tools_common)
target_include_directories(checkMoveGen PRIVATE ${CMAKE_SOURCE_DIR}/external/stb)

add_executable(benchMoveGen benchmarkMoveGen.cpp)
target_link_libraries(benchMoveGen PRIVATE core)
target_link_libraries(benchMoveGen PRIVATE imgui glfw OpenGL::GL)
target_link_libraries(benchMoveGen PRIVATE core)
target_link_libraries(benchMoveGen PRIVATE tools_common)
target_include_directories(benchMoveGen PRIVATE ${CMAKE_SOURCE_DIR}/external/stb)

# Define paths
set(DATA_DIR ${CMAKE_SOURCE_DIR}/testing/data)
set(TEST_FENS_BUILD ${CMAKE_BINARY_DIR}/testing/)
set(FENS_FILES ${DATA_DIR}/fens10000.txt ${DATA_DIR}/fens10.txt)

# Move test fens files only if they have changed
add_custom_command(
    OUTPUT ${TEST_FENS_BUILD}/fens10000.txt ${TEST_FENS_BUILD}/fens10.txt
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FENS_FILES} ${TEST_FENS_BUILD}
    COMMENT "Copy test fens to build directory"
    DEPENDS ${FENS_FILES}
)

add_custom_target(copyFens
    DEPENDS ${TEST_FENS_BUILD}/fens10000.txt ${TEST_FENS_BUILD}/fens10.txt
)

add_custom_target(profile
    COMMAND chdir ${CMAKE_BINARY_DIR} && rm -f callgrind.out.*
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR} valgrind --tool=callgrind ./testing/benchMoveGen --quick
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR} kcachegrind callgrind.out.*
    COMMENT "Running callgrind and opening results in kcachegrind"
)

add_custom_target(runCopyFens ALL DEPENDS copyFens)