cmake_minimum_required(VERSION 3.28)

add_subdirectory(engine)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})