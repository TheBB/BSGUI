cmake_minimum_required(VERSION 2.8.11)

project(BSGUI CXX)

set(BSGUI_VERSION_MAJOR 0)
set(BSGUI_VERSION_MINOR 0)
set(BSGUI_VERSION_PATCH 7)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

add_definitions(-std=c++11)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5OpenGL REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(GoTools REQUIRED)

add_subdirectory(src)