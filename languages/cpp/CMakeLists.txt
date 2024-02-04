cmake_minimum_required(VERSION 3.5)
project(theia_sdk)

add_library(theia_sdk INTERFACE)
target_include_directories(theia_sdk INTERFACE "${CMAKE_CURRENT_LIST_DIR}")
