
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "common/stackalloc.h"
#include "codegen/instructions.h"
#include "common/executil.h"

int main(int argc, char** argv) {
    StackAllocator data_memory = STACK_ALLOCATOR_INITIALIZER;
    StackAllocator jit_memory = STACK_ALLOCATOR_INITIALIZER;
    long* test = (long*)alloc_aligned(&data_memory, sizeof(long));
    *test = 42;

    addMovImm64ToReg(&jit_memory, REG_8, (uint64_t)test);
    addMovImm32ToReg(&jit_memory, REG_A, 12);
    addMovMemRegToReg(&jit_memory, REG_A, REG_8);
    addRetN(&jit_memory);

    printMemoryContent(stderr, jit_memory.memory, jit_memory.occupied);
    int ret = 0;
    if(executeFunctionInMemory(jit_memory.memory, jit_memory.occupied, &ret)) {
        perror("Failed to execute");
    } else {
        fprintf(stderr, "Return: %i\n", ret);
    }
    
    free_stack(&jit_memory);
    free_stack(&data_memory);
    return 0;
}