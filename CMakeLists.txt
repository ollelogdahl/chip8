cmake_minimum_required(VERSION 3.17)
project(chip8)

set(CMAKE_C_STANDARD 99)

find_package(SDL2 REQUIRED)

add_executable(chip8 src/main.c src/cpu.c src/cpu.h src/emulator.c src/emulator.h)
target_link_libraries(chip8
        PRIVATE SDL2)