
add_library(tools_common STATIC toolUtils.cpp)
target_link_libraries(tools_common PUBLIC core)
target_include_directories(tools_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
