cmake_minimum_required(VERSION 3.28)
project(LunaSolEngine VERSION 1.0 DESCRIPTION "A C++ Graphics/Game Engine with modern C++" LANGUAGES CXX)
message(STATUS "Building project: ${PROJECT_NAME}")
add_library(${PROJECT_NAME} STATIC)

find_package(fmt CONFIG REQUIRED)
find_package(directxmath CONFIG REQUIRED)
find_package(directx-headers CONFIG REQUIRED)
find_package(directxtex CONFIG REQUIRED)
find_package(directxtk CONFIG REQUIRED)
find_package(directxtk12 CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)

add_subdirectory(inc/engine)
add_subdirectory(mod)

target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:_DEBUG=1> $<$<CMAKE_HOST_WIN32>:LS_WINDOWS_BUILD=1>)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
        fmt::fmt 
        assimp::assimp
        Microsoft::DirectX-Headers
        Microsoft::DirectXMath
        Microsoft::DirectXTex
        Microsoft::DirectXTK
        Microsoft::DirectXTK12
        engine_headers 
    )
