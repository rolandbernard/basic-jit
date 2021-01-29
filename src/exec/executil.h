#ifndef _EXECUTIL_H_
#define _EXECUTIL_H_

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#define EXIT_NORMAL 200
#define EXIT_FORK_ERROR 201
#define EXIT_MEM_ERROR 202
#define EXIT_SEGV_ERROR 203

void executeFunctionInMemory(void* mem, size_t len, int* ret);

void printMemoryContent(FILE* file, void* mem, size_t len);

#endif