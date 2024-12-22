cmake_minimum_required(VERSION 3.28)

project(LunaSolEngine VERSION 1.0 DESCRIPTION "A simple cmake test" LANGUAGES CXX)
add_library(engine_headers INTERFACE)

target_sources(engine_headers INTERFACE FILE_SET HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(engine_headers INTERFACE ${CMAKE_CURRENT_LIST_DIR})

find_package(fmt CONFIG REQUIRED)

add_executable(cmake_tutorial cmake_test.cpp)

message(STATUS "    A simple status - Proj Source Dir: ${PROJECT_SOURCE_DIR}")

add_subdirectory(src)
add_subdirectory(src/windows)
add_subdirectory(modules)

target_compile_definitions(cmake_tutorial PRIVATE $<$<CONFIG:Debug>:_DEBUG=1>)
target_compile_features(cmake_tutorial PUBLIC cxx_std_20)
target_link_libraries(cmake_tutorial PRIVATE fmt::fmt windows_lib src_headers modules_lib)