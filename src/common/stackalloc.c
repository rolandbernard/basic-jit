
#include <stdlib.h>
#include <stdint.h>

#include "common/stackalloc.h"

#define INITIAL_CAPACITY (1 << 20);

void* alloc_unaligned(StackAllocator* mem, size_t size) {
    if(mem->capacity <= mem->occupied + size) {
        if(mem->capacity == 0) {
            mem->capacity = INITIAL_CAPACITY;
        }
        while(mem->capacity <= mem->occupied + size) {
            mem->capacity *= 2;
        }
        mem->memory = realloc(mem->memory, mem->capacity);
    }
    void* ret = mem->memory + mem->occupied;
    mem->occupied += size;
    return (void*)ret;
}

void* alloc_aligned(StackAllocator* mem, size_t size) {
    mem->occupied = (mem->occupied + 15) & ~(size_t)15;
    return alloc_unaligned(mem, size);
}

void free_stack(StackAllocator* mem) {
    mem->occupied = 0;
    mem->capacity = 0;
    free(mem->memory);
}