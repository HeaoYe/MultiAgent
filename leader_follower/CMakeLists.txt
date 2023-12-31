cmake_minimum_required(VERSION 3.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
enable_language(CXX)

project(MultiAgent LANGUAGES CXX)

add_executable(MultiAgent src/main.cpp)
