add_subdirectory(../imgui)
add_library(file_browser INTERFACE)
target_include_directories(file_browser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(file_browser INTERFACE imgui)
