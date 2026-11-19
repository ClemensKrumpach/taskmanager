cmake_minimum_required(VERSION 3.16)

project(taskmanager LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(taskmanager
    src/main.c
    src/process.c
    src/system.c
)

target_include_directories(taskmanager PRIVATE include)

target_compile_options(taskmanager PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)