cmake_minimum_required(VERSION 3.11)

project(PixelSorter VERSION 1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Change to wherever the final executable is outputed.
# Only used for windows
set(EXE_OUTPUT_FOLDER "${CMAKE_BINARY_DIR}\\Debug")

# Require SDL2
# Find sdl2
set(CMAKE_PROJECT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
if (WIN32) # Windows shenanigans due to windows library system being irritating
    set(SDL2_DIR "${CMAKE_PROJECT_PATH}\\external\\sdl\\SDL2\\cmake")  # Replace with the actual SDL2 directory
    set(SDL2_image_DIR "${CMAKE_PROJECT_PATH}\\external\\sdl\\SDL2_image\\cmake")
    find_package(SDL2 REQUIRED)
    find_package(SDL2_image REQUIRED)
else()
    find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
    # Due to find_package being unable to find SDL2_image use a external module
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
    find_package(SDL2_image REQUIRED)
endif ()

# Setup subdirectories, executable, and linking
add_subdirectory(src external)
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

# The dlls are not copied over so do it manually
if (WIN32)
    # Support both 32 and 64 bit builds
    if (${CMAKE_SIZEOF_VOID_P} MATCHES 8)
        configure_file("${SDL2_DIR}\\..\\lib\\x64\\SDL2.dll" "${EXE_OUTPUT_FOLDER}\\SDL2.dll" COPYONLY)
        configure_file("${SDL2_image_DIR}\\..\\lib\\x64\\SDL2_image.dll" "${EXE_OUTPUT_FOLDER}\\SDL2_image.dll" COPYONLY)
    else ()
        configure_file("${SDL2_DIR}\\..\\lib\\x86\\SDL2.dll" "${EXE_OUTPUT_FOLDER}\\SDL2.dll" COPYONLY)
        configure_file("${SDL2_image_DIR}\\..\\lib\\x86\\SDL2_image.dll" "${EXE_OUTPUT_FOLDER}\\SDL2_image.dll" COPYONLY)
    endif ()
endif ()



