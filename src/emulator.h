//
// Created by olle on 2020-08-12.
//

#ifndef CHIP8_EMULATOR_H
#define CHIP8_EMULATOR_H

#include "cpu.h"

#include <stdbool.h>

static unsigned char const BG_R = 35;
static unsigned char const BG_G = 20;
static unsigned char const BG_B = 35;

static unsigned char const FG_R = 255;
static unsigned char const FG_G = 120;
static unsigned char const FG_B = 255;

static unsigned const PIXEL_SIZE = 12;
static unsigned const BEEP_AMPLITUDE = 10;
static unsigned const BEEP_FREQUENCY = 28000;

int initialize_emulator(unsigned char verbose_lvl);
// Loads a rom into memory
int load_rom(const char *file);

// Starts the emulator.
void run();
// Performs one cycle; performing multiple cpu updates, and updating timers.
void cycle();

void handleNativeEvents();

bool isKeyPressed(byte k);
// returns a pointer to the first key pressed. returns null if none.
// Returns the first key pressed by index. if no index pressed, return 255.
byte getNextKeypress();

void render_buffer();

void print_memory();

#endif //CHIP8_EMULATOR_H
