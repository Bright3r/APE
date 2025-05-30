cmake_minimum_required(VERSION 3.10)

project(ape)

find_package(SDL3 CONFIG REQUIRED)

add_executable(ape main.cpp)

target_link_libraries(ape PRIVATE SDL3::SDL3)
