//
// Created by olle on 2020-08-10.
//

#ifndef CHIP8_CPU_H
#define CHIP8_CPU_H

#include <stdbool.h>

typedef unsigned char byte;
typedef union {
    unsigned short int WORD;
    struct {
        byte low, high;
    } BYTE;
} word;

typedef struct {
    byte v[0x10];   // emulated cpu registers
    word i;         // index register
    word sp;        // stack pointer
    word pc;        // program counter
    byte st, dt;    // sound and delay timer
    word operand;   // the current operand
    word stack[16]; // stack allows 16 levels of nested subroutines.
    bool need_repaint; // True if screen needs repainting (updating)
    bool running;   // is the cpu running
} chip8registries;

extern byte memory[4096];   // 4K memory
extern chip8registries cpu;

static const unsigned PROGRAM_START_OFFSET = 0x200;
static const unsigned FONTSET_START_OFFSET = 0x000;
static const unsigned STEPS_PER_CYCLE = 10;

static const byte fontset[80] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void initialize_cpu();

void cpu_process();
void execute_opcode(word code);

bool toggle_pixel();

// instruction set -----------------------------------------------------------------------------------------------------
void sys_jmp(word addr);                        // 0nnn JMP
                                                // jump to a machine routine at nnn.
                                                // Used on old computers, not implemented!
void clear_display();                           // 00E0 CLS
void return_from_subroutine();                  // 00EE RET
void jump(word addr);                           // 1nnn JMP
void call_subroutine(word addr);                // 2nnn CALL
void skip_if_equal(byte reg, byte val);         // 3xkk SE
void skip_if_not_equal(byte reg, byte val);     // 4xkk SNE
void skip_if_equal_reg(byte reg1, byte reg2);   // 5xy0 SE
void load(byte reg, byte val);                  // 6xkk LD
void add(byte reg, byte val);                   // 7xkk ADD

void load_reg(byte reg1, byte reg2);            // 8xy0 LD
void or_reg(byte reg1, byte reg2);              // 8xy1 OR
void and_reg(byte reg1, byte reg2);             // 8xy2 AND
void xor_reg(byte reg1, byte reg2);             // 8xy3 XOR
void add_reg(byte reg1, byte reg2);             // 8xy4 ADD
void sub_reg(byte reg1, byte reg2);             // 8xy5 SUB
void shr_reg(byte reg1, byte reg2);             // 8xy6 SHR
void subn_reg(byte reg1, byte reg2);            // 8xy7 SUBN
void shl_reg(byte reg1, byte reg2);             // 8xyE SHL

void skip_if_not_equal_reg(byte reg1, byte reg2);   // 9xy0 SNE
void load_i(word addr);                         // Annn LD
void jump_offset(word addr);                    // Bnnn JP
void rnd_reg(byte reg, byte val);               // Cxkk RND
void draw(byte x, byte y, byte nib);            // Dxyn DRW
void skip_if_key(byte reg);                     // Ex9E SKP
void skip_if_not_key(byte reg);                 // ExA1 SKNP
void load_delay_get(byte reg);                  // Fx07 LD
void load_key(byte reg);                        // Fx0A LD
void load_delay_set(byte reg);                  // Fx15 LD
void load_sound_set(byte reg);                  // Fx18 LD
void add_i(byte reg);                           // Fx1E ADD I, Vx
void load_sprite(byte reg);                     // Fx29 LD F, Vx
void load_bcd(byte reg);                        // Fx33 LD B, Vx
void copy_reg(byte reg);                        // Fx55 LD [I], Vx
void read_reg(byte reg);                        // Fx65 LD Vx, [I]

#endif //CHIP8_CPU_H
