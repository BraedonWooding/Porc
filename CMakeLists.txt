cmake_minimum_required (VERSION 3.14)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project (Porc)
include_directories(SYSTEM lib)
include_directories(interpreter_common)
include_directories(compiler)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH};"lib/Catch2/contrib")
add_subdirectory(lib/Catch2)
ENABLE_TESTING()

file(GLOB SOURCES
	compiler/*.cpp
	compiler/*.hpp
	compiler/*.inc
)

file(GLOB LIB_SOURCES
	interpreter_common/*.h
	interpreter_common/*.c
)

file(GLOB APP_SOURCES
	app/*.hpp
	app/*.cpp
)

file(GLOB TEST_PARSER_SOURCES
	tests/test_output/*.cpp)

add_library(interpreter_common STATIC ${LIB_SOURCES})
add_library(porc_backend STATIC ${SOURCES})

SET ( APP_EXE Porc )

add_executable(${APP_EXE} ${APP_SOURCES})
target_link_libraries(${APP_EXE} interpreter_common)
target_link_libraries(${APP_EXE} porc_backend)

add_executable(test_output ${TEST_PARSER_SOURCES})
target_link_libraries(test_output porc_backend)
target_link_libraries(test_output Catch2::Catch2)

include(CTest)
include(lib/Catch2/contrib/Catch.cmake)
catch_discover_tests(test_output)
