#ifndef _EXECUTIL_H_
#define _EXECUTIL_H_

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#define EXIT_ERROR 200
#define EXIT_NORMAL 201

bool executeFunctionInMemory(void* mem, size_t len, int* ret);

void printMemoryContent(FILE* file, void* mem, size_t len);

#endif