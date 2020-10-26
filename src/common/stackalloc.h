#ifndef _INSTMEM_H_
#define _INSTMEM_H_

#include <stdint.h>
#include <stdlib.h>

typedef struct StackAllocator_s {
    void* memory;
    size_t capacity;
    size_t occupied;
    struct StackAllocator_s* next;
} StackAllocator;

#define STACK_ALLOCATOR_INITIALIZER { .memory = NULL, .capacity = 0, .occupied = 0, .next = NULL}

void* allocAligned(StackAllocator* mem, size_t size);

void* allocUnaligned(StackAllocator* mem, size_t size);

void freeStack(StackAllocator* mem);

void resetStack(StackAllocator* mem);

#endif