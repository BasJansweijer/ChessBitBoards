add_library(core STATIC
    source/chessMoveGen.cpp
    source/makeMove.cpp
    source/bitBoardHelpers.cpp
    source/boardVisualizer.cpp
    source/boardHelpers.cpp
    source/initBoard.cpp
    source/fen.cpp
)

target_include_directories(core PUBLIC
    ${CMAKE_SOURCE_DIR}/external/imgui
    ${CMAKE_SOURCE_DIR}/external/imgui/backends
)


set(ASSETS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets)
# configure_file(${ASSETS_DIR}/kings.png ${CMAKE_BINARY_DIR}/kings.png COPYONLY)

set(ASSETS_DIR "${CMAKE_SOURCE_DIR}/core/assets")
set(OUTPUT_ASSETS_DIR "${CMAKE_BINARY_DIR}/assets")

file(MAKE_DIRECTORY ${OUTPUT_ASSETS_DIR})

# Find all PNG files in the assets directory
file(GLOB PNG_FILES ${ASSETS_DIR}/*.png)

# # Copy all the PNG files to the build directory
foreach(PNG_FILE ${PNG_FILES})
    configure_file(${PNG_FILE} ${OUTPUT_ASSETS_DIR} COPYONLY)
endforeach()

# Make headers available to other targets
target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(core PRIVATE ${CMAKE_SOURCE_DIR}/external/stb)