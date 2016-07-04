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

cmake_minimum_required(VERSION 2.8)

project(ospDebugViewer)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)

  if(${CMAKE_MAJOR_VERSION} EQUAL 3)
    cmake_policy(SET CMP0042 NEW)
  endif(${CMAKE_MAJOR_VERSION} EQUAL 3)
endif(COMMAND cmake_policy)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# force 'Release' build type on initial configuration (otherwise is empty)
set(CONFIGURATION_TYPES "Debug" "Release" "RelWithDebInfo")
if (WIN32)
  if (NOT DEFAULT_CMAKE_CONFIGURATION_TYPES_SET)
    set(CMAKE_CONFIGURATION_TYPES "${CONFIGURATION_TYPES}" CACHE STRING "List of generated configurations." FORCE)
    set(DEFAULT_CMAKE_CONFIGURATION_TYPES_SET ON CACHE INTERNAL "Default CMake configuration types set.")
  endif()
else()
  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${CONFIGURATION_TYPES})
  endif()
endif()

# unhide compiler to make it easier for users to see what they are using
mark_as_advanced(CLEAR CMAKE_CXX_COMPILER)

##############################################################
# Compiler configuration
##############################################################

# compiler selection for the non-MIC part
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Intel")
  include(${PROJECT_SOURCE_DIR}/cmake/icc.cmake)
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
  include(${PROJECT_SOURCE_DIR}/cmake/gcc.cmake)
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  include(${PROJECT_SOURCE_DIR}/cmake/clang.cmake)
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
  include(${PROJECT_SOURCE_DIR}/cmake/msvc.cmake)
else()
  message(FATAL_ERROR "Unsupported compiler: '${OSPRAY_COMPILER}'")
endif()

##############################################################
# Project configuration
##############################################################

include(${CMAKE_SOURCE_DIR}/cmake/glut.cmake)

set(LIBRARY_OUTPUT_PATH    ${CMAKE_BINARY_DIR})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

find_package(ospray REQUIRED)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads REQUIRED)

include_directories(
  ${CMAKE_SOURCE_DIR}
  ${OSPRAY_INCLUDE_DIRS}
  common
  common/ospcommon
  common/script/chaiscript
)

# RPath settings
set(CMAKE_SKIP_BUILD_RPATH  FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

##############################################################
# Build apps
##############################################################

# common utility classes
add_subdirectory(common)

# apps
add_subdirectory(app)
