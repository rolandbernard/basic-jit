#ifndef _INSTRUCTIONS_H_
#define _INSTRUCTIONS_H_

#include <stdint.h>
#include "common/stackalloc.h"

#define REG_COUNT 12

typedef enum {
    REG_A = 1 << 0,
    REG_B = 1 << 1,
    REG_C = 1 << 2,
    REG_D = 1 << 3,

    REG_8 = 1 << 4,
    REG_9 = 1 << 5,
    REG_10 = 1 << 6,
    REG_11 = 1 << 7,
    REG_12 = 1 << 8,
    REG_13 = 1 << 9,
    REG_14 = 1 << 10,
    REG_15 = 1 << 11,
} Register;

void addJmpRelative32(StackAllocator* mem, int32_t value);

void addJmpAbsoluteReg(StackAllocator* mem, Register reg);

void addMovImm32ToReg(StackAllocator* mem, Register reg, int32_t value);

void addMovImm64ToReg(StackAllocator* mem, Register reg, int64_t value);

void addMovRegToReg(StackAllocator* mem, Register dest, Register src);

void addMovMemRegToReg(StackAllocator* mem, Register dest, Register src_pos);

void addRetN(StackAllocator* mem);

void addPush(StackAllocator* mem, Register reg);

void addPop(StackAllocator* mem, Register reg);

#endif