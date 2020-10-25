
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "common/stackalloc.h"

#define INITIAL_CAPACITY (1 << 20)

// This is used for instruction allocation, where the absolute address doesn't mather (I only use relative jumps)
void* allocUnaligned(StackAllocator* mem, size_t size) {
    if(mem->capacity <= mem->occupied + size) {
        if(mem->capacity == 0) {
            mem->memory = malloc(INITIAL_CAPACITY);
            mem->capacity = INITIAL_CAPACITY;
        } else {
            while(mem->capacity <= mem->occupied + size) {
                mem->capacity *= 2;
            }
            mem->memory = realloc(mem->memory, mem->capacity);
        }
    }
    void* ret = mem->memory + mem->occupied;
    mem->occupied += size;
    return (void*)ret;
}

// This is used for ast allocation, where the absolute address must stay constant
void* allocAligned(StackAllocator* mem, size_t size) {
    if(size > INITIAL_CAPACITY) {
        if(mem->next == NULL) {
            StackAllocator* next = (StackAllocator*)malloc(sizeof(StackAllocator));
            next->capacity = size;
            next->occupied = size;
            next->memory = malloc(size);
            next->next = NULL;
            mem->next = next;
            return next->memory;
        } else {
            return allocAligned(mem->next, size);
        }
    } else {
        if (mem->capacity <= mem->occupied + 2 * size) {
            if (mem->capacity == 0) {
                mem->memory = malloc(INITIAL_CAPACITY);
            } else {
                if (mem->next == NULL) {
                    StackAllocator* next = (StackAllocator*)malloc(sizeof(StackAllocator));
                    next->capacity = 0;
                    next->occupied = 0;
                    next->memory = NULL;
                    next->next = NULL;
                    mem->next = next;
                }
                return allocAligned(mem->next, size);
            }
        }
        mem->occupied = (mem->occupied + 15) & ~(size_t)15;
        void* ret = mem->memory + mem->occupied;
        mem->occupied += size;
        return (void*)ret;
    }
}

void freeStack(StackAllocator* mem) {
    if(mem != NULL) {
        mem->occupied = 0;
        mem->capacity = 0;
        free(mem->memory);
        freeStack(mem->next);
        free(mem->next);
    }
}