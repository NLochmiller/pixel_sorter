find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
find_package(SDL2_image REQUIRED)
add_library(sources ColorConversion.cpp ImGui_SDL2_helpers.cpp
  LineCollision.cpp main.cpp ImGui_ImageZoomable.cpp Knob.cpp
  LineInterpolator.cpp PixelSorter.cpp)
add_subdirectory(../external external)
target_include_directories(sources PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
if (WIN32)
  target_link_libraries(sources external SDL2::SDL2 SDL2_image::SDL2_image)
else()
  include_directories(${SDL2_IMAGE_INCLUDE_DIRS})
  target_link_libraries(sources external SDL2::SDL2 ${SDL2_IMAGE_LIBRARIES})
endif()
