#ifndef _INSTMEM_H_
#define _INSTMEM_H_

#include <stdint.h>
#include <stdlib.h>

typedef struct {
    uint8_t* memory;
    size_t capacity;
    size_t occupied;
} StackAllocator;

#define STACK_ALLOCATOR_INITIALIZER { .memory = NULL, .capacity = 0, .occupied = 0}

void* alloc_aligned(StackAllocator* mem, size_t size);

void* alloc_unaligned(StackAllocator* mem, size_t size);

void free_stack(StackAllocator* mem);

#endif