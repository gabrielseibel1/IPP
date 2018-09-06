cmake_minimum_required(VERSION 3.9)

project(fpi_assignment_1)
set(CMAKE_CXX_STANDARD 11)

# Headers folder
include_directories(include)

# Core Library
add_library(image_manipulation_lib STATIC
        include/image_manipulation.h
        lib/img/image_manipulation.c
)
set_target_properties(image_manipulation_lib PROPERTIES PUBLIC_HEADER include/image_manipulation.h)

# GUI library
add_library(gui_lib STATIC
        include/gui.h
        lib/gui/gui.c
)
set_target_properties(gui_lib PROPERTIES PUBLIC_HEADER include/gui_lib.h)

# Main executable
add_executable(fpi_assignment_1
        src/main.cpp
)
target_link_libraries(fpi_assignment_1
        image_manipulation_lib
        gui_lib
)