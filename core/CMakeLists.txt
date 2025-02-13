add_library(core STATIC
    source/chessBoard.cpp
    source/bitBoardHelpers.cpp
    source/boardVisualizer.cpp
)

target_include_directories(core PUBLIC
    ${CMAKE_SOURCE_DIR}/external/imgui
    ${CMAKE_SOURCE_DIR}/external/imgui/backends
)

# Make headers available to other targets
target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)