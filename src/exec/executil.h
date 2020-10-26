#ifndef _EXECUTIL_H_
#define _EXECUTIL_H_

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

bool executeFunctionInMemory(void* mem, size_t len, int* ret);

void printMemoryContent(FILE* file, void* mem, size_t len);

#endif