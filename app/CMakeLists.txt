## ======================================================================== ##
## Copyright 2009-2015 Intel Corporation                                    ##
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

SET(APP_NAME ospDebugViewer)

ADD_SUBDIRECTORY(miniSG)

FIND_PACKAGE(Readline)

IF(READLINE_FOUND)
  ADD_DEFINITIONS(-DUSE_SYSTEM_READLINE)
  SET(Readline_INC ${Readline_INCLUDE_DIR})
  SET(Readline_LIB ${Readline_LIBRARY})
ELSE()
  MESSAGE(STATUS "Readline NOT found, command history feature NOT enabled.")
ENDIF()

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/script
  ${Readline_INC}
)

# --------------------------------------------
# main executable
# --------------------------------------------
ADD_EXECUTABLE(${APP_NAME}
  script/OSPRayScriptHandler.cpp
  script/OSPRayScriptHandler.h
  DebugViewerScriptHandler.cpp
  DebugViewerScriptHandler.h
  CommandLineSceneBuilder.cpp
  CommandLineSceneBuilder.h
  MSGViewer.cpp
  MSGViewer.h
  ViewerConfig.h
  modelViewer.cpp
  ospray_cpp.h
)

TARGET_LINK_LIBRARIES(${APP_NAME}
  ospray_minisg
  ospray_glut3d
  ${OPENGL_LIBRARIES}
  ${GLUT_LIBRARIES}
  ${OSPRAY_LIBRARIES}
  ${Readline_LIB}
)
# ------------------------------------------------------------

INSTALL(TARGETS ${APP_NAME} DESTINATION bin)
