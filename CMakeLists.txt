cmake_minimum_required (VERSION 2.8.11)
project (q-digest-mpc)
#SET(CMAKE_BUILD_TYPE "Debug")
set(NAME "q-digest-mpc")

find_path(CMAKE_FOLDER NAMES cmake/emp-tool-config.cmake)
include(${CMAKE_FOLDER}/cmake/emp-base.cmake)
#include(${CMAKE_FOLDER}/cmake/source_of_randomness.cmake)
#include(${CMAKE_FOLDER}/cmake/threading.cmake)

find_package(emp-tool REQUIRED)
include_directories(${EMP-TOOL_INCLUDE_DIRS})
find_package(emp-ot REQUIRED)
include_directories(${EMP-OT_INCLUDE_DIRS})
find_package(emp-sh2pc REQUIRED)
include_directories(${EMP-SH2PC_INCLUDE_DIRS})
 
# Installation
install(FILES cmake/q-digest-mpc-config.cmake DESTINATION cmake/)
install(DIRECTORY q-digest-mpc DESTINATION include/)

ENABLE_TESTING()
ADD_SUBDIRECTORY(test)
