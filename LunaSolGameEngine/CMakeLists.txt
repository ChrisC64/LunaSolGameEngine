cmake_minimum_required(VERSION 3.28)
project(LunaSolEngine VERSION 1.0 DESCRIPTION "A C++ Graphics/Game Engine with modern C++" LANGUAGES CXX)
message(STATUS "Building project: ${PROJECT_NAME}")
add_library(${PROJECT_NAME} STATIC)

find_package(directxmath CONFIG REQUIRED)
find_package(directxtex CONFIG REQUIRED)
find_package(directxtk CONFIG REQUIRED)
find_package(directxtk12 CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)

add_subdirectory(inc)
add_subdirectory(mod)

set(COMPILER_FLAGS 
    $<$<CXX_COMPILER_ID:MSVC>: W4 MP GS Zc:wchar_t Zc:forscope> 
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>: Wall>
    )

if (WIN32)
target_compile_definitions(${PROJECT_NAME} PRIVATE 
    $<$<CONFIG:Debug>:_DEBUG Od> $<$<CONFIG:Release>: O2> 
    LS_WIN32_BUILD _UNICODE UNICODE ${COMPILER_FLAGS}
    )
endif()

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
        assimp::assimp
        Microsoft::DirectXMath
        Microsoft::DirectXTex
        Microsoft::DirectXTK
        Microsoft::DirectXTK12
    )
