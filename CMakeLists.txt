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

add_library(interpreter_common STATIC ${LIB_SOURCES})

SET ( APP_EXE Porc )

add_executable(${APP_EXE} ${SOURCES})
target_link_libraries(${APP_EXE} interpreter_common)


