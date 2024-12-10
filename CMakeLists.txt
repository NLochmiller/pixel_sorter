cmake_minimum_required(VERSION 3.29)

project(TestSDL2 VERSION 1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Require SDL2
find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
# Due to find_package being unable to find SDL2_image use a external module
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
find_package(SDL2_image REQUIRED)
# Setup subdirectories, executable, and linking
add_subdirectory(src libs)
add_executable(${CMAKE_PROJECT_NAME} src/main.cpp)
if(TARGET SDL2::SDL2main)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE SDL2::SDL2main)
endif()
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE sources SDL2::SDL2)
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
  "${PROJECT_BINARY_DIR}"
  "${PROJECT_SOURCE_DIR}/src"
  "${SDL2_INCLUDE_DIRS}"
)



