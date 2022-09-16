# ------------------------------------------
# User config between these lines

# Set project name
set(PROJECT "ruc")

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
	set(RUC_STANDALONE TRUE)
endif()

# Options
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(RUC_BUILD_TESTS "Build the RUC test programs" ${RUC_STANDALONE})

# ------------------------------------------

cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(${PROJECT} CXX)

# ------------------------------------------
# Setup C++ compiler

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags used for all build types
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
# -Wall      = All warnings about contructions that are easily avoidable
# -Wextra    = Extra warning flags not covered by -Wall
# -Wpedantic = Warnings for compiler extensions not part of the standard

# Set default build type if not specified
set(DEFAULT_BUILD_TYPE Release)
if(EXISTS ${CMAKE_SOURCE_DIR}/.git)
	set(DEFAULT_BUILD_TYPE Debug)
endif()
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE ${DEFAULT_BUILD_TYPE})
endif()

# Set build type specific compiler flags
message("--- ${CMAKE_BUILD_TYPE} ---")
if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og -g -pg")
	# -Og = Optimizations that do not interfere with debugging
	# -g  = Produce debugging information in OS's native format
	# -pg = Generate profile information for analysis with gprof
	# $ gprof <PROJECT> gmon.out > profile-data.txt
elseif(${CMAKE_BUILD_TYPE} STREQUAL Release)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
	# -O3 = Optimizations that increase compilation time and performance
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ------------------------------------------
# Library target

# Define library source files
file(GLOB_RECURSE LIBRARY_SOURCES "src/*.cpp")

add_library(${PROJECT} ${LIBRARY_SOURCES})
target_include_directories(${PROJECT} PRIVATE
	"src")

# ------------------------------------------
# Unit test library target

# Define source files
file(GLOB TEST_LIBRARY_SOURCES "test/*.cpp" "src/ruc/timer.cpp")

add_library(${PROJECT}-test ${TEST_LIBRARY_SOURCES})
target_include_directories(${PROJECT}-test PRIVATE
	"src"
	"test")

# ------------------------------------------
# Unit test target

if (RUC_BUILD_TESTS)
	# Define test source files
	file(GLOB_RECURSE TEST_SOURCES "test/unit/*.cpp")

	add_executable(${PROJECT}-unit-test ${TEST_SOURCES})
	target_include_directories(${PROJECT}-unit-test PRIVATE
		"src"
		"test")
	target_link_libraries(${PROJECT}-unit-test ruc)
	target_link_libraries(${PROJECT}-unit-test ruc-test)
endif()
