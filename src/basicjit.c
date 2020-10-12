
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "common/stackalloc.h"
#include "codegen/x86-64.h"
#include "common/executil.h"

int main(int argc, char** argv) {
    StackAllocator jit_memory = STACK_ALLOCATOR_INITIALIZER;
    long test = 42;

    // addMovImm32ToReg(&jit_memory, REG_A, 42);
    addPush(&jit_memory, REG_8);
    addMovImm64ToReg(&jit_memory, REG_8, (uint64_t)&test);
    addMovImm64ToReg(&jit_memory, REG_A, 12);
    addMovRegToMemReg(&jit_memory, REG_8, REG_A);
    addMovMemRegToReg(&jit_memory, REG_8, REG_8);
    addMovRegToReg(&jit_memory, REG_A, REG_8);
    addPop(&jit_memory, REG_8);
    // addAdd(&jit_memory, REG_A, REG_B);
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