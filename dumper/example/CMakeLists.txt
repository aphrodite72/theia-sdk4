cmake_minimum_required(VERSION 3.15)

project(dumper_example)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} dumper_sdk)
