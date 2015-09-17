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

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(modelViewer)

IF(COMMAND cmake_policy)
  CMAKE_POLICY(SET CMP0003 NEW)

  IF(${CMAKE_MAJOR_VERSION} EQUAL 3)
    CMAKE_POLICY(SET CMP0042 NEW)
  ENDIF(${CMAKE_MAJOR_VERSION} EQUAL 3)
ENDIF(COMMAND cmake_policy)

##############################################################
# Compiler configuration
##############################################################

# compiler selection for the non-MIC part
IF (${CMAKE_CXX_COMPILER_ID} STREQUAL "Intel")
  INCLUDE(${PROJECT_SOURCE_DIR}/cmake/icc.cmake)
ELSEIF (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
  INCLUDE(${PROJECT_SOURCE_DIR}/cmake/gcc.cmake)
ELSEIF (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  INCLUDE(${PROJECT_SOURCE_DIR}/cmake/clang.cmake)
ELSEIF (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
  INCLUDE(${PROJECT_SOURCE_DIR}/cmake/msvc.cmake)
ELSE()
  MESSAGE(FATAL_ERROR "Unsupported compiler: '${OSPRAY_COMPILER}'")
ENDIF()

# the arch we're targeting for the non-MIC/non-xeon phi part of ospray
SET(OSPRAY_BUILD_ISA "ALL" CACHE STRING "Target ISA (SSE, AVX, AVX2, or ALL)")
IF (OSPRAY_BUILD_ENABLE_KNL)
  SET_PROPERTY(CACHE OSPRAY_BUILD_ISA PROPERTY STRINGS ALL SSE AVX AVX2 AVX512)
  OPTION(OSPRAY_ISPC_KNL_NATIVE "Enable ISPC's native KNL build") # TODO: autodetect
ELSE()
  SET_PROPERTY(CACHE OSPRAY_BUILD_ISA PROPERTY STRINGS ALL SSE AVX AVX2)
ENDIF()

##############################################################
# Project configuration
##############################################################

INCLUDE(${CMAKE_SOURCE_DIR}/cmake/glut.cmake)

SET(LIBRARY_OUTPUT_PATH    ${CMAKE_BINARY_DIR}/lib)
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

find_package(ospray)

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}
  ${OSPRAY_INCLUDE_DIRS}
)

##############################################################
# Build app
##############################################################

# common utility classes for 3D viewer widget
ADD_SUBDIRECTORY(common/widgets)

# common xml utility classes
ADD_SUBDIRECTORY(common/xml)

ADD_SUBDIRECTORY(app)
