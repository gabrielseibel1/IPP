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
target_link_libraries(image_manipulation_lib jpeg)
set_target_properties(image_manipulation_lib PROPERTIES PUBLIC_HEADER include/image_manipulation.h)

# GUI library
add_library(gui_lib STATIC
        include/gui.h
        lib/gui/gui.c
)
set_target_properties(gui_lib PROPERTIES PUBLIC_HEADER include/gui_lib.h)

# GUI test executable
add_executable(fpi_assignment_1
        src/MyApp.cpp
)
find_package(wxWidgets REQUIRED COMPONENTS net core base)
include(${wxWidgets_USE_FILE})
target_link_libraries(fpi_assignment_1
        image_manipulation_lib
        ${wxWidgets_LIBRARIES}
)

# Img lib test executable
add_executable(img_lib_test
        src/main.c
)
target_link_libraries(img_lib_test
        image_manipulation_lib
)