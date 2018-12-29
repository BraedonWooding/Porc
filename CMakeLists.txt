cmake_minimum_required (VERSION 2.8.11)
set(CMAKE_C_STANDARD 11)
project (LLV)
add_executable(porc src/ast.c src/main.c src/map.c src/parser.c src/str.c src/tokenizer.c src/vector.c)
