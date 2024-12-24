cmake_minimum_required(VERSION 3.28)

# add_library(win32_mod)

target_sources(${PROJECT_NAME} PUBLIC
    FILE_SET win32_module TYPE CXX_MODULES 
    FILES
        ComUtils.ixx
        Win32App.ixx
        Win32Utils.ixx
        Win32Window.ixx
    )

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})