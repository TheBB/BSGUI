# include_directories(${Qt5Widgets_INCLUDE_DIRS})
# include_directories(${Qt5Gui_INCLUDE_DIRS})
# include_directories(${Qt5OpenGL_INCLUDE_DIRS})

configure_file(
  "${CMAKE_CURRENT_LIST_DIR}/main.h.in"
  "${PROJECT_BINARY_DIR}/main.h"
  )

include_directories(${PROJECT_BINARY_DIR})

add_executable(BSGUI main.cpp)

qt5_use_modules(BSGUI Core Widgets Gui OpenGL)
target_link_libraries(BSGUI GL)

# target_link_libraries(BSGUI ${Qt5Widgets_LIBRARIES})
# target_link_libraries(BSGUI ${Qt5Gui_LIBRARIES})
# target_link_libraries(BSGUI ${Qt5OpenGL_LIBRARIES})

# message(STATUS ${Qt5OpenGL_INCLUDE_DIRS})
