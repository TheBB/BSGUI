cmake_minimum_required(VERSION 2.8.11)

project(BSGUI CXX)

set(BSGUI_VERSION_MAJOR 0)
set(BSGUI_VERSION_MINOR 0)
set(BSGUI_VERSION_PATCH 2)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

add_definitions(-std=c++11)

find_package(Qt5Core)
find_package(Qt5Widgets)
find_package(Qt5Gui)
find_package(Qt5OpenGL)

add_subdirectory(src)