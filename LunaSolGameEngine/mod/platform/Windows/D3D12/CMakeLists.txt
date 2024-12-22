cmake_minimum_required(VERSION 3.28)

# add_library(d3d12_mod)
# add_library(d3d12_helper_mod)
message(STATUS "--- In Folder: ${CMAKE_CURRENT_SOURCE_DIR}")
target_sources(${PROJECT_NAME} PUBLIC
    FILE_SET CXX_MODULES 
    FILES
        Helpers/D3D12Utils.ixx
        Helpers/D3D12UtilsCommandList.ixx
        Helpers/D3D12UtilsDescriptors.ixx
        Helpers/D3D12UtilsPipeline.ixx
    )

target_sources(${PROJECT_NAME} PUBLIC FILE_SET CXX_MODULES
    FILES
        CommandListDx12.ixx
        CommandQueueDx12.ixx
        D3D12Common.ixx
        D3D12Lib.ixx
        DescriptorHeapDx12.ixx
        DeviceD3D12.ixx
        FrameBufferDxgi.ixx
        FrameDx12.ixx
        RendererDx12.ixx
        ResourceManagerD3D12.ixx
    )

# target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
# target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
# target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/Helpers)
# target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})