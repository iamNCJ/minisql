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
include_directories(RecordManager)
aux_source_directory(RecordManager RECORDMANAGER)

# Index Manager
include_directories(IndexManager)
aux_source_directory(IndexManager INDEXMANAGER)

# Buffer Manager
include_directories(BufferManager)
aux_source_directory(BufferManager BUFFERMANAGER)

add_executable(MiniSQL ${BUFFERMANAGER} ${RECORDMANAGER} ${INDEXMANAGER} BPTree.h DataStructure.h main.cpp)
