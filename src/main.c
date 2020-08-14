#include <stdio.h>

#include "emulator.h"
#include "cpu.h"

#include <getopt.h>

int main(const int argc, const char *argv[]) {
    initialize_emulator();

    int status = load_rom(argv[1]);
    if (status > 0) return 1;

    initialize_cpu();

    run();

    return 0;
}
