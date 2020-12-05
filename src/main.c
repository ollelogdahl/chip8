#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "emulator.h"
#include "cpu.h"

#define ERR(...) fprintf(stderr, __VA_ARGS__)

int main(const int argc, char **argv) {

    int helpflag = 0;
    unsigned char verbosity = 1;

    // Parse command line options
    int opt;
    extern char *optarg;
    extern int opterr, optind, optopt;
    while ((opt = getopt(argc, argv, "hv:")) != -1) {
        switch (opt) {
        case 'h': // help
            helpflag++;
            break;
        case 'v': // set verbosity
            verbosity = atoi(optarg);
            break;
        case ':': // arg without operand
            ERR("Option -%c requires an operand\n", opt);
            helpflag++;
            break;
        case '?': // unrecognized arg
            ERR("Unrecognized option: '-%c'\n", optopt);
            helpflag++;
        }
    }

    // ensure given romfile
    if(argv[optind] == NULL) {
        ERR("Mandatory argument missing.\n");
        helpflag++;
    }

    // If helpflag
    if(helpflag) {
        const char *helpstr = 
          "usage: %s [options] rom\n"
          "options:\n"
          "  -v [lvl]   Sets the verbosity level (default 1).\n"
          "  -h         Displays help.\n";
        printf(helpstr, argv[0]);
        return 2;
    }

    initialize_emulator(verbosity);

    int status = load_rom(argv[optind]);
    if (status > 0) return 1;

    initialize_cpu(verbosity);

    run();

    return 0;
}
