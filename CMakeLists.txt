CMAKE_MINIMUM_REQUIRED(VERSION 3.1.0)

PROJECT(SM-IPC-DEMO)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra -Wno-unused-parameter")
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)


set(BOOST_ROOT "" CACHE PATH "")

if (BOOST_ROOT STREQUAL "") 
message(FATAL_ERROR ":Required Boost source directory path")
endif () 

INCLUDE_DIRECTORIES(${BOOST_ROOT})

ADD_SUBDIRECTORY(tiny-process-library-master)
ADD_SUBDIRECTORY(HelloWorld)
ADD_SUBDIRECTORY(Executor)
