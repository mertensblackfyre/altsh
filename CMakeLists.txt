cmake_minimum_required(VERSION 3.10)
project(CoralisProject)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g3")

include_directories(${PROJECT_SOURCE_DIR}/include)

enable_testing()

add_subdirectory(src)

add_subdirectory(tests)
