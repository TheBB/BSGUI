configure_file(
  "${CMAKE_CURRENT_LIST_DIR}/main.h.in"
  "${PROJECT_BINARY_DIR}/main.h"
  )

include_directories(${PROJECT_BINARY_DIR})
include_directories(${EIGEN3_INCLUDE_DIR})
include_directories(${GoTools_INCLUDE_DIRS})

add_executable(BSGUI
  main.cpp
  GLWidget.cpp
  MainWindow.cpp
  ObjectSet.cpp
  ToolBox.cpp
  DisplayObject.cpp
  DisplayObjects/Volume.cpp
  )

qt5_use_modules(BSGUI Core Widgets Gui OpenGL)
target_link_libraries(BSGUI GL)
target_link_libraries(BSGUI ${GoTools_LIBRARIES})

install(TARGETS BSGUI RUNTIME DESTINATION bin)