# root/CMakeLists.txt

# Specify the minimum CMake version required
cmake_minimum_required(VERSION 3.28)

# Set the project name
project(GameDemo VERSION 1.0 DESCRIPTION "A simple demonstration of LSE")

# Add the GameLib subdirectory
add_subdirectory(GameTest)

# Add the GameTest subdirectory
add_subdirectory(LunaSolGameEngine)
