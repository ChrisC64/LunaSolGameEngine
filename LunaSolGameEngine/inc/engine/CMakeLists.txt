cmake_minimum_required(VERSION 3.28)

# add_library(engine_headers INTERFACE)
target_sources(${PROJECT_NAME} INTERFACE FILE_SET HEADERS)
# target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_LIST_DIR})