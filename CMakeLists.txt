cmake_minimum_required(VERSION 2.8.11)

project(BSGUI CXX)

set(BSGUI_VERSION_MAJOR 0)
set(BSGUI_VERSION_MINOR 0)
set(BSGUI_VERSION_PATCH 8)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

add_definitions(-std=c++11)

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5OpenGL REQUIRED)
find_package(GoTools REQUIRED)
find_package(GoTrivariate REQUIRED)

configure_file(
  "${CMAKE_CURRENT_LIST_DIR}/src/main.h.in"
  "${PROJECT_BINARY_DIR}/main.h"
  )

include_directories(${PROJECT_BINARY_DIR})
include_directories(${GoTools_INCLUDE_DIRS})
include_directories(${GoTrivariate_INCLUDE_DIRS})
include_directories(src)

qt5_add_resources(RESOURCES resources/resources.qrc)

add_executable(BSGUI
  ${RESOURCES}
  src/main.cpp
  src/GLWidget.cpp
  src/MainWindow.cpp
  src/ObjectSet.cpp
  src/ToolBox.cpp
  src/DisplayObject.cpp
  src/DisplayObjects/Volume.cpp
  )

qt5_use_modules(BSGUI Core Widgets Gui OpenGL)

target_link_libraries(BSGUI
  ${GoTrivariate_LIBRARIES}
  ${GoTools_LIBRARIES}
  GL
  )

install(TARGETS BSGUI RUNTIME DESTINATION bin)