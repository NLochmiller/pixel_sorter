find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
add_library(sources ColorConversion.cpp ImGui_SDL2_helpers.cpp
  LineCollision.cpp main.cpp ImGui_ImageZoomable.cpp Knob.cpp
  LineInterpolator.cpp PixelSorter.cpp)
add_subdirectory(../libs libs)
target_include_directories(sources PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(sources libs_main SDL2::SDL2)
