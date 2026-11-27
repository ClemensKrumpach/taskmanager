cmake_minimum_required(VERSION 3.16)

project(taskmanager LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)

add_executable(taskmanager_gui
    src/taskmanager_gui.c
    src/process.c
)

target_include_directories(taskmanager_gui PRIVATE include)
target_link_libraries(taskmanager_gui PRIVATE PkgConfig::GTK4)
target_compile_options(taskmanager_gui PRIVATE -Wall -Wextra -Wpedantic)