## ======================================================================== ##
## Copyright 2009-2016 Intel Corporation                                    ##
##                                                                          ##
## Licensed under the Apache License, Version 2.0 (the "License");          ##
## you may not use this file except in compliance with the License.         ##
## You may obtain a copy of the License at                                  ##
##                                                                          ##
##     http://www.apache.org/licenses/LICENSE-2.0                           ##
##                                                                          ##
## Unless required by applicable law or agreed to in writing, software      ##
## distributed under the License is distributed on an "AS IS" BASIS,        ##
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. ##
## See the License for the specific language governing permissions and      ##
## limitations under the License.                                           ##
## ======================================================================== ##

set(APP_NAME ospDebugViewerQt)

project(${APP_NAME})

find_package(Qt4 COMPONENTS QtCore QtGui QtOpenGL REQUIRED)
find_package(OpenGL REQUIRED)

include(${QT_USE_FILE})

include_directories(
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/script
  ${PROJECT_BINARY_DIR}
)

qt4_wrap_ui(UIS
  MainWindow.ui
)

qt4_wrap_cpp(CPPS
  widgets/QConsole.h
  widgets/QChaiConsole.h
  widgets/QOSPRayWindow.h
  MainWindow.h
)

add_executable(${APP_NAME}
  widgets/QConsole.cpp
  widgets/QChaiConsole.cpp
  widgets/QOSPRayWindow.cpp
  widgets/Viewport.h
  main.cpp
  MainWindow.cpp
  ${UIS}
  ${CPPS}
)

target_link_libraries(${APP_NAME}
  ospray_commandline
  ospray_common
  ospray_minisg
  ospray_script
  ${OSPRAY_LIBRARIES}
  ${QT_LIBRARIES}
  ${OPENGL_LIBRARIES}
)

install(TARGETS ${APP_NAME} DESTINATION bin)
