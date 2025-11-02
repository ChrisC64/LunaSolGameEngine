cmake_minimum_required(VERSION 3.28)
project(GameTest VERSION 1.0 DESCRIPTION "A simple hello world example for my engine")
message(STATUS "Building the Game Test")
add_executable(${PROJECT_NAME} main.cpp)

message(STATUS "${PROJECT_NAME}: CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "${PROJECT_NAME}: VCPKG_INSTALLED_DIR: ${VCPKG_INSTALLED_DIR}")
message(STATUS "${PROJECT_NAME}: VCPKG_MANIFEST_DIR: ${VCPKG_MANIFEST_DIR}")

set(COMPILER_FLAGS 
    $<$<CXX_COMPILER_ID:MSVC>:/Wall;/MP;/GS;>
    $<$<CXX_COMPILER_ID:GNU>:-Wall>
    )

if (MSVC)
target_compile_options(${PROJECT_NAME} PRIVATE ${COMPILER_FLAGS} 
    $<$<CONFIG:Debug>:/Od;> $<$<CONFIG:Release>:/O2>
    )
target_compile_definitions(${PROJECT_NAME} PRIVATE 
    LS_WIN32_BUILD _UNICODE UNICODE MP
    $<$<CONFIG:Debug>:_DEBUG>
    )
endif()
target_link_libraries(${PROJECT_NAME}
    PRIVATE LunaSolEngine)