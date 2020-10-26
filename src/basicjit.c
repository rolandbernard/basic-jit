
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "exec/fileexec.h"

int main(int argc, char** argv) {
    if(argc == 1) {

    } else if(argc == 2) {
        executeFile(argv[1], true);
    } else {
        fprintf(stderr, "Usage: %s [FILE]\n", argv[0]);
    }
    return 0;
}