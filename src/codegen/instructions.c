
#include "codegen/instructions.h"
    
void addJmpRelative32(StackAllocator* mem, int32_t value) {
    uint8_t* ptr = (uint8_t*)alloc_unaligned(mem, 5);
    ptr[0] = 0xe9;
    ptr[1] = (value) & 0xff;
    ptr[2] = (value >> 8) & 0xff;
    ptr[3] = (value >> 16) & 0xff;
    ptr[4] = (value >> 24) & 0xff;
}

void addMovImm32ToReg(StackAllocator* mem, Register reg, int32_t value) {
    uint8_t* ptr = (uint8_t*)alloc_unaligned(mem, 5);
    ptr[0] = 0xb8 + reg;
    ptr[1] = (value) & 0xff;
    ptr[2] = (value >> 8) & 0xff;
    ptr[3] = (value >> 16) & 0xff;
    ptr[4] = (value >> 24) & 0xff;
}

void addRetN(StackAllocator* mem) {
    uint8_t* ptr = (uint8_t*)alloc_unaligned(mem, 1);
    ptr[0] = 0xc3;
}

void addPush(StackAllocator* mem, Register reg) {
    uint8_t* ptr = (uint8_t*)alloc_unaligned(mem, 1);
    ptr[0] = 0x50 + reg;
}

void addPop(StackAllocator* mem, Register reg) {
    uint8_t* ptr = (uint8_t*)alloc_unaligned(mem, 1);
    ptr[0] = 0x58 + reg;
}
