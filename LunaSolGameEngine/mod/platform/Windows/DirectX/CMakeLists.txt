cmake_minimum_required(VERSION 3.28)

# add_library(directx_mod)
message(STATUS "--- In Folder: ${CMAKE_CURRENT_SOURCE_DIR}")
target_sources(${PROJECT_NAME} PUBLIC
    FILE_SET directx_module TYPE CXX_MODULES 
    FILES
        D3DCompiler.ixx
        DirectXCommon.ixx
        DXCamera.ixx
        DXCameraController.ixx
        DXMatrixUtils.ixx
    )

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})