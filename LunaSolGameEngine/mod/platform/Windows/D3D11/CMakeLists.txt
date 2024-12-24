cmake_minimum_required(VERSION 3.28)

# add_library(d3d11_mod)
# add_library(d3d11_helper_mod)
message(STATUS "--- In Folder: ${CMAKE_CURRENT_SOURCE_DIR}")
target_sources(${PROJECT_NAME} PUBLIC
    FILE_SET d3d11_helper_module TYPE CXX_MODULES 
    FILES
        Helpers/D3D11HelperStates.ixx
        Helpers/D3D11MemoryHelper.ixx
        Helpers/D3D11TypeWrapper.ixx
        Helpers/D3D11Utils.ixx
    )

target_sources(${PROJECT_NAME} PUBLIC 
    FILE_SET d3d11_module TYPE CXX_MODULES
    FILES
        D3D11Lib.ixx
        DeviceD3D11.ixx
        RenderCommandD3D11.ixx
        RenderD3D11.ixx
        RenderFuncs.ixx
        ShaderD3D11.ixx
        )

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_include_directories(${PROJECT_NAME} 
    PUBLIC 
    ${CMAKE_CURRENT_LIST_DIR} 
    ${CMAKE_CURRENT_LIST_DIR}/Helpers
    )