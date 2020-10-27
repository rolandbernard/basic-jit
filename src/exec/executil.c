
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#include "exec/executil.h"

typedef int (*JitFunction)();

bool executeFunctionInMemory(void* mem, size_t len, int* ret) {
    int pid = fork();
    if(pid == -1) {
        return true;
    } else if(pid == 0) {
        JitFunction entry = (JitFunction)mem;
        size_t pagesize = sysconf(_SC_PAGESIZE);
        uintptr_t start = (uintptr_t)mem;
        uintptr_t end = start + len;
        uintptr_t pagestart = start & -pagesize;
        if (mprotect((void*)pagestart, end - pagestart, PROT_EXEC | PROT_READ | PROT_WRITE)) {
            exit(123);
        } else {
            signal(SIGINT, SIG_DFL);
            exit(entry());
        }
    } else {
        for(int i = 0; i < rand() % 100; i++);
        waitpid(pid, ret, 0);
        *ret = WEXITSTATUS(*ret);
        return *ret == 123;
    }
}

void printMemoryContent(FILE* file, void* mem, size_t len) {
    fprintf(file, "memory at %p:\n", mem);
    for (int i = 0; i < len; i++) {
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
