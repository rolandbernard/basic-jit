#ifndef _STACK_ALLOC_H_
#define _STACK_ALLOC_H_

#include <stdlib.h>

void* alloc_on_stack(size_t size);

void free_stack_till(void* addr);

void free_complete_stack();

#endif