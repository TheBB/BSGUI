cmake_minimum_required(VERSION 2.8.11)

project(BSGUI CXX C)


# Custom modules

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")


# BSGUI version

set(BSGUI_VERSION_MAJOR 0)
set(BSGUI_VERSION_MINOR 1)
set(BSGUI_VERSION_PATCH 2)


# MOC is needed for Qt

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)


# We want C++11, because it's cool

include(TestCXXAcceptsFlag)
check_cxx_accepts_flag("-std=c++11" CXX_FLAG_CXX11)
if(CXX_FLAG_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
  check_cxx_accepts_flag("-std=gnu++0x" CXX_FLAG_GNU)
  if(CXX_FLAG_GNU)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")
  else()
    message(FATAL_ERROR "C++11 support is required, bailing")
  endif()
endif()


# Include and build Ogre

find_package(OGRE REQUIRED)
set(OGRE_RESOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/ogre/resources")


# Find Qt5, GoTools and OpenGL

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5OpenGL REQUIRED)
find_package(Qt5X11Extras REQUIRED)
find_package(GoTools REQUIRED)
find_package(GoTrivariate REQUIRED)
find_package(OpenGL REQUIRED)


# Configure files

configure_file(
  "${CMAKE_CURRENT_LIST_DIR}/src/main.h.in"
  "${PROJECT_BINARY_DIR}/main.h")

configure_file(
  "${CMAKE_CURRENT_LIST_DIR}/ogre/plugins.cfg.in"
  "${PROJECT_BINARY_DIR}/ogre/plugins.cfg")

configure_file(
  "${CMAKE_CURRENT_LIST_DIR}/ogre/resources.cfg.in"
  "${PROJECT_BINARY_DIR}/ogre/resources.cfg")

configure_file(
  "${CMAKE_CURRENT_LIST_DIR}/Doxyfile.in"
  "${PROJECT_BINARY_DIR}/Doxyfile")


# Includes

include_directories(${PROJECT_BINARY_DIR})
include_directories(${GoTools_INCLUDE_DIRS})
include_directories(${GoTrivariate_INCLUDE_DIRS})
include_directories(${OPENGL_INCLUDE_DIRS})
include_directories(${OGRE_INCLUDE_DIRS})
include_directories(src)


# Varia

qt5_add_resources(RESOURCES resources/resources.qrc)


# Executable BSGUI

add_executable(BSGUI
  ${RESOURCES}
  src/main.cpp
  src/MainWindow.cpp
  src/QOgreWidget.cpp
  # src/GLWidget.cpp
  # src/ObjectSet.cpp
  # src/ToolBox.cpp
  # src/InfoBox.cpp
  # src/DisplayObject.cpp
  # src/DisplayObjects/Volume.cpp
  # src/DisplayObjects/Surface.cpp
  # src/DisplayObjects/Curve.cpp
  )

qt5_use_modules(BSGUI Core Widgets Gui OpenGL X11Extras)

target_link_libraries(BSGUI
  ${GoTrivariate_LIBRARIES}
  ${GoTools_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${OGRE_LIBRARIES}
  )


# Install

install(TARGETS BSGUI RUNTIME DESTINATION bin)


# Docs

find_package(Doxygen)
if(DOXYGEN_FOUND)
  add_custom_target(doc doxygen ${PROJECT_BINARY_DIR}/Doxyfile
                    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                    COMMENT "Generating API documentation" VERBATIM)
endif()
