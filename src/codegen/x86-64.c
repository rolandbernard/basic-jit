
#include "codegen/x86-64.h"
    
static int reg_to_opcodeno[] = {
    [REG_A] = 0,
    [REG_B] = 3,
    [REG_C] = 1,
    [REG_D] = 2,
    
    [REG_8] = 0,
    [REG_9] = 1,
    [REG_10] = 2,
    [REG_11] = 3,
    [REG_12] = 4,
    [REG_13] = 5,
    [REG_14] = 6,
    [REG_15] = 7,
};

void addJmpRelative32(StackAllocator* mem, int32_t value) {
    uint8_t* ptr = (uint8_t*)alloc_unaligned(mem, 5);
    ptr[0] = 0xe9;
    ptr[1] = (value) & 0xff;
    ptr[2] = (value >> 8) & 0xff;
    ptr[3] = (value >> 16) & 0xff;
    ptr[4] = (value >> 24) & 0xff;
}

void addJmpAbsoluteReg(StackAllocator* mem, X86Register reg) {
    if(reg >= REG_8) {
        uint8_t* ptr = (uint8_t*)alloc_unaligned(mem, 1);
        ptr[0] = 0x41;
    }
    uint8_t* ptr = (uint8_t*)alloc_unaligned(mem, 2);
    ptr[0] = 0xff;
    ptr[1] = 0xE0 + reg_to_opcodeno[reg];
}

void addMovImm32ToReg(StackAllocator* mem, X86Register reg, int32_t value) {
    if(reg >= REG_8) {
        uint8_t* ptr = (uint8_t*)alloc_unaligned(mem, 1);
        ptr[0] = 0x41;
    }
    uint8_t* ptr = (uint8_t*)alloc_unaligned(mem, 5);
    ptr[0] = 0xb8 + reg_to_opcodeno[reg];
    ptr[1] = (value) & 0xff;
    ptr[2] = (value >> 8) & 0xff;
    ptr[3] = (value >> 16) & 0xff;
    ptr[4] = (value >> 24) & 0xff;
}

void addMovImm64ToReg(StackAllocator* mem, X86Register reg, int64_t value) {
    uint8_t* ptr = (uint8_t*)alloc_unaligned(mem, 10);
    if(reg >= REG_8) {
        ptr[0] = 0x49;
    } else {
        ptr[0] = 0x48;
    }
    ptr[1] = 0xb8 + reg_to_opcodeno[reg];
    ptr[2] = (value) & 0xff;
    ptr[3] = (value >> 8) & 0xff;
    ptr[4] = (value >> 16) & 0xff;
    ptr[5] = (value >> 24) & 0xff;
    ptr[6] = (value >> 32) & 0xff;
    ptr[7] = (value >> 40) & 0xff;
    ptr[8] = (value >> 48) & 0xff;
    ptr[9] = (value >> 56) & 0xff;
}
void addMovRegToReg(StackAllocator* mem, X86Register dest, X86Register src) {
    uint8_t* ptr = (uint8_t*)alloc_unaligned(mem, 3);
    ptr[0] = 0x48 + (dest >= REG_8 ? 1 : 0) + (src >= REG_8 ? 4 : 0);
    ptr[1] = 0x89;
    ptr[2] = 0xc0 + reg_to_opcodeno[dest] + (reg_to_opcodeno[src] * 8);
}

void addMovMemRegToReg(StackAllocator* mem, X86Register dest, X86Register src_pos) {
    uint8_t* ptr = (uint8_t*)alloc_unaligned(mem, 3);
    ptr[0] = 0x48 + (dest >= REG_8 ? 4 : 0) + (src_pos >= REG_8 ? 1 : 0);
    ptr[1] = 0x8b;
    ptr[2] = 0x00 + (reg_to_opcodeno[dest] * 8) + reg_to_opcodeno[src_pos];
}

void addMovRegToMemReg(StackAllocator* mem, X86Register dest_pos, X86Register src) {
    uint8_t* ptr = (uint8_t*)alloc_unaligned(mem, 3);
    ptr[0] = 0x48 + (dest_pos >= REG_8 ? 1 : 0) + (src >= REG_8 ? 4 : 0);
    ptr[1] = 0x89;
    ptr[2] = 0x00 + reg_to_opcodeno[dest_pos] + (reg_to_opcodeno[src] * 8);
}

void addRetN(StackAllocator* mem) {
    uint8_t* ptr = (uint8_t*)alloc_unaligned(mem, 1);
    ptr[0] = 0xc3;
}

void addPush(StackAllocator* mem, X86Register reg) {
    if(reg >= REG_8) {
        uint8_t* ptr = (uint8_t*)alloc_unaligned(mem, 1);
        ptr[0] = 0x41;
    }
    uint8_t* ptr = (uint8_t*)alloc_unaligned(mem, 1);
    ptr[0] = 0x50 + reg_to_opcodeno[reg];
}

void addPop(StackAllocator* mem, X86Register reg) {
    if(reg >= REG_8) {
        uint8_t* ptr = (uint8_t*)alloc_unaligned(mem, 1);
        ptr[0] = 0x41;
    }
    uint8_t* ptr = (uint8_t*)alloc_unaligned(mem, 1);
    ptr[0] = 0x58 + reg_to_opcodeno[reg];
}

void addAdd(StackAllocator* mem, X86Register dest, X86Register src) {
    uint8_t* ptr = (uint8_t*)alloc_unaligned(mem, 3);
    ptr[0] = 0x48 + (dest >= REG_8 ? 1 : 0) + (src >= REG_8 ? 4 : 0);
    ptr[1] = 0x01;
    ptr[2] = 0xc0 + reg_to_opcodeno[dest] + (reg_to_opcodeno[src] * 8);
}

void addSub(StackAllocator* mem, X86Register dest, X86Register src) {
    uint8_t* ptr = (uint8_t*)alloc_unaligned(mem, 3);
    ptr[0] = 0x48 + (dest >= REG_8 ? 1 : 0) + (src >= REG_8 ? 4 : 0);
    ptr[1] = 0x29;
    ptr[2] = 0xc0 + reg_to_opcodeno[dest] + (reg_to_opcodeno[src] * 8);
}

void addTest(StackAllocator* mem, X86Register dest, X86Register src) {
    uint8_t* ptr = (uint8_t*)alloc_unaligned(mem, 3);
    ptr[0] = 0x48 + (dest >= REG_8 ? 1 : 0) + (src >= REG_8 ? 4 : 0);
    ptr[1] = 0x85;
    ptr[2] = 0xc0 + reg_to_opcodeno[dest] + (reg_to_opcodeno[src] * 8);
}

void addIMul(StackAllocator* mem, X86Register dest, X86Register src) {
    uint8_t* ptr = (uint8_t*)alloc_unaligned(mem, 4);
    ptr[0] = 0x48 + (dest >= REG_8 ? 4 : 0) + (src >= REG_8 ? 1 : 0);
    ptr[1] = 0x0f;
    ptr[2] = 0xaf;
    ptr[3] = 0xc0 + (reg_to_opcodeno[dest] * 8) + reg_to_opcodeno[src];
}

void addIDiv(StackAllocator* mem, X86Register by) {
    uint8_t* ptr = (uint8_t*)alloc_unaligned(mem, 3);
    ptr[0] = 0x48 + (by >= REG_8 ? 1 : 0);
    ptr[1] = 0xf7;
    ptr[2] = 0xf8 + reg_to_opcodeno[by];
}

void addCallReg(StackAllocator* mem, X86Register reg) {
    if(reg >= REG_8) {
        uint8_t* ptr = (uint8_t*)alloc_unaligned(mem, 1);
        ptr[0] = 0x41;
    }
    uint8_t* ptr = (uint8_t*)alloc_unaligned(mem, 2);
    ptr[0] = 0xff;
    ptr[1] = 0xD0 + reg_to_opcodeno[reg];
}

void addJmpEQ(StackAllocator* mem, uint32_t rel) {
    uint8_t* ptr = (uint8_t*)alloc_unaligned(mem, 6);
    ptr[0] = 0x0f;
    ptr[1] = 0x84;
    ptr[2] = (rel) & 0xff;
    ptr[3] = (rel >> 8) & 0xff;
    ptr[4] = (rel >> 16) & 0xff;
    ptr[5] = (rel >> 24) & 0xff;
}

void addJmpNE(StackAllocator* mem, uint32_t rel) {
    uint8_t* ptr = (uint8_t*)alloc_unaligned(mem, 6);
    ptr[0] = 0x0f;
    ptr[1] = 0x85;
    ptr[2] = (rel) & 0xff;
    ptr[3] = (rel >> 8) & 0xff;
    ptr[4] = (rel >> 16) & 0xff;
    ptr[5] = (rel >> 24) & 0xff;
}

void addJmpGT(StackAllocator* mem, uint32_t rel) {
    uint8_t* ptr = (uint8_t*)alloc_unaligned(mem, 6);
    ptr[0] = 0x0f;
    ptr[1] = 0x8f;
    ptr[2] = (rel) & 0xff;
    ptr[3] = (rel >> 8) & 0xff;
    ptr[4] = (rel >> 16) & 0xff;
    ptr[5] = (rel >> 24) & 0xff;
}

void addJmpLS(StackAllocator* mem, uint32_t rel) {
    uint8_t* ptr = (uint8_t*)alloc_unaligned(mem, 6);
    ptr[0] = 0x0f;
    ptr[1] = 0x8c;
    ptr[2] = (rel) & 0xff;
    ptr[3] = (rel >> 8) & 0xff;
    ptr[4] = (rel >> 16) & 0xff;
    ptr[5] = (rel >> 24) & 0xff;
}

void addJmpGE(StackAllocator* mem, uint32_t rel) {
    uint8_t* ptr = (uint8_t*)alloc_unaligned(mem, 6);
    ptr[0] = 0x0f;
    ptr[1] = 0x8d;
    ptr[2] = (rel) & 0xff;
    ptr[3] = (rel >> 8) & 0xff;
    ptr[4] = (rel >> 16) & 0xff;
    ptr[5] = (rel >> 24) & 0xff;
}

void addJmpLE(StackAllocator* mem, uint32_t rel) {
    uint8_t* ptr = (uint8_t*)alloc_unaligned(mem, 6);
    ptr[0] = 0x0f;
    ptr[1] = 0x8e;
    ptr[2] = (rel) & 0xff;
    ptr[3] = (rel >> 8) & 0xff;
    ptr[4] = (rel >> 16) & 0xff;
    ptr[5] = (rel >> 24) & 0xff;
}

void addMovFRegToReg(StackAllocator* mem, X86Register dest, X86Register fsrc) {

}

void addMovRegToFReg(StackAllocator* mem, X86Register fdest, X86Register src) {

}

void addMovFRegToFReg(StackAllocator* mem, X86Register fdest, X86Register fsrc) {

}

void addMovMemRegToFReg(StackAllocator* mem, X86Register fdest, X86Register src_pos) {

}

void addMovFRegToMemReg(StackAllocator* mem, X86Register dest_pos, X86Register fsrc) {
    
}
