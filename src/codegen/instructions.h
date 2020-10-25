#ifndef _INSTRUCTIONS_H_
#define _INSTRUCTIONS_H_

#include <stdint.h>
#include <stdbool.h>

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

uint64_t getUsedRegister(RegisterSet regs);

uint64_t getUsedFRegister(RegisterSet regs);

int countFreeRegister(RegisterSet regs);

int countFreeFRegister(RegisterSet regs);

uint64_t getFirstRegister();

uint64_t getFirstFRegister();

void addInstMovRegToReg(StackAllocator* mem, RegisterSet regs, Register dest, Register src);

size_t addInstMovImmToReg(StackAllocator* mem, RegisterSet regs, Register reg, int64_t value, bool force64bit);

void addInstMovMemToReg(StackAllocator* mem, RegisterSet regs, Register reg, void* addr);

size_t addInstMovRegToMem(StackAllocator* mem, RegisterSet regs, Register reg, void* addr);

void addInstMovMemRegToReg(StackAllocator* mem, RegisterSet regs, Register reg, Register addr);

void addInstMovRegToMemReg(StackAllocator* mem, RegisterSet regs, Register addr, Register reg);

void addInstJmp(StackAllocator* mem, RegisterSet regs, void* to);

size_t addInstJmpRel(StackAllocator* mem, RegisterSet regs, int32_t to);

void addInstPush(StackAllocator* mem, RegisterSet regs, Register reg);

void addInstPop(StackAllocator* mem, RegisterSet regs, Register reg);

void addInstPushAll(StackAllocator* mem, RegisterSet regs);

void addInstPopAll(StackAllocator* mem, RegisterSet regs);

void addInstCall(StackAllocator* mem, RegisterSet regs, void* func);

size_t addInstCallRel(StackAllocator* mem, RegisterSet regs, int32_t func);

void addInstReturn(StackAllocator* mem, RegisterSet regs);

void addInstAdd(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b);

void addInstSub(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b);

void addInstMul(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b);

void addInstDiv(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b);

void addInstRem(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b);

size_t addInstCondJmpRel(StackAllocator* mem, RegisterSet regs, JmpCondistions cond, Register a, Register b, int32_t to);

void addInstFAdd(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b);

void addInstFSub(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b);

void addInstFMul(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b);

void addInstFDiv(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b);

void addInstFFrac(StackAllocator* mem, RegisterSet regs, Register dest, Register src);

void addInstFTrunc(StackAllocator* mem, RegisterSet regs, Register dest, Register src);

void addInstMovImmToFReg(StackAllocator* mem, RegisterSet regs, Register reg, double value);

void addInstMovMemToFReg(StackAllocator* mem, RegisterSet regs, Register reg, void* addr);

void addInstMovFRegToMem(StackAllocator* mem, RegisterSet regs, Register reg, void* addr);

void addInstMovFRegToFReg(StackAllocator* mem, RegisterSet regs, Register dest, Register src);

void addInstMovRegToFReg(StackAllocator* mem, RegisterSet regs, Register dest, Register src);

void addInstMovFRegToReg(StackAllocator* mem, RegisterSet regs, Register dest, Register src);

void addInstMovMemRegToFReg(StackAllocator* mem, RegisterSet regs, Register reg, Register addr);

void addInstMovFRegToMemReg(StackAllocator* mem, RegisterSet regs, Register addr, Register reg);

void addInstFunctionCallUnary(StackAllocator* mem, RegisterSet regs, Register ret, Register a, void* func);

void addInstFunctionCallBinary(StackAllocator* mem, RegisterSet regs, Register ret, Register a, Register b, void* func);

void addInstFunctionCallUnaryNoRet(StackAllocator* mem, RegisterSet regs, Register a, void* func);

void addInstFunctionCallRetOnly(StackAllocator* mem, RegisterSet regs, Register ret, void* func);

void addInstFunctionCallSimple(StackAllocator* mem, RegisterSet regs, void* func);

void addInstPushCallerRegs(StackAllocator* mem, RegisterSet regs);

void addInstPopCallerRegs(StackAllocator* mem, RegisterSet regs);

#ifdef __x86_64__

#define pop(R) \
    asm ( \
        "pop %0" \
        : "=r" (R) \
    );

#define push(V) \
    asm ( \
        "push %0" \
        : \
        : "r" (V) \
    );

#define popF(R) { \
    union { \
        int64_t i; \
        double d; \
    } double_to_uint64; \
    pop(double_to_uint64.i); \
    R = double_to_uint64.d; \
}

#define pushF(V) { \
    union { \
        int64_t i; \
        double d; \
    } double_to_uint64; \
    double_to_uint64.d = V; \
    push(double_to_uint64.i); \
}

#endif

#endif