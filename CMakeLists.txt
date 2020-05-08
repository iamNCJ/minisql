cmake_minimum_required(VERSION 3.14)
project(MiniSQL)

set(CMAKE_CXX_STANDARD 17)

# Interpreter
include_directories(Interpreter)
aux_source_directory(Interpreter INTERPRETER)

# API

# Catalog Manager

# Record Manager
# Index Manager
# Buffer Manager

add_executable(MiniSQL ${INTERPRETER} main.cpp)