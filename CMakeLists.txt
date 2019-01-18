cmake_minimum_required(VERSION 2.8)
project(tinyrdb)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

aux_source_directory(. DIR_LIB_SRCS)

add_library (tinyrdb ${DIR_LIB_SRCS})


