
#include <stdlib.h>
#include <stdint.h>

#include "stackalloc.h"

#define INITIAL_CAPACITY (1 << 20);

void* memory = NULL;
size_t capacity = 0;
size_t allocated = 0;

void* alloc_on_stack(size_t size) {
    if(capacity <= allocated + size) {
        if(capacity == 0) {
            capacity = INITIAL_CAPACITY;
        }
        while(capacity <= allocated + size) {
            capacity *= 2;
        }
        memory = realloc(memory, capacity);
    }
    intptr_t ret = (intptr_t)memory + allocated;
    allocated += size;
    ret = (ret + 15) & ~(intptr_t)15;
    return (void*)ret;
}

void free_stack_till(void* addr) {
    allocated = (addr - memory);
    if(allocated < capacity / 64) {
        while(allocated < capacity / 64) {
            capacity /= 2;
        }
        memory = realloc(memory, capacity);
    }
}

void free_complete_stack() {
    allocated = 0;
    capacity = 0;
    free(memory);
}
