
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define INITIAL_MEMORY_SIZE 1024

typedef int (*JitFunction)();

static uint8_t test_program[] = {
    0xB8, 0x2a, 0x00, 0x00, 0x00, // mov aex, 42
    0xe9, 0x05, 0x00, 0x00, 0x00, // jr 5
    0xB8, 0x0c, 0x00, 0x00, 0x00, // mov aex, 12
    0xc3, // retn
};

int main(int argc, char** argv) {
    size_t memory_size = INITIAL_MEMORY_SIZE;
    uint8_t* jit_memory = (uint8_t*)malloc(memory_size * sizeof(uint8_t));
    JitFunction entry = (JitFunction)jit_memory;

    memcpy(jit_memory, test_program, sizeof(test_program));
    
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
