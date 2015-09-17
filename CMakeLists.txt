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
IF (WIN32)
  SET(OSPRAY_COMPILER "MSVC" CACHE INTERNAL "MSVC used on Windows.")
ELSE()
  IF (APPLE)
    SET(OSPRAY_COMPILER "CLANG" CACHE STRING "compiler to use (ICC,GCC, or CLANG)")
    SET_PROPERTY(CACHE OSPRAY_COMPILER PROPERTY STRINGS ICC GCC CLANG)
  ELSE(APPLE)
    SET(OSPRAY_COMPILER "GCC" CACHE STRING "compiler to use (ICC or GCC)")
    SET_PROPERTY(CACHE OSPRAY_COMPILER PROPERTY STRINGS ICC GCC)
  ENDIF(APPLE)
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
INCLUDE(${CMAKE_SOURCE_DIR}/cmake/ospray.cmake)

SET(LIBRARY_OUTPUT_PATH lib)
SET(EXECUTABLE_OUTPUT_PATH bin)

CONFIGURE_OSPRAY()

#TODO: use find_package config
INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}
  /opt
  /opt/ospray/include
  /opt/ospray/include/ospray/embree
  /opt/ospray/include/ospray/embree/common
)

##############################################################
# Build app
##############################################################

IF(NOT THIS_IS_MIC)

  # common utility classes for 3D viewer widget
  ADD_SUBDIRECTORY(common/widgets)

  # common xml utility classes
  ADD_SUBDIRECTORY(common/xml)

  ADD_SUBDIRECTORY(app)

  CONFIGURE_OSPRAY()
  INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/ospray/include)

ENDIF()
