set(PROJECT_NAME raytracer)

# https://stackoverflow.com/questions/10851247/how-to-activate-c-11-in-cmake
# Fix behavior of CMAKE_CXX_STANDARD when targeting macOS.
if(POLICY CMP0025)
    cmake_policy(SET CMP0025 NEW)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# https://trenki2.github.io/blog/2017/06/02/using-sdl2-with-cmake/
# Include the FindSDL2.cmake file in order to find SDL2 on all platforms
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

cmake_minimum_required(VERSION 3.15.0)
set(CMAKE_CXX_STANDARD 11)

project(${PROJECT_NAME})

# Can manually add sources as follows if desired:
# set(SOURCES src/main.cpp src/anotherfile.cpp)

# Use wildcard to include all source files under /src
# Can also use GLOB_RECURSE
file(GLOB SOURCES "src/*.cpp")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

# set(SDL2_INCLUDE_DIRS /Library/Frameworks/SDL2.framework/Headers)
# set(SDL2_LIBRARIES /Library/Frameworks/SDL2/framework/SDL2)
# find_package(SDL2 REQUIRED)
# find_package(SDL2_image REQUIRED)
# include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS})

set(BINDINGS
    # ${CMAKE_CURRENT_SOURCE_DIR}/bindings/imgui_impl_opengl2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/bindings/imgui_impl_sdl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/bindings/imgui_impl_sdl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/bindings/imgui_impl_sdlrenderer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/bindings/imgui_impl_sdlrenderer.cpp
)
# file(GLOB BINDINGS "bindings/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES} ${BINDINGS})
target_link_libraries(${PROJECT_NAME} ${CONAN_LIBS})