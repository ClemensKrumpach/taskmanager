cmake_minimum_required(VERSION 3.16)

project(taskmanager LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(NOTCURSES_CORE REQUIRED IMPORTED_TARGET notcurses-core)

add_executable(taskmanager
    src/main.c
    src/process.c
    src/system.c
)

add_executable(notcurses_demo src/notcurses_demo.c)
target_link_libraries(notcurses_demo PRIVATE PkgConfig::NOTCURSES_CORE)

target_include_directories(taskmanager PRIVATE include)

target_compile_options(taskmanager PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)