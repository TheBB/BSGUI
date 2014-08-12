cmake_minimum_required(VERSION 2.8.11)

project(BSGUI CXX C)

set(BSGUI_VERSION_MAJOR 0)
set(BSGUI_VERSION_MINOR 1)
set(BSGUI_VERSION_PATCH 2)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

add_definitions(-std=c++11)

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5OpenGL REQUIRED)
find_package(GoTools REQUIRED)
find_package(GoTrivariate REQUIRED)
find_package(OpenGL REQUIRED)

configure_file(
  "${CMAKE_CURRENT_LIST_DIR}/src/main.h.in"
  "${PROJECT_BINARY_DIR}/main.h"
  )

configure_file(
  "${CMAKE_CURRENT_LIST_DIR}/Doxyfile.in"
  "${PROJECT_BINARY_DIR}/Doxyfile"
  )

include_directories(${PROJECT_BINARY_DIR})
include_directories(${GoTools_INCLUDE_DIRS})
include_directories(${GoTrivariate_INCLUDE_DIRS})
include_directories(${OPENGL_INCLUDE_DIRS})
include_directories(src)

qt5_add_resources(RESOURCES resources/resources.qrc)

add_executable(BSGUI
  ${RESOURCES}
  src/main.cpp
  src/GLWidget.cpp
  src/MainWindow.cpp
  src/ObjectSet.cpp
  src/ToolBox.cpp
  src/InfoBox.cpp
  src/DisplayObject.cpp
  src/DisplayObjects/Volume.cpp
  src/DisplayObjects/Surface.cpp
  src/DisplayObjects/Curve.cpp
  )

qt5_use_modules(BSGUI Core Widgets Gui OpenGL)

target_link_libraries(BSGUI
  ${GoTrivariate_LIBRARIES}
  ${GoTools_LIBRARIES}
  ${OPENGL_LIBRARIES}
  )

install(TARGETS BSGUI RUNTIME DESTINATION bin)

# For generating the doxy
find_package(Doxygen)
if(DOXYGEN_FOUND)
  add_custom_target(doc doxygen ${PROJECT_BINARY_DIR}/Doxyfile
                    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                    COMMENT "Generating API documentation" VERBATIM)
endif()
