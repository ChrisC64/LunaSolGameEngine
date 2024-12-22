cmake_minimum_required(VERSION 3.28)

add_library(engine_headers INTERFACE)

target_sources(engine_headers INTERFACE FILE_SET HEADERS)
target_include_directories(engine_headers INTERFACE ${CMAKE_CURRENT_LIST_DIR})