add_library(imgui_base imgui.cpp imgui_demo.cpp imgui_draw.cpp imgui_tables.cpp imgui_widgets.cpp)
# We use imgui throughout the project, so we want it to be public
target_include_directories(imgui_base PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
