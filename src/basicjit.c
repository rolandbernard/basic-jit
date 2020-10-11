
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "common/stackalloc.h"
#include "codegen/instructions.h"
#include "common/executil.h"

int main(int argc, char** argv) {
    StackAllocator jit_memory = STACK_ALLOCATOR_INITIALIZER;

    addMovImm32ToReg(&jit_memory, REG_8, 42);
    addMovImm32ToReg(&jit_memory, REG_A, 12);
    addMovRegToReg(&jit_memory, REG_14, REG_8);
    addMovRegToReg(&jit_memory, REG_A, REG_14);
    addRetN(&jit_memory);

    printMemoryContent(stderr, jit_memory.memory, jit_memory.occupied);
    int ret = 0;
    if(executeFunctionInMemory(jit_memory.memory, jit_memory.occupied, &ret)) {
        perror("Failed to execute");
    } else {
        fprintf(stderr, "Return: %i\n", ret);
    }
    
    free_stack(&jit_memory);
    return 0;
}