cmake_minimum_required(VERSION 3.8)
project(btrees CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -static-libgcc -static-libstdc++")

add_subdirectory(root/prj/0.1/sol/projects/btrees_lib/src)
add_subdirectory(root/prj/0.1/sol/tests/btrees_lib_tests)
add_subdirectory(root/prj/0.1/sol/projects/btrees_exp/src)
add_subdirectory(root/prj/0.1/sol/projects/csv_generator/src)
