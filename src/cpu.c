//
// Created by olle on 2020-08-10.
//

#include "cpu.h"

#include <stdlib.h>
#include <string.h>

void initialize_cpu() {
    cpu.pc.WORD = PROGRAM_START_OFFSET;

    // copy fontset into memory
    memcpy(memory+FONTSET_START_OFFSET, fontset, sizeof(fontset));

    cpu.running = true;
}

void cpu_process() {
    word opcode;

    // opcodes are stored in ram as little-endian
    // 4f 13 -> JMP 34f
    opcode.BYTE.high = memory[cpu.pc.WORD];
    opcode.BYTE.low = memory[cpu.pc.WORD+1];
    execute_opcode(opcode);
}


void execute_opcode(word code) {
    word nnn = {code.WORD & 0xFFF};
    byte kk = code.BYTE.low;

    byte x = (code.WORD & 0x0F00) >> 8;
    byte y  = (code.WORD & 0x00F0) >> 4;
    byte n = (code.WORD & 0x000F);

    // printf("%04x : $%04x   ", cpu.pc.WORD, code.WORD);

    if (code.WORD == 0x00E0) return clear_display();
    else if(code.WORD == 0x00EE) return return_from_subroutine();

    switch (code.WORD >> 12) {
        case 0x0: return sys_jmp(nnn);
        case 0x1: return jump(nnn);
        case 0x2: return call_subroutine(nnn);
        case 0x3: return skip_if_equal(x, kk);
        case 0x4: return skip_if_not_equal(x, kk);
        case 0x5:
            if (n == 0) return skip_if_equal_reg(x, y);
        case 0x6: return load(x, kk);
        case 0x7: return add(x, kk);
        case 0x8:
            switch (n) {
                case 0x0: return load_reg(x, y);
                case 0x1: return or_reg(x, y);
                case 0x2: return and_reg(x, y);
                case 0x3: return xor_reg(x, y);
                case 0x4: return add_reg(x, y);
                case 0x5: return sub_reg(x, y);
                case 0x6: return shr_reg(x, y);
                case 0x7: return subn_reg(x, y);
                case 0xE: return shl_reg(x, y);
            };
        case 0x9:
            if (n == 0) return skip_if_not_equal_reg(x, y);
        case 0xA: return load_i(nnn);
        case 0xB: return jump_offset(nnn);
        case 0xC: return rnd_reg(x, kk);
        case 0xD: return draw(x, y, n);
        case 0xE:
            if (kk == 0x9E) return skip_if_key(x);
            if (kk == 0xA1) return skip_if_not_key(x);
        case 0xF:
            switch (kk) {
                case 0x07: return load_delay_get(x);
                case 0x0A: return load_key(x);
                case 0x15: return load_delay_set(x);
                case 0x18: return load_sound_set(x);
                case 0x1E: return add_i(x);
                case 0x29: return load_sprite(x);
                case 0x33: return load_bcd(x);
                case 0x55: return copy_reg(x);
                case 0x65: return read_reg(x);
            };
    };
    printf("%04x : $%04x   ", cpu.pc.WORD, code.WORD);
    printf("NULL\nUnregonized opcode: %04x\n", code.WORD);
    cpu.running = false;
}

// Instruction set

void sys_jmp(word addr) {
    // printf("SYS 0x%03x\n", addr.WORD);
    cpu.pc.WORD += 2;
    cpu.running = false;
}

void return_from_subroutine() {
    // printf("RET\n");
    // jump back in stack, and move one instr. forward
    cpu.pc.WORD = cpu.stack[cpu.sp.WORD].WORD + 2;
    cpu.sp.WORD--;
}

void jump(word addr) {
    // printf("JMP 0x%03x\n", addr.WORD);
    cpu.pc = addr;
}

void call_subroutine(word addr) {
    // printf("CALL 0x%03x\n", addr.WORD);
    cpu.sp.WORD++;
    cpu.stack[cpu.sp.WORD] = cpu.pc;
    cpu.pc = addr;
}

void skip_if_equal(byte reg, byte val) {
    // printf("SE V%x, 0x%02x\n", reg, val);
    if(cpu.v[reg] == val) cpu.pc.WORD += 2;

    cpu.pc.WORD += 2;
}

void skip_if_not_equal(byte reg, byte val) {
    // printf("SNE V%x, 0x%02x\n", reg, val);
    if(cpu.v[reg] != val) cpu.pc.WORD += 2;

    cpu.pc.WORD += 2;
}

void skip_if_equal_reg(byte reg1, byte reg2) {
    // printf("SE V%x, V%x\n", reg1, reg2);
    if(cpu.v[reg1] == cpu.v[reg2]) cpu.pc.WORD += 2;

    cpu.pc.WORD += 2;
}

void load(byte reg, byte val) {
    // printf("LD V%x, 0x%02x\n", reg, val);
    cpu.v[reg] = val;

    cpu.pc.WORD += 2;
}

void add(byte reg, byte val) {
    // printf("ADD V%x, 0x%02x\n", reg, val);
    cpu.v[reg] += val;

    cpu.pc.WORD += 2;
}

void load_reg(byte reg1, byte reg2) {
    // printf("LD V%x, V%x\n", reg1, reg2);
    cpu.v[reg1] = cpu.v[reg2];

    cpu.pc.WORD += 2;
}

void or_reg(byte reg1, byte reg2) {
    // printf("OR V%x, V%x\n", reg1, reg2);
    cpu.v[reg1] = cpu.v[reg1] | cpu.v[reg2];

    cpu.pc.WORD += 2;
}

void and_reg(byte reg1, byte reg2) {
    // printf("AND V%x, V%x\n", reg1, reg2);
    cpu.v[reg1] = cpu.v[reg1] & cpu.v[reg2];

    cpu.pc.WORD += 2;
}

void xor_reg(byte reg1, byte reg2) {
    // printf("XOR V%x, V%x\n", reg1, reg2);
    cpu.v[reg1] = cpu.v[reg1] ^ cpu.v[reg2];

    cpu.pc.WORD += 2;
}

void add_reg(byte reg1, byte reg2) {
    // printf("ADD V%x, V%x\n", reg1, reg2);
    int sum = cpu.v[reg1] + cpu.v[reg2];
    if (sum > 255) cpu.v[0xF] = 1;
    else cpu.v[0xF] = 0;
    cpu.v[reg1] = sum & 0xFFFF;

    cpu.pc.WORD += 2;
}

void sub_reg(byte reg1, byte reg2) {
    // printf("SUB V%x, V%x\n", reg1, reg2);
    if (cpu.v[reg1] > cpu.v[reg2]) cpu.v[0xF] = 1;
    else cpu.v[0xF] = 0;
    cpu.v[reg1] -= cpu.v[reg2];

    cpu.pc.WORD += 2;
}

void shr_reg(byte reg1, byte reg2) {
    // printf("SHR V%x, V%x\n", reg1, reg2);
    cpu.v[0xF] = cpu.v[reg2] & 0x01;
    cpu.v[reg1] = cpu.v[reg2] >> 1;

    cpu.pc.WORD += 2;
}

void subn_reg(byte reg1, byte reg2) {
    // printf("SUBN V%x, V%x\n", reg1, reg2);
    cpu.v[0xF] = (cpu.v[reg2] > cpu.v[reg1]) ? 1 : 0;
    cpu.v[reg1] = cpu.v[reg2] - cpu.v[reg1];

    cpu.pc.WORD += 2;
}

void shl_reg(byte reg1, byte reg2) {
    // printf("SHL V%x, V%x\n", reg1, reg2);
    cpu.v[0xF] = cpu.v[reg2] & 0x01;
    cpu.v[reg1] = cpu.v[reg2] << 1;

    cpu.pc.WORD += 2;
}

void skip_if_not_equal_reg(byte reg1, byte reg2) {
    // printf("SNE V%x, V%x\n", reg1, reg2);
    if(cpu.v[reg1] != cpu.v[reg2]) cpu.pc.WORD += 2;

    cpu.pc.WORD += 2;
}

void load_i(word addr) {
    // printf("LD I, 0x%03x\n", addr);
    cpu.i = addr;
    cpu.pc.WORD += 2;
}

void jump_offset(word addr) {
    // printf("JMP V0, 0x%03x\n", addr);
    cpu.pc.WORD = cpu.v[0x0] + addr.WORD;
}

void rnd_reg(byte reg, byte val) {
    // printf("RNG V%x, 0x%02x\n", reg, val);
    int r = rand() % 256;
    cpu.v[reg] = r & val;
    cpu.pc.WORD += 2;
}

void load_delay_get(byte reg) {
    // printf("LD V%x, DT\n", reg);
    cpu.v[reg] = cpu.dt;
    cpu.pc.WORD += 2;
}

void load_delay_set(byte reg) {
    // printf("LD DT, V%x\n", reg);
    cpu.dt = cpu.v[reg];
    cpu.pc.WORD += 2;
}

void load_sound_set(byte reg) {
    // printf("LD ST, V%x\n", reg);
    cpu.st = cpu.v[reg];
    cpu.pc.WORD += 2;
}

void add_i(byte reg) {
    // printf("ADD I, V%x\n", reg);
    cpu.i.WORD = cpu.i.WORD + cpu.v[reg];
    cpu.pc.WORD += 2;
}

void load_sprite(byte reg) {
    // printf("LD F, V%x\n", reg);
    cpu.i.WORD = cpu.v[reg] * 5;
    cpu.pc.WORD += 2;
}

void load_bcd(byte reg) {
    // printf("LD B, V%x\n", reg);
    memory[cpu.i.WORD] = cpu.v[reg] / 100;
    memory[cpu.i.WORD+1] = (cpu.v[reg] / 10) % 10;
    memory[cpu.i.WORD+2] = cpu.v[reg] % 10;
    cpu.pc.WORD += 2;
}

void copy_reg(byte reg) {
    // printf("LD [I], V%x\n", reg);
    for(unsigned x = 0; x < reg; ++x)
        memory[cpu.i.WORD + x] = cpu.v[x];
    cpu.i.WORD = cpu.i.WORD + reg + 1;
    cpu.pc.WORD += 2;
}

void read_reg(byte reg) {
    // printf("LD V%x, [I]\n", reg);
    for(unsigned x = 0; x < reg; ++x)
        cpu.v[x] = memory[cpu.i.WORD+x];
    cpu.i.WORD = cpu.i.WORD + reg + 1;
    cpu.pc.WORD += 2;
}