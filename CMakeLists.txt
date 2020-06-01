cmake_minimum_required(VERSION 3.14)
project(MiniSQL)

set(CMAKE_CXX_STANDARD 17)

# Interpreter
include_directories(Interpreter)
aux_source_directory(Interpreter INTERPRETER)

# API
include_directories(API)
aux_source_directory(API API)

# Catalog Manager
include_directories(CatalogManager)
aux_source_directory(CatalogManager CATALOGMANAGER)

# Record Manager
# Index Manager
# Buffer Manager
include_directories(BufferManager)
aux_source_directory(BufferManager BUFFERMANAGER)

add_executable(MiniSQL ${INTERPRETER} ${API} ${CATALOGMANAGER} ${BUFFERMANAGER} DataStructure.h main.cpp)
