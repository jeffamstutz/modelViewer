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

set(LIBRARY_OUTPUT_PATH    ${CMAKE_BINARY_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

find_package(ospray REQUIRED)

SET(CMAKE_THREAD_PREFER_PTHREAD TRUE)
FIND_PACKAGE(Threads REQUIRED)

include_directories(
  ${CMAKE_SOURCE_DIR}
  ${OSPRAY_INCLUDE_DIRS}
  common
  common/script/chaiscript
)

# RPath settings
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

##############################################################
# Build apps
##############################################################

# common utility classes
add_subdirectory(common)

# debug app
add_subdirectory(app)

# benchmarks
option(BUILD_BENCHMARKS "Enable/Disable building of benchmark apps" ON)
if(BUILD_BENCHMARKS)
  add_subdirectory(bench)
endif()
