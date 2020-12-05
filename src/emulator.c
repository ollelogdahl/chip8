//
// Created by olle on 2020-08-12.
//

#include "emulator.h"

#include <SDL2/SDL.h>
#include <stdio.h>
#include <errno.h>

// Macro for verbose printing. 0 = no prints, 1 = only info, etc.
unsigned char cpu_verbosity;
#define ERR(...) fprintf(stderr, __VA_ARGS__)
#define INFO(...) if(cpu_verbosity > 0) printf(__VA_ARGS__)
#define LOG(...) if(cpu_verbosity > 1) printf(__VA_ARGS__)
#define VERBOSE(...) if(cpu_verbosity > 2) printf(__VA_ARGS__)

SDL_Renderer *renderer;

byte memory[4096];   // 4K memory
chip8registries cpu;
Uint8 *keyboard_state;

// Contains screen data.
bool screen_buffer[64 * 32];

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

int initialize_emulator(unsigned char verbose_level) {
    cpu_verbosity = verbose_level;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        ERR("SDL_Init error: %s", SDL_GetError());
        return 1;
    }

    // Open window
    SDL_Window *win = SDL_CreateWindow("Chip8 Emulator - pacman", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                       64 * PIXEL_SIZE, 32 * PIXEL_SIZE, SDL_WINDOW_SHOWN);
    if (win == NULL) {
        ERR("SDL_CreateWindow error: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if(renderer == NULL) {
        SDL_DestroyWindow(win);
        ERR("SDL_CreateRenderer error: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderDrawColor(renderer, BG_R, BG_G, BG_B, 255);
    SDL_RenderClear(renderer);

    LOG("SDL Initialized.\n");
    return 0;
}

int load_rom(const char *file) {
    // load rom
    INFO("Loading rom %s...", file);

    const unsigned max_size = 0xE00;

    FILE *f = fopen(file, "rb");    // read in binary mode
    if (f == NULL) {
        ERR("\nCouldn't open %s (%s)", file, strerror(errno));
        return 1;
    }

    // read file size size
    fseek(f, 0, SEEK_END);
    unsigned size = ftell(f);
    rewind(f);
    INFO(" [%u bytes]\n", size);

    if (size > max_size) {
        ERR("File size exceeds %u bytes. Can't load into memory.\n", max_size);
        return 1;
    }

    fread(memory+PROGRAM_START_OFFSET, 1, max_size, f);
    fclose(f);
    return 0;
}

void run() {
    while(cpu.running) {
        handleNativeEvents();
        cycle();
    }
}

void cycle() {
    for(unsigned i = 0; i < STEPS_PER_CYCLE; ++i) {
        if(!cpu.running) return;

        SDL_PumpEvents();
        keyboard_state = (char *)SDL_GetKeyboardState(NULL);
        cpu_process();
        SDL_Delay(2);

        if(cpu.need_repaint) render_buffer();
    }

    if(cpu.dt > 0) cpu.dt--;
    if(cpu.st > 0) {
        cpu.st--;
        // TODO: play beep.
    }
}

void print_memory() {
    printf("0x000: ");
    for (unsigned i = 0; i < 4096; ++i) {
        if(i % 16 == 0 && i) printf("\n0x%03x: ", i);
        printf("%02x ", memory[i]);
    }
    printf("\n");
}

void render_buffer() {
    SDL_SetRenderDrawColor(renderer, BG_R, BG_G, BG_B, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, FG_R, FG_G, FG_B, 255);

    for(unsigned i = 0; i < 32*64; ++i) {
        if(!screen_buffer[i]) continue;

        int x = i % 64;
        int y = (i / 64);

        SDL_Rect rect = {
                PIXEL_SIZE * x,
                PIXEL_SIZE * y,
                PIXEL_SIZE,
                PIXEL_SIZE
        };
        SDL_RenderFillRect(renderer, &rect);
    }
    SDL_RenderPresent(renderer);
    cpu.need_repaint = false;
}

bool isKeyPressed(byte k) {
    switch (k) {
        case 0x1: return keyboard_state[SDL_SCANCODE_1] > 0;
        case 0x2: return keyboard_state[SDL_SCANCODE_2] > 0;
        case 0x3: return keyboard_state[SDL_SCANCODE_3] > 0;
        case 0xC: return keyboard_state[SDL_SCANCODE_4] > 0;
        case 0x4: return keyboard_state[SDL_SCANCODE_Q] > 0;
        case 0x5: return keyboard_state[SDL_SCANCODE_W] > 0;
        case 0x6: return keyboard_state[SDL_SCANCODE_E] > 0;
        case 0xD: return keyboard_state[SDL_SCANCODE_R] > 0;
        case 0x7: return keyboard_state[SDL_SCANCODE_A] > 0;
        case 0x8: return keyboard_state[SDL_SCANCODE_S] > 0;
        case 0x9: return keyboard_state[SDL_SCANCODE_D] > 0;
        case 0xE: return keyboard_state[SDL_SCANCODE_F] > 0;
        case 0xA: return keyboard_state[SDL_SCANCODE_Z] > 0;
        case 0x0: return keyboard_state[SDL_SCANCODE_X] > 0;
        case 0xB: return keyboard_state[SDL_SCANCODE_C] > 0;
        case 0xF: return keyboard_state[SDL_SCANCODE_V] > 0;
    }
}

void handleNativeEvents() {
    // handle sdl_events
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        switch(event.type) {
            case SDL_KEYDOWN:
                if(event.key.keysym.sym == SDLK_ESCAPE) cpu.running = false;
                break;
            case SDL_QUIT:
                cpu.running = false;
                break;
        }
    }
}

byte getNextKeypress() {
    for(byte i = 0; i < 16; ++i)
        if(isKeyPressed(i)) return i;
    return 255;
}

void clear_display() {
    VERBOSE("CLS\n");
    memset(screen_buffer, 0, sizeof(screen_buffer));
    cpu.pc.WORD += 2;

    cpu.need_repaint = true;
}

void draw(byte x, byte y, byte nib) {
    VERBOSE("DRW V%x, V%x, 0x%01x\n", x, y, nib);

    // set collision flag
    cpu.v[0xF] = 0;

    // blit sprite at I reg to Vx, Vy
    for(unsigned h = 0; h < nib; ++h) {
        unsigned rowOffset = (cpu.v[y] + h) * 64 + cpu.v[x];

        // read a row of the sprite (always 1 byte)
        byte sprite = memory[cpu.i.WORD + h];
        for(unsigned v = 0; v < 8; ++v) {
            // reads the v bit from sprite byte
            byte offsetMask = 0x80 >> v;
            bool spritePixel = (sprite & offsetMask);

            if(spritePixel && screen_buffer[rowOffset + v])
                cpu.v[0xF] = 1;

            // XOR the new pixel value with the old
            screen_buffer[rowOffset + v] ^= spritePixel;
        }
    }
    cpu.need_repaint = true;
    cpu.pc.WORD += 2;
}

void skip_if_key(byte reg) {
    VERBOSE("SKP V%x\n", reg);
    if (isKeyPressed(cpu.v[reg])) cpu.pc.WORD += 2;
    cpu.pc.WORD += 2;
}

void skip_if_not_key(byte reg) {
    VERBOSE("SKNP V%x\n", reg);
    if (!isKeyPressed(cpu.v[reg])) cpu.pc.WORD += 2;
    cpu.pc.WORD += 2;
}

void load_key(byte reg) {
    VERBOSE("LD V%x, K\n", reg);

    // halt until any key pressed
    byte pressed = getNextKeypress();
    if (pressed != 255) cpu.pc.WORD += 2;
}