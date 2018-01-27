cmake_minimum_required(VERSION 3.8)
project(btrees)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        root/prj/0.1/sol/projects/btrees_lib/src/utils.h
        root/prj/0.1/sol/projects/btrees_lib/src/btree.h
        root/prj/0.1/sol/projects/btrees_lib/src/btree.cpp
        root/prj/0.1/sol/projects/btrees_lib/src/indexer.h
        root/prj/0.1/sol/projects/btrees_lib/src/indexer.cpp)

add_library(btrees ${SOURCE_FILES})
