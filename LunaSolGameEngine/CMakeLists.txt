cmake_minimum_required(VERSION 3.28)
project(LunaSolEngine VERSION 1.0 DESCRIPTION "A C++ Graphics/Game Engine with modern C++" LANGUAGES CXX)
message(STATUS "Building project: ${PROJECT_NAME}")
add_library(${PROJECT_NAME} STATIC)

message(STATUS "${PROJECT_NAME}: CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "${PROJECT_NAME}: VCPKG_INSTALLED_DIR: ${VCPKG_INSTALLED_DIR}")
message(STATUS "${PROJECT_NAME}: VCPKG_MANIFEST_DIR: ${VCPKG_MANIFEST_DIR}")

find_package(directxmath CONFIG REQUIRED)
find_package(directxtex CONFIG REQUIRED)
find_package(directxtk CONFIG REQUIRED)
find_package(directxtk12 CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)

add_subdirectory(inc)
add_subdirectory(mod)

set(COMPILER_FLAGS 
    $<$<CXX_COMPILER_ID:MSVC>:/W4;/MP;/GS;/Zc:wchar_t;/Zc:forScope>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall>
    )

if (MSVC)
target_compile_options(${PROJECT_NAME} PRIVATE ${COMPILER_FLAGS} 
    $<$<CONFIG:Debug>:/Od> $<$<CONFIG:Release>:/O2>
    )
add_compile_definitions(${PROJECT_NAME} PRIVATE 
    LS_WIN32_BUILD _UNICODE UNICODE MP
    $<$<CONFIG:Debug>:_DEBUG>
    )
endif()

target_compile_features(LunaSolEngine PRIVATE cxx_std_20)
target_link_libraries(LunaSolEngine 
    PRIVATE 
        assimp::assimp
        Microsoft::DirectXMath
        Microsoft::DirectXTex
        Microsoft::DirectXTK
        Microsoft::DirectXTK12
    )
