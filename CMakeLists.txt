cmake_minimum_required(VERSION 3.10)

project(Redis-Lite LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build Redis-Lite as a static library
add_library(redis_lite
    src/kv_store.cpp
    
    
)

target_include_directories(redis_lite PUBLIC include)

# Example executable
add_executable(redis_lite_example
    examples/example_usage.cpp
)

target_link_libraries(redis_lite_example
    redis_lite
)

add_executable(redis_lite_tests
    tests/basic_tests.cpp
)

target_link_libraries(redis_lite_tests
    redis_lite
)

