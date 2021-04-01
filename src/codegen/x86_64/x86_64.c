#ifdef __x86_64__

#include "codegen/x86_64/x86_64.h"

static int regToOpcodeNo(X86Register reg) {
    switch (reg) {
    case REG_A:
        return 0;
    case REG_B:
        return 3;
    case REG_C:
        return 1;
    case REG_D:
        return 2;
    case REG_DI:
        return 7;
    case REG_SI:
        return 6;

    case REG_8:
        return 0;
    case REG_9:
        return 1;
    case REG_10:
        return 2;
    case REG_11:
        return 3;
    case REG_12:
        return 4;
    case REG_13:
        return 5;
    case REG_14:
        return 6;
    case REG_15:
        return 7;

    case FREG_0:
        return 0;
    case FREG_1:
        return 1;
    case FREG_2:
        return 2;
    case FREG_3:
        return 3;
    case FREG_4:
        return 4;
    case FREG_5:
        return 5;
    case FREG_6:
        return 6;
    case FREG_7:
        return 7;
    default:
        return 0;
        break;
    }
}

void addJmpRelative32(StackAllocator* mem, int32_t value) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 5);
    ptr[0] = 0xe9;
    ptr[1] = (value)&0xff;
    ptr[2] = (value >> 8) & 0xff;
    ptr[3] = (value >> 16) & 0xff;
    ptr[4] = (value >> 24) & 0xff;
}

void addCallRel(StackAllocator* mem, int32_t value) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 5);
    ptr[0] = 0xe8;
    ptr[1] = (value)&0xff;
    ptr[2] = (value >> 8) & 0xff;
    ptr[3] = (value >> 16) & 0xff;
    ptr[4] = (value >> 24) & 0xff;
}

void addJmpAbsoluteReg(StackAllocator* mem, X86Register reg) {
    if (reg >= REG_8) {
        uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 1);
        ptr[0] = 0x41;
    }
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 2);
    ptr[0] = 0xff;
    ptr[1] = 0xE0 + regToOpcodeNo(reg);
}

void addMovImm32ToReg(StackAllocator* mem, X86Register reg, int32_t value) {
    if (reg >= REG_8) {
        uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 1);
        ptr[0] = 0x41;
    }
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 5);
    ptr[0] = 0xb8 + regToOpcodeNo(reg);
    ptr[1] = (value)&0xff;
    ptr[2] = (value >> 8) & 0xff;
    ptr[3] = (value >> 16) & 0xff;
    ptr[4] = (value >> 24) & 0xff;
}

void addMovImm64ToReg(StackAllocator* mem, X86Register reg, int64_t value) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 10);
    if (reg >= REG_8) {
        ptr[0] = 0x49;
    } else {
        ptr[0] = 0x48;
    }
    ptr[1] = 0xb8 + regToOpcodeNo(reg);
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
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 3);
    ptr[0] = 0x48 + (dest >= REG_8 ? 1 : 0) + (src >= REG_8 ? 4 : 0);
    ptr[1] = 0x89;
    ptr[2] = 0xc0 + regToOpcodeNo(dest) + (regToOpcodeNo(src) * 8);
}

void addMovMemRegToReg(StackAllocator* mem, X86Register dest, X86Register src_pos) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 3);
    ptr[0] = 0x48 + (dest >= REG_8 ? 4 : 0) + (src_pos >= REG_8 ? 1 : 0);
    ptr[1] = 0x8b;
    ptr[2] = 0x00 + (regToOpcodeNo(dest) * 8) + regToOpcodeNo(src_pos);
}

void addMovRegToMemReg(StackAllocator* mem, X86Register dest_pos, X86Register src) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 3);
    ptr[0] = 0x48 + (dest_pos >= REG_8 ? 1 : 0) + (src >= REG_8 ? 4 : 0);
    ptr[1] = 0x89;
    ptr[2] = 0x00 + regToOpcodeNo(dest_pos) + (regToOpcodeNo(src) * 8);
}

void addRetN(StackAllocator* mem) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 1);
    ptr[0] = 0xc3;
}

void addPush(StackAllocator* mem, X86Register reg) {
    if (reg >= REG_8) {
        uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 1);
        ptr[0] = 0x41;
    }
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 1);
    ptr[0] = 0x50 + regToOpcodeNo(reg);
}

void addPop(StackAllocator* mem, X86Register reg) {
    if (reg >= REG_8) {
        uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 1);
        ptr[0] = 0x41;
    }
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 1);
    ptr[0] = 0x58 + regToOpcodeNo(reg);
}

void addAdd(StackAllocator* mem, X86Register dest, X86Register src) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 3);
    ptr[0] = 0x48 + (dest >= REG_8 ? 1 : 0) + (src >= REG_8 ? 4 : 0);
    ptr[1] = 0x01;
    ptr[2] = 0xc0 + regToOpcodeNo(dest) + (regToOpcodeNo(src) * 8);
}

void addSub(StackAllocator* mem, X86Register dest, X86Register src) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 3);
    ptr[0] = 0x48 + (dest >= REG_8 ? 1 : 0) + (src >= REG_8 ? 4 : 0);
    ptr[1] = 0x29;
    ptr[2] = 0xc0 + regToOpcodeNo(dest) + (regToOpcodeNo(src) * 8);
}

void addAnd(StackAllocator* mem, X86Register dest, X86Register src) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 3);
    ptr[0] = 0x48 + (dest >= REG_8 ? 1 : 0) + (src >= REG_8 ? 4 : 0);
    ptr[1] = 0x21;
    ptr[2] = 0xc0 + regToOpcodeNo(dest) + (regToOpcodeNo(src) * 8);
}

void addXor(StackAllocator* mem, X86Register dest, X86Register src) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 3);
    ptr[0] = 0x48 + (dest >= REG_8 ? 1 : 0) + (src >= REG_8 ? 4 : 0);
    ptr[1] = 0x31;
    ptr[2] = 0xc0 + regToOpcodeNo(dest) + (regToOpcodeNo(src) * 8);
}

void addOr(StackAllocator* mem, X86Register dest, X86Register src) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 3);
    ptr[0] = 0x48 + (dest >= REG_8 ? 1 : 0) + (src >= REG_8 ? 4 : 0);
    ptr[1] = 0x09;
    ptr[2] = 0xc0 + regToOpcodeNo(dest) + (regToOpcodeNo(src) * 8);
}

void addNot(StackAllocator* mem, X86Register src) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 3);
    ptr[0] = 0x48 + (src >= REG_8 ? 1 : 0);
    ptr[1] = 0xf7;
    ptr[2] = 0xd0 + regToOpcodeNo(src);
}

void addTest(StackAllocator* mem, X86Register dest, X86Register src) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 3);
    ptr[0] = 0x48 + (dest >= REG_8 ? 1 : 0) + (src >= REG_8 ? 4 : 0);
    ptr[1] = 0x85;
    ptr[2] = 0xc0 + regToOpcodeNo(dest) + (regToOpcodeNo(src) * 8);
}

void addCmp(StackAllocator* mem, X86Register dest, X86Register src) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 3);
    ptr[0] = 0x48 + (dest >= REG_8 ? 1 : 0) + (src >= REG_8 ? 4 : 0);
    ptr[1] = 0x39;
    ptr[2] = 0xc0 + regToOpcodeNo(dest) + (regToOpcodeNo(src) * 8);
}

void addIMul(StackAllocator* mem, X86Register dest, X86Register src) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 4);
    ptr[0] = 0x48 + (dest >= REG_8 ? 4 : 0) + (src >= REG_8 ? 1 : 0);
    ptr[1] = 0x0f;
    ptr[2] = 0xaf;
    ptr[3] = 0xc0 + (regToOpcodeNo(dest) * 8) + regToOpcodeNo(src);
}

void addIMulRax(StackAllocator* mem, X86Register src) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 3);
    ptr[0] = 0x48 + (src >= REG_8 ? 1 : 0);
    ptr[1] = 0xf7;
    ptr[2] = 0xe8 + regToOpcodeNo(src);
}

void addIDiv(StackAllocator* mem, X86Register by) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 3);
    ptr[0] = 0x48 + (by >= REG_8 ? 1 : 0);
    ptr[1] = 0xf7;
    ptr[2] = 0xf8 + regToOpcodeNo(by);
}

void addCallReg(StackAllocator* mem, X86Register reg) {
    if (reg >= REG_8) {
        uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 1);
        ptr[0] = 0x41;
    }
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 2);
    ptr[0] = 0xff;
    ptr[1] = 0xD0 + regToOpcodeNo(reg);
}

void addJmpEQ(StackAllocator* mem, int32_t rel) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 6);
    ptr[0] = 0x0f;
    ptr[1] = 0x84;
    ptr[2] = (rel)&0xff;
    ptr[3] = (rel >> 8) & 0xff;
    ptr[4] = (rel >> 16) & 0xff;
    ptr[5] = (rel >> 24) & 0xff;
}

void addJmpNE(StackAllocator* mem, int32_t rel) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 6);
    ptr[0] = 0x0f;
    ptr[1] = 0x85;
    ptr[2] = (rel)&0xff;
    ptr[3] = (rel >> 8) & 0xff;
    ptr[4] = (rel >> 16) & 0xff;
    ptr[5] = (rel >> 24) & 0xff;
}

void addJmpGT(StackAllocator* mem, int32_t rel) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 6);
    ptr[0] = 0x0f;
    ptr[1] = 0x8f;
    ptr[2] = (rel)&0xff;
    ptr[3] = (rel >> 8) & 0xff;
    ptr[4] = (rel >> 16) & 0xff;
    ptr[5] = (rel >> 24) & 0xff;
}

void addJmpLT(StackAllocator* mem, int32_t rel) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 6);
    ptr[0] = 0x0f;
    ptr[1] = 0x8c;
    ptr[2] = (rel)&0xff;
    ptr[3] = (rel >> 8) & 0xff;
    ptr[4] = (rel >> 16) & 0xff;
    ptr[5] = (rel >> 24) & 0xff;
}

void addJmpGE(StackAllocator* mem, int32_t rel) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 6);
    ptr[0] = 0x0f;
    ptr[1] = 0x8d;
    ptr[2] = (rel)&0xff;
    ptr[3] = (rel >> 8) & 0xff;
    ptr[4] = (rel >> 16) & 0xff;
    ptr[5] = (rel >> 24) & 0xff;
}

void addJmpLE(StackAllocator* mem, int32_t rel) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 6);
    ptr[0] = 0x0f;
    ptr[1] = 0x8e;
    ptr[2] = (rel)&0xff;
    ptr[3] = (rel >> 8) & 0xff;
    ptr[4] = (rel >> 16) & 0xff;
    ptr[5] = (rel >> 24) & 0xff;
}

void addJmpA(StackAllocator* mem, int32_t rel) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 6);
    ptr[0] = 0x0f;
    ptr[1] = 0x87;
    ptr[2] = (rel)&0xff;
    ptr[3] = (rel >> 8) & 0xff;
    ptr[4] = (rel >> 16) & 0xff;
    ptr[5] = (rel >> 24) & 0xff;
}

void addJmpB(StackAllocator* mem, int32_t rel) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 6);
    ptr[0] = 0x0f;
    ptr[1] = 0x82;
    ptr[2] = (rel)&0xff;
    ptr[3] = (rel >> 8) & 0xff;
    ptr[4] = (rel >> 16) & 0xff;
    ptr[5] = (rel >> 24) & 0xff;
}

void addJmpAE(StackAllocator* mem, int32_t rel) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 6);
    ptr[0] = 0x0f;
    ptr[1] = 0x83;
    ptr[2] = (rel)&0xff;
    ptr[3] = (rel >> 8) & 0xff;
    ptr[4] = (rel >> 16) & 0xff;
    ptr[5] = (rel >> 24) & 0xff;
}

void addJmpBE(StackAllocator* mem, int32_t rel) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 6);
    ptr[0] = 0x0f;
    ptr[1] = 0x86;
    ptr[2] = (rel)&0xff;
    ptr[3] = (rel >> 8) & 0xff;
    ptr[4] = (rel >> 16) & 0xff;
    ptr[5] = (rel >> 24) & 0xff;
}

void addMovFRegToReg(StackAllocator* mem, X86Register dest, X86Register fsrc) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 5);
    ptr[0] = 0x66;
    ptr[1] = 0x48 + (dest >= REG_8 ? 1 : 0);
    ptr[2] = 0x0f;
    ptr[3] = 0x7e;
    ptr[4] = 0xc0 + (regToOpcodeNo(fsrc) * 8) + regToOpcodeNo(dest);
}

void addMovRegToFReg(StackAllocator* mem, X86Register fdest, X86Register src) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 5);
    ptr[0] = 0x66;
    ptr[1] = 0x48 + (src >= REG_8 ? 1 : 0);
    ptr[2] = 0x0f;
    ptr[3] = 0x6e;
    ptr[4] = 0xc0 + (regToOpcodeNo(fdest) * 8) + regToOpcodeNo(src);
}

void addMovFRegToFReg(StackAllocator* mem, X86Register fdest, X86Register fsrc) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 4);
    ptr[0] = 0xf3;
    ptr[1] = 0x0f;
    ptr[2] = 0x7e;
    ptr[3] = 0xc0 + (regToOpcodeNo(fdest) * 8) + regToOpcodeNo(fsrc);
}

void addMovMemRegToFReg(StackAllocator* mem, X86Register fdest, X86Register src_pos) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 4);
    ptr[0] = 0xf3;
    ptr[1] = 0x0f;
    ptr[2] = 0x7e;
    ptr[3] = 0x00 + (regToOpcodeNo(fdest) * 8) + regToOpcodeNo(src_pos);
}

void addMovFRegToMemReg(StackAllocator* mem, X86Register dest_pos, X86Register fsrc) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 4);
    ptr[0] = 0x66;
    ptr[1] = 0x0f;
    ptr[2] = 0xd6;
    ptr[3] = 0x00 + (regToOpcodeNo(fsrc) * 8) + regToOpcodeNo(dest_pos);
}

void addFAdd(StackAllocator* mem, X86Register fdest, X86Register fsrc) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 4);
    ptr[0] = 0xf2;
    ptr[1] = 0x0f;
    ptr[2] = 0x58;
    ptr[3] = 0xC0 + (regToOpcodeNo(fdest) * 8) + regToOpcodeNo(fsrc);
}

void addFSub(StackAllocator* mem, X86Register fdest, X86Register fsrc) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 4);
    ptr[0] = 0xf2;
    ptr[1] = 0x0f;
    ptr[2] = 0x5c;
    ptr[3] = 0xC0 + (regToOpcodeNo(fdest) * 8) + regToOpcodeNo(fsrc);
}

void addFMul(StackAllocator* mem, X86Register fdest, X86Register fsrc) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 4);
    ptr[0] = 0xf2;
    ptr[1] = 0x0f;
    ptr[2] = 0x59;
    ptr[3] = 0xC0 + (regToOpcodeNo(fdest) * 8) + regToOpcodeNo(fsrc);
}

void addFDiv(StackAllocator* mem, X86Register fdest, X86Register fsrc) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 4);
    ptr[0] = 0xf2;
    ptr[1] = 0x0f;
    ptr[2] = 0x5e;
    ptr[3] = 0xC0 + (regToOpcodeNo(fdest) * 8) + regToOpcodeNo(fsrc);
}

void addPxor(StackAllocator* mem, X86Register fdest, X86Register fsrc) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 4);
    ptr[0] = 0x66;
    ptr[1] = 0x0f;
    ptr[2] = 0xef;
    ptr[3] = 0xC0 + (regToOpcodeNo(fdest) * 8) + regToOpcodeNo(fsrc);
}

void addFCom(StackAllocator* mem, X86Register fdest, X86Register fsrc) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 4);
    ptr[0] = 0x66;
    ptr[1] = 0x0f;
    ptr[2] = 0x2f;
    ptr[3] = 0xC0 + (regToOpcodeNo(fdest) * 8) + regToOpcodeNo(fsrc);
}

void addFRegCvtToReg(StackAllocator* mem, X86Register dest, X86Register fsrc) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 5);
    ptr[0] = 0xF2;
    ptr[1] = 0x48 + (dest >= REG_8 ? 4 : 0);
    ptr[2] = 0x0F;
    ptr[3] = 0x2C;
    ptr[4] = 0xC0 + (regToOpcodeNo(dest) * 8) + regToOpcodeNo(fsrc);
}

void addRegCvtToFReg(StackAllocator* mem, X86Register fdest, X86Register src) {
    uint8_t* ptr = (uint8_t*)allocUnaligned(mem, 5);
    ptr[0] = 0xF2;
    ptr[1] = 0x48 + (src >= REG_8 ? 1 : 0);
    ptr[2] = 0x0F;
    ptr[3] = 0x2A;
    ptr[4] = 0xC0 + (regToOpcodeNo(fdest) * 8) + regToOpcodeNo(src);
}

#endif
