cmake_minimum_required(VERSION 3.28)

# add_library(windows_lib_mod)
# add_library(d3d11_helper_mod STATIC)
# add_library(d3d11_mod STATIC)
# add_library(d3d12_mod STATIC)
# add_library(d3d12_helper_mod STATIC)
# add_library(directx_mod STATIC)
# add_library(dxgi_mod STATIC)
# add_library(win32_mod STATIC)

add_subdirectory(D3D11)
add_subdirectory(D3D12)
add_subdirectory(DirectX)
add_subdirectory(DXGI)
add_subdirectory(Win32)
message(STATUS "--- In Folder: ${CMAKE_CURRENT_SOURCE_DIR}")
target_sources(${PROJECT_NAME} PUBLIC
    FILE_SET CXX_MODULES 
    FILES
        WindowsLib.ixx
    )

# target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
# target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})