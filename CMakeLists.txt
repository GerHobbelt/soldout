# CMake version configuration
cmake_minimum_required(VERSION 3.5...3.19)
if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

# Define project
project(hoedown VERSION 3.0.7 LANGUAGES C)

# Export library
add_library(hoedown STATIC
    # Sources
    src/autolink.c
    src/buffer.c
    src/document.c
    src/escape.c
    src/html.c
    src/html_blocks.c
    src/html_smartypants.c
    src/stack.c
    src/version.c

    # Headers
    src/autolink.h
    src/buffer.h
    src/document.h
    src/escape.h
    src/html.h
    src/stack.h
    src/version.h
)
target_include_directories(hoedown INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Alias to namespaced variant
add_library(Hoedown::Hoedown ALIAS hoedown)
