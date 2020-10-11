#ifndef _INSTRUCTIONS_H_
#define _INSTRUCTIONS_H_

#include <stdint.h>
#include "common/stackalloc.h"

typedef enum {
    REG_A = 0,
    REG_C = 1,
    REG_D = 2,
    REG_B = 3,
} Register;

void addJmpRelative32(StackAllocator* mem, int32_t value);

void addMovImm32ToReg(StackAllocator* mem, Register reg, int32_t value);

void addRetN(StackAllocator* mem);

void addPush(StackAllocator* mem, Register reg);

void addPop(StackAllocator* mem, Register reg);

#endif