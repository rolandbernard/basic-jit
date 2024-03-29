
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>

#include "exec/executil.h"

typedef int (*JitFunction)();

static uint64_t hash(uint64_t x) {
    x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
    x = x ^ (x >> 31);
    return x;
}

void executeFunctionInMemory(void* mem, size_t len, int* ret) {
    int pid = fork();
    if(pid == -1) {
        *ret = EXIT_FORK_ERROR;
    } else if(pid == 0) {
        struct timeval time;
        gettimeofday(&time, NULL);
        srand(hash(time.tv_sec) ^ hash(time.tv_usec));
        JitFunction entry = (JitFunction)mem;
        size_t pagesize = sysconf(_SC_PAGESIZE);
        uintptr_t start = (uintptr_t)mem;
        uintptr_t end = start + len;
        uintptr_t pagestart = start & -pagesize;
        if (mprotect((void*)pagestart, end - pagestart, PROT_EXEC | PROT_READ | PROT_WRITE)) {
            exit(EXIT_MEM_ERROR);
        } else {
            signal(SIGINT, SIG_DFL);
            entry();
            exit(EXIT_NORMAL);
        }
    } else {
        waitpid(pid, ret, 0);
        if (WIFEXITED(*ret)) {
            *ret = WEXITSTATUS(*ret);
        } else if (WIFSIGNALED(*ret)) {
            *ret = EXIT_SIGNAL_ERROR_START + WTERMSIG(*ret);
        }
    }
}

void printMemoryContent(FILE* file, void* mem, size_t len) {
    fprintf(file, "memory at %p:\n", mem);
    for (size_t i = 0; i < len; i++) {
        if (i % 16 == 0) {
            fprintf(file, "  ");
        }
        fprintf(file, "%02hhx ", ((uint8_t*)mem)[i]);
        if (i % 16 == 15) {
            fprintf(file, "\n");
        }
    }
    if (len % 16 != 0) {
        fprintf(file, "\n");
    }
}

