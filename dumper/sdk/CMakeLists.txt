cmake_minimum_required(VERSION 3.15)

project(dumper_sdk)

add_library(${PROJECT_NAME} STATIC impl.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/public)
