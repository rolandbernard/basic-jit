
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>

#define INITIAL_MEMORY_SIZE 1024

typedef int (*JitFunction)();

int main(int argc, char** argv) {
    size_t memory_size = INITIAL_MEMORY_SIZE;
    uint8_t* jit_memory = (uint8_t*)malloc(memory_size * sizeof(uint8_t));
    JitFunction entry = (JitFunction)jit_memory;

    jit_memory[0] = 0x90;
    jit_memory[1] = 0x90; 
    jit_memory[2] = 0x90;
    jit_memory[3] = 0x90; 
    jit_memory[4] = 0xB8; 
    jit_memory[5] = 0x2a;
    jit_memory[6] = 0x00; 
    jit_memory[7] = 0x00;
    jit_memory[8] = 0x00; 
    jit_memory[9] = 0xc3; 
    
    size_t pagesize = sysconf(_SC_PAGESIZE);
    uintptr_t start = (uintptr_t)jit_memory;
    uintptr_t end = start + memory_size;
    uintptr_t pagestart = start & -pagesize;
    if(mprotect((void*)pagestart, end - pagestart, PROT_EXEC | PROT_READ | PROT_WRITE)) {
        perror("Failed to set memory protection");
    } else {
        fprintf(stderr, "Return: %i\n", entry());
    }
    
    free(jit_memory);
    return 0;
}
