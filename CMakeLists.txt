cmake_minimum_required (VERSION 3.14)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project (Porc)
include_directories(SYSTEM lib)
include_directories(interpreter_common)
file(GLOB SOURCES
	src/*.cpp
	src/*.hpp
	src/*.inc
)

file(GLOB LIB_SOURCES
	interpreter_common/*.h
	interpreter_common/*.c
)

add_library(interpreter_common ${LIB_SOURCES})
add_executable(porc ${SOURCES})
