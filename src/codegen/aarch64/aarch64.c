
#ifdef __aarch64__

#include "codegen/aarch64/aarch64.h"

void addInstruction(StackAllocator* mem, Aarch64Instruction instr) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 4);
    ptr[0] = (instr.instruction) & 0xff;
    ptr[1] = (instr.instruction >> 8) & 0xff;
    ptr[2] = (instr.instruction >> 16) & 0xff;
    ptr[3] = (instr.instruction >> 24) & 0xff;
}

void updateInstruction(StackAllocator* mem, size_t pos, Aarch64Instruction instr) {
    uint8_t* ptr = mem->memory + pos;
    ptr[0] = (instr.instruction) & 0xff;
    ptr[1] = (instr.instruction >> 8) & 0xff;
    ptr[2] = (instr.instruction >> 16) & 0xff;
    ptr[3] = (instr.instruction >> 24) & 0xff;
}

Aarch64Instruction getInstruction(StackAllocator* mem, size_t pos) {
    uint8_t* ptr = mem->memory + pos;
    Aarch64Instruction instr;
    instr.instruction = 0;
    instr.instruction |= ((uint32_t)ptr[0]);
    instr.instruction |= ((uint32_t)ptr[1]) << 8UL;
    instr.instruction |= ((uint32_t)ptr[2]) << 16UL;
    instr.instruction |= ((uint32_t)ptr[3]) << 24UL;
    return instr;
}

#endif
