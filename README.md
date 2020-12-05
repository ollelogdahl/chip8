# Chip8
### A chip8 emulator written in C
Olle Logdahl, 14 August 2020

Emulator for the complete [instruction set](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM) of *chip8*. Uses [SDL2](https://www.libsdl.org/download-2.0.php) for rendering. Everything works except sound.

## Installation / Getting Started
*Note: developed for linux. Codebase should be portable, but the cmake build may need to be modified.*
```bash
git clone https://github.com/ollelogdahl/chip8.git
mkdir build
cd build
cmake .. && make -j4
```
Usage:
```bash
chip8 <rom name>
```
