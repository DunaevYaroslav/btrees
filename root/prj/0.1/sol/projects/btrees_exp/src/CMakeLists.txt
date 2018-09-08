add_executable(btrees_exp
        experiment.h
        main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../btrees_lib/src/btree.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../btrees_lib/src/btree.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../btrees_lib/src/indexer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../btrees_lib/src/indexer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../btrees_lib/src/utils.h
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../btrees_lib/src)
