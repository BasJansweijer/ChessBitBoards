add_library(core STATIC
    source/core.cpp
    source/math_utils.cpp
)

# Make headers available to other targets
target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)