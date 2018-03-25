add_executable(btrees_lib_tests
    individual.h
    btree_test.cpp
    bplustree_test.cpp
    btree_based_index_tests.cpp
    bplustree_based_index_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../projects/btrees_lib/src/utils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../projects/btrees_lib/src/btree.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../projects/btrees_lib/src/btree.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../projects/btrees_lib/src/indexer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../projects/btrees_lib/src/indexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gtest-fus/gtest.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gtest-fus/gtest-all.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/gtest-fus/gtest_main.cc
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../../projects/btrees_lib/src)
