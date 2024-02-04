cmake_minimum_required(VERSION 3.12)
project(autocrypted)

set(CMAKE_CXX_STANDARD 20)

option(AUTOCRYPTED_SEED "Root seed for autocrypted container" 12346567)

add_library(autocrypted INTERFACE)
target_include_directories(autocrypted INTERFACE "include")

# target_compile_definitions(autocrypted "AUTOCRYPTED_SEED=${AUTOCRYPTED_SEED}")
# make executable from the library
add_executable(autocrypted_test "example_autocrypted.cpp")
target_link_libraries(autocrypted_test PRIVATE autocrypted)