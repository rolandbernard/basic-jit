
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "instructions.h"

#define INITIAL_MEMORY_SIZE 1024

typedef int (*JitFunction)();

int main(int argc, char** argv) {
    size_t memory_size = INITIAL_MEMORY_SIZE;
    uint8_t* jit_memory = (uint8_t*)malloc(memory_size * sizeof(uint8_t));
    JitFunction entry = (JitFunction)jit_memory;

    uint8_t* insert_location = jit_memory;
    insert_location += addMovImm32ToReg(insert_location, REG_A, 42);
    insert_location += addJmpRelative32(insert_location, 5);
    insert_location += addMovImm32ToReg(insert_location, REG_A, 12);
    insert_location += addRetN(insert_location);
    
    size_t pagesize = sysconf(_SC_PAGESIZE);
    uintptr_t start = (uintptr_t)jit_memory;
    uintptr_t end = start + memory_size;
    uintptr_t pagestart = start & -pagesize;
    if(mprotect((void*)pagestart, end - pagestart, PROT_EXEC | PROT_READ | PROT_WRITE)) {
        perror("Failed to set memory protection");
    } else {
        fprintf(stderr, "JIT memory: %p\n", jit_memory);
        fprintf(stderr, "Return:     %i\n", entry());
    }
    
    free(jit_memory);
    return 0;
}
