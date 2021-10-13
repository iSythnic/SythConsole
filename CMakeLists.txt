cmake_minimum_required(VERSION 3.20.5)
set(CMAKE_CXX_STANDARD 17)
add_definitions(-DUNICODE -D_UNICODE)

project(SythConsole VERSION 1.0 DESCRIPTION "Console Graphics Engine")
set_target_properties(SythConsole PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 1)

add_library(SythConsole STATIC src/SythConsole.cpp)
target_include_directories (SythConsole PRIVATE include)