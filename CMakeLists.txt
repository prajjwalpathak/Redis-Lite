cmake_minimum_required(VERSION 3.10)

project(Redis-Lite LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build Redis-Lite as a static library
add_library(redis_lite
    src/kv_store.cpp
    src/lru_policy.cpp
    src/lfu_policy.cpp
)

target_include_directories(redis_lite PUBLIC include)

# Example executable
# add_executable(example
#    examples/example_usage.cpp
#)

#target_link_libraries(example redis_lite)
