configure_file(
  "${CMAKE_CURRENT_LIST_DIR}/main.h.in"
  "${PROJECT_BINARY_DIR}/main.h"
  )

include_directories(${PROJECT_BINARY_DIR})

add_executable(BSGUI
  main.cpp
  glwidget.cpp
  dispobject.cpp)

qt5_use_modules(BSGUI Core Widgets Gui OpenGL)
target_link_libraries(BSGUI GL)
