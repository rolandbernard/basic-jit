#ifndef _STACK_ALLOC_H_
#define _STACK_ALLOC_H_

#include <stdlib.h>

void* alloc_ast(size_t size);

void free_all_asts();

#endif