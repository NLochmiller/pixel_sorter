add_subdirectory(../imgui_base imgui_base)
# Must find SDL2 since we use it.
find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
add_library(imgui_sdlrenderer2 imgui_impl_sdl2.cpp  imgui_impl_sdlrenderer2.cpp)
target_include_directories(imgui_sdlrenderer2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(imgui_sdlrenderer2 imgui_base SDL2::SDL2)
