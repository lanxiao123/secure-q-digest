cmake_minimum_required (VERSION 2.8.11)
project (q-digest-mpc)
#SET(CMAKE_BUILD_TYPE "Debug")
set(NAME "q-digest-mpc")

find_path(CMAKE_FOLDER NAMES cmake/emp-tool-config.cmake)
include(${CMAKE_FOLDER}/cmake/common.cmake)
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

# Test cases
macro (add_test _name)
	add_test_with_lib(${_name} ${EMP-OT_LIBRARIES})
endmacro()

add_test (q-digest-example)
add_test (func-test)
add_test (merge-result-check)
add_test (q-digest-ocheck)
