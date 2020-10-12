#ifndef _X86_64_H_
#define _X86_64_H_

#include <stdint.h>
#include "common/stackalloc.h"

#define REG_COUNT 12
#define FREG_COUNT 12

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
    
    FREG_1 = 1 << 12,
    FREG_1 = 1 << 12,
    FREG_1 = 1 << 12,
    FREG_1 = 1 << 12,
    FREG_1 = 1 << 12,
    FREG_1 = 1 << 12,
    FREG_1 = 1 << 12,
    FREG_1 = 1 << 12,
} X86Register;

void addJmpRelative32(StackAllocator* mem, int32_t value);

void addJmpAbsoluteReg(StackAllocator* mem, X86Register reg);

void addMovImm32ToReg(StackAllocator* mem, X86Register reg, int32_t value);

void addMovImm64ToReg(StackAllocator* mem, X86Register reg, int64_t value);

void addMovRegToReg(StackAllocator* mem, X86Register dest, X86Register src);

void addMovMemRegToReg(StackAllocator* mem, X86Register dest, X86Register src_pos);

void addMovRegToMemReg(StackAllocator* mem, X86Register dest_pos, X86Register src);

void addRetN(StackAllocator* mem);

void addPush(StackAllocator* mem, X86Register reg);

void addPop(StackAllocator* mem, X86Register reg);

void addAdd(StackAllocator* mem, X86Register dest, X86Register src);

void addSub(StackAllocator* mem, X86Register dest, X86Register src);

void addTest(StackAllocator* mem, X86Register dest, X86Register src);

void addIMul(StackAllocator* mem, X86Register dest, X86Register src);

void addIDiv(StackAllocator* mem, X86Register by);

void addCallReg(StackAllocator* mem, X86Register reg);

void addJmpEQ(StackAllocator* mem, uint32_t rel);

void addJmpNE(StackAllocator* mem, uint32_t rel);

void addJmpGT(StackAllocator* mem, uint32_t rel);

void addJmpLS(StackAllocator* mem, uint32_t rel);

void addJmpGE(StackAllocator* mem, uint32_t rel);

void addJmpLE(StackAllocator* mem, uint32_t rel);

void addMovFRegToReg(StackAllocator* mem, X86Register dest, X86Register fsrc);

void addMovRegToFReg(StackAllocator* mem, X86Register fdest, X86Register src);

void addMovFRegToFReg(StackAllocator* mem, X86Register fdest, X86Register fsrc);

void addMovMemRegToFReg(StackAllocator* mem, X86Register fdest, X86Register src_pos);

void addMovFRegToMemReg(StackAllocator* mem, X86Register dest_pos, X86Register fsrc);

#endif