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

ADD_SUBDIRECTORY(miniSG)

# TODO: write the find_package module for ChaiScript...
#find_package(ChaiScript)

SET(ChaiScript_ROOT "/opt/chaiscript" CACHE PATH "install path of ChaiScript")

SET(ChaiScript_INCLUDE_DIR ${ChaiScript_ROOT}/include)
SET(ChaiScript_LIBRARIES
  ${ChaiScript_ROOT}/lib/chaiscript/libchaiscript_stdlib-5.7.2${CMAKE_SHARED_LIBRARY_SUFFIX}
  #${ChaiScript_ROOT}/lib/chaiscript/libstl_extra${CMAKE_SHARED_LIBRARY_SUFFIX}
)

INCLUDE_DIRECTORIES(${ChaiScript_INCLUDE_DIR})

# --------------------------------------------
# main executable
# --------------------------------------------
ADD_EXECUTABLE(ospModelViewer
  CommandLineSceneBuilder.cpp
  CommandLineSceneBuilder.h
  MSGViewer.cpp
  MSGViewer.h
  ViewerConfig.h
  modelViewer.cpp
)

TARGET_LINK_LIBRARIES(ospModelViewer
  ospray_minisg
  ospray_glut3d
  ${OPENGL_LIBRARIES}
  ${GLUT_LIBRARIES}
  ${OSPRAY_LIBRARIES}
  ${ChaiScript_LIBRARIES}
)
# ------------------------------------------------------------

INSTALL(TARGETS ospModelViewer DESTINATION bin)
