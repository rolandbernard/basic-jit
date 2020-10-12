#ifndef _INSTRUCTIONS_H_
#define _INSTRUCTIONS_H_

#include <stdint.h>

#include "common/stackalloc.h"

typedef enum {
    COND_EQ,
    COND_NE,
    COND_LT,
    COND_GT,
    COND_LE,
    COND_GE
} JmpCondistions;

typedef uint64_t RegisterSet;
typedef uint64_t Register;

uint64_t getFreeRegister(RegisterSet regs);

uint64_t getFreeFRegister(RegisterSet regs);

void addInstMovRegToReg(StackAllocator* mem, RegisterSet regs, Register dest, Register src);

void addInstMovImmToReg(StackAllocator* mem, RegisterSet regs, Register reg, int64_t value);

void addInstMovMemToReg(StackAllocator* mem, RegisterSet regs, Register reg, void* addr);

void addInstMovRegToMem(StackAllocator* mem, RegisterSet regs, Register reg, void* addr);

void addInstJmp(StackAllocator* mem, RegisterSet regs, Register reg, void* to);

void addInstPush(StackAllocator* mem, RegisterSet regs, Register reg);

void addInstPop(StackAllocator* mem, RegisterSet regs, Register reg);

void addInstPushAll(StackAllocator* mem, RegisterSet regs);

void addInstPopAll(StackAllocator* mem, RegisterSet regs);

void addInstCall(StackAllocator* mem, RegisterSet regs, void* func);

void addInstReturn(StackAllocator* mem, RegisterSet regs);

void addInstAdd(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b);

void addInstSub(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b);

void addInstMul(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b);

void addInstDiv(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b);

void addInstRem(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b);

void addInstCondJmp(StackAllocator* mem, RegisterSet regs, JmpCondistions cond, Register a, Register b, void* to);

void addInstFAdd(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b);

void addInstFSub(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b);

void addInstFMul(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b);

void addInstFDiv(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b);

void addInstFFrac(StackAllocator* mem, RegisterSet regs, Register dest, Register src);

void addInstFTrunc(StackAllocator* mem, RegisterSet regs, Register dest, Register src);

void addInstMovImmToFReg(StackAllocator* mem, RegisterSet regs, Register reg, int64_t value);

void addInstMovMemToFReg(StackAllocator* mem, RegisterSet regs, Register reg, void* addr);

void addInstMovFRegToMem(StackAllocator* mem, RegisterSet regs, Register reg, void* addr);

void addInstMovFRegToFReg(StackAllocator* mem, RegisterSet regs, Register dest, Register src);

int64_t pop();

int64_t push();

#endif