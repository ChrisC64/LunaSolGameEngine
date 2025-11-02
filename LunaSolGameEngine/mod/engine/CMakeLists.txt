cmake_minimum_required(VERSION 3.28)

# add_library(engine_mod)
message(STATUS "--- In Folder: ${CMAKE_CURRENT_SOURCE_DIR}")
target_sources(${PROJECT_NAME} PUBLIC
    FILE_SET engine_module TYPE CXX_MODULES 
    FILES
        App.ixx
        Camera.ixx
        Clock.ixx
        Codes.ixx
        Defines.ixx
        Device.ixx
        Input.ixx
        Logger.ixx
        LSEngine.ixx
        Shader.ixx
        Window.ixx
    )

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})