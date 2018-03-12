cmake_minimum_required(VERSION 3.8)
project(btrees CXX)

set(CMAKE_CXX_STANDARD 11)

#set(SOURCE_FILES
#        root/prj/0.1/sol/projects/btrees_lib/src/utils.h
#        root/prj/0.1/sol/projects/btrees_lib/src/btree.h
#        root/prj/0.1/sol/projects/btrees_lib/src/btree.cpp
#        root/prj/0.1/sol/projects/btrees_lib/src/indexer.h
#        root/prj/0.1/sol/projects/btrees_lib/src/indexer.cpp)
#set(TEST_SOURCE_FILES
#        root/prj/0.1/sol/tests/btrees_lib_tests)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_BINARY_DIR}/bin)
#set(INTERMEDIATE_DIR ${CMAKE_CURRENT_BINARY_DIR}/intermediate)

#add_library(btrees ${SOURCE_FILES} ${TEST_SOURCE_FILES})

add_subdirectory(root/prj/0.1/sol/projects/btrees_lib/src)
add_subdirectory(root/prj/0.1/sol/tests/btrees_lib_tests)
