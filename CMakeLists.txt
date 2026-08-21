cmake_minimum_required(VERSION 3.10.0)
project(short-notes VERSION 0.1.0 LANGUAGES C)

add_executable(short-notes
    src/main.c
    src/add_note.c
)
target_include_directories(short-notes PRIVATE include)
