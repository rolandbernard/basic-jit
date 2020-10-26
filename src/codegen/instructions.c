
#include "codegen/instructions.h"

#ifdef __x86_64__

#include "codegen/x86-64.h"

uint64_t getFreeRegister(RegisterSet regs) {
    for(int i = 0; i < REG_COUNT; i++) {
        if((regs & (1 << i)) == 0) {
            return 1 << i;
        }
    }
    return 0;
}

uint64_t getFreeFRegister(RegisterSet regs) {
    for(int i = REG_COUNT; i < REG_COUNT + FREG_COUNT; i++) {
        if((regs & (1 << i)) == 0) {
            return 1 << i;
        }
    }
    return 0;
}

uint64_t getUsedRegister(RegisterSet regs) {
    for(int i = 0; i < REG_COUNT; i++) {
        if((regs & (1 << i)) != 0) {
            return 1 << i;
        }
    }
    return 0;
}

uint64_t getUsedFRegister(RegisterSet regs) {
    for(int i = REG_COUNT; i < REG_COUNT + FREG_COUNT; i++) {
        if((regs & (1 << i)) != 0) {
            return 1 << i;
        }
    }
    return 0;
}

int countFreeRegister(RegisterSet regs) {
    int ret = 0;
    for(int i = 0; i < REG_COUNT; i++) {
        if((regs & (1 << i)) == 0) {
            ret++;
        }
    }
    return ret;
}

int countFreeFRegister(RegisterSet regs) {
    int ret = 0;
    for(int i = REG_COUNT; i < REG_COUNT + FREG_COUNT; i++) {
        if((regs & (1 << i)) == 0) {
            ret++;
        }
    }
    return ret;
}

uint64_t getFirstRegister() {
    return 1;
}

uint64_t getFirstFRegister() {
    return 1 << REG_COUNT;
}

void addInstMovRegToReg(StackAllocator* mem, RegisterSet regs, Register dest, Register src) {
    addMovRegToReg(mem, dest, src);
}

size_t addInstMovImmToReg(StackAllocator* mem, RegisterSet regs, Register reg, int64_t value, bool force64) {
    if((value >> 32 == 0 || value >> 32 == -1) && !force64) {
        if(value == 0) {
            addXor(mem, reg, reg);
            return -1;
        } else {
            addMovImm32ToReg(mem, reg, value);
            return mem->occupied - 4;
        }
    } else {
        addMovImm64ToReg(mem, reg, value);
        return mem->occupied - 8;
    }
}

void addInstMovMemToReg(StackAllocator* mem, RegisterSet regs, Register reg, void* addr) {
    addMovImm64ToReg(mem, reg, (uint64_t)addr);
    addMovMemRegToReg(mem, reg, reg);
}

size_t addInstMovRegToMem(StackAllocator* mem, RegisterSet regs, Register reg, void* addr) {
    size_t ret = 0;
    int free_reg = getFreeRegister(regs);
    if(free_reg == 0) {
        addPush(mem, REG_A);
        addMovImm64ToReg(mem, REG_A, (uint64_t)addr);
        ret = mem->occupied - 8;
        addMovRegToMemReg(mem, reg, reg);
        addPop(mem, REG_A);
    } else {
        addMovImm64ToReg(mem, free_reg, (uint64_t)addr);
        ret = mem->occupied - 8;
        addMovRegToMemReg(mem, free_reg, reg);
    }
    return ret;
}

void addInstMovMemRegToReg(StackAllocator* mem, RegisterSet regs, Register reg, Register addr) {
    addMovMemRegToReg(mem, reg, addr);
}

void addInstMovRegToMemReg(StackAllocator* mem, RegisterSet regs, Register addr, Register reg) {
    addMovRegToMemReg(mem, addr, reg);
}

void addInstJmp(StackAllocator* mem, RegisterSet regs, void* to) {
    int free_reg = getFreeRegister(regs);
    if(free_reg == 0) {
        addPush(mem, REG_A);
        addMovImm64ToReg(mem, REG_A, (uint64_t)to);
        addJmpAbsoluteReg(mem, REG_A);
        addPop(mem, REG_A);
    } else {
        addMovImm64ToReg(mem, free_reg, (uint64_t)to);
        addJmpAbsoluteReg(mem, free_reg);
    }
}

size_t addInstJmpRel(StackAllocator* mem, RegisterSet regs, int32_t to) {
    addJmpRelative32(mem, to);
    return mem->occupied - 4;
}

void addInstPush(StackAllocator* mem, RegisterSet regs, Register reg) {
    if(reg >= (1 << REG_COUNT)) {
        int free_reg = getFreeRegister(regs);
        if(free_reg == 0) {
            addPush(mem, REG_A);
            addMovFRegToReg(mem, REG_A, reg);
            addPush(mem, REG_A);
            addPop(mem, REG_A);
        } else {
            addMovFRegToReg(mem, free_reg, reg);
            addPush(mem, free_reg);
        }
    } else {
        addPush(mem, reg);
    }
}

void addInstPop(StackAllocator* mem, RegisterSet regs, Register reg) {
    if(reg >= (1 << REG_COUNT)) {
        int free_reg = getFreeRegister(regs);
        if(free_reg == 0) {
            addPush(mem, REG_A);
            addPop(mem, REG_A);
            addMovRegToFReg(mem, reg, REG_A);
            addPop(mem, REG_A);
        } else {
            addPop(mem, free_reg);
            addMovRegToFReg(mem, reg, free_reg);
        }
    } else {
        addPop(mem, reg);
    }
}

void addInstPushAll(StackAllocator* mem, RegisterSet regs) {
    int push_cnt = 0;
    Register double_pop = 0;
    for(int i = 0; i < REG_COUNT + FREG_COUNT; i++) {
        if((regs & (1 << i)) != 0) {
            push_cnt++;
            double_pop = (1 << i); 
            addInstPush(mem, regs, (1 << i));
        }
    }
    if(push_cnt % 2 == 1) {
        addPush(mem, double_pop);
    }
}

void addInstPopAll(StackAllocator* mem, RegisterSet regs) {
    int pop_cnt = 0;
    Register double_pop = 0;
    for(int i = REG_COUNT + FREG_COUNT - 1; i >= 0; i--) {
        if((regs & (1 << i)) != 0) {
            pop_cnt++;
            double_pop = (1 << i); 
        }
    }
    if(pop_cnt % 2 == 1) {
        addPop(mem, double_pop);
    }
    for(int i = REG_COUNT + FREG_COUNT - 1; i >= 0; i--) {
        if((regs & (1 << i)) != 0) {
            addInstPop(mem, regs, (1 << i));
        }
    }
}

size_t addInstCallRel(StackAllocator* mem, RegisterSet regs, int32_t func) {
    addCallRel(mem, func);
    return mem->occupied - 4;
}

void addInstCall(StackAllocator* mem, RegisterSet regs, void* func) {
    int free_reg = getFreeRegister(regs);
    if(free_reg == 0) {
        addPush(mem, REG_A);
        addMovImm64ToReg(mem, REG_A, (uint64_t)func);
        addCallReg(mem, REG_A);
        addPop(mem, REG_A);
    } else {
        addMovImm64ToReg(mem, free_reg, (uint64_t)func);
        addCallReg(mem, free_reg);
    }
}

void addInstReturn(StackAllocator* mem, RegisterSet regs) {
    addRetN(mem);
}

void addInstAdd(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b) {
    if(a == dest) {
        addAdd(mem, dest, b);
    } else if(b == dest) {
        addAdd(mem, dest, a);
    } else {
        addMovRegToReg(mem, dest, a);
        addAdd(mem, dest, b);
    }
}

void addInstSub(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b) {
    if(a == dest) {
        addSub(mem, dest, b);
    } else if(b == dest) {
        int free_reg = getFreeRegister(regs);
        if (free_reg == 0) {
            addPush(mem, REG_A);
            addMovRegToReg(mem, REG_A, b);
            addMovRegToReg(mem, dest, a);
            addSub(mem, dest, REG_A);
            addPop(mem, REG_A);
        } else {
            addMovRegToReg(mem, free_reg, b);
            addMovRegToReg(mem, dest, a);
            addSub(mem, dest, free_reg);
        }
    } else {
        addMovRegToReg(mem, dest, a);
        addSub(mem, dest, b);
    }
}

void addInstMul(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b) {
    if(a == dest) {
        addIMul(mem, dest, b);
    } else if(b == dest) {
        addIMul(mem, dest, a);
    } else {
        addMovRegToReg(mem, dest, a);
        addIMul(mem, dest, b);
    }
}

void addInstDiv(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b) {
    if (((regs & REG_A) != 0) && dest != REG_A) {
        addPush(mem, REG_A);
    }
    if (((regs & REG_D) != 0) && dest != REG_D) {
        addPush(mem, REG_D);
    }
    if(b == REG_D) {
        int free_reg = getFreeRegister(regs);
        if(free_reg == 0) {
            addPush(mem, REG_B);
            addMovRegToReg(mem, REG_B, b);
            if(a != REG_A) {
                addMovRegToReg(mem, REG_A, a);
            }
            addXor(mem, REG_D, REG_D);
            addIDiv(mem, REG_B);
            if(dest != REG_A) {
                addMovRegToReg(mem, dest, REG_A);
            }
            addPop(mem, REG_B);
        } else {
            addMovRegToReg(mem, free_reg, b);
            if(a != REG_A) {
                addMovRegToReg(mem, REG_A, a);
            }
            addXor(mem, REG_D, REG_D);
            addIDiv(mem, free_reg);
            if(dest != REG_A) {
                addMovRegToReg(mem, dest, REG_A);
            }
        }
    } else {
        if(a != REG_A) {
            addMovRegToReg(mem, REG_A, a);
        }
        addXor(mem, REG_D, REG_D);
        addIDiv(mem, b);
        if(dest != REG_A) {
            addMovRegToReg(mem, dest, REG_A);
        }
    }
    if (((regs & REG_D) != 0) && dest != REG_D) {
        addPop(mem, REG_D);
    }
    if (((regs & REG_A) != 0) && dest != REG_A) {
        addPop(mem, REG_A);
    }
}

void addInstRem(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b) {
    if (((regs & REG_A) != 0) && dest != REG_A) {
        addPush(mem, REG_A);
    }
    if (((regs & REG_D) != 0) && dest != REG_D) {
        addPush(mem, REG_D);
    }
    if(b == REG_D) {
        int free_reg = getFreeRegister(regs);
        if(free_reg == 0) {
            addPush(mem, REG_B);
            addMovRegToReg(mem, REG_B, b);
            if(a != REG_A) {
                addMovRegToReg(mem, REG_A, a);
            }
            addXor(mem, REG_D, REG_D);
            addIDiv(mem, REG_B);
            if(dest != REG_D) {
                addMovRegToReg(mem, dest, REG_D);
            }
            addPop(mem, REG_B);
        } else {
            addMovRegToReg(mem, free_reg, b);
            if(a != REG_A) {
                addMovRegToReg(mem, REG_A, a);
            }
            addXor(mem, REG_D, REG_D);
            addIDiv(mem, free_reg);
            if(dest != REG_D) {
                addMovRegToReg(mem, dest, REG_D);
            }
        }
    } else {
        if(a != REG_A) {
            addMovRegToReg(mem, REG_A, a);
        }
        addXor(mem, REG_D, REG_D);
        addIDiv(mem, b);
        if(dest != REG_D) {
            addMovRegToReg(mem, dest, REG_D);
        }
    }
    if (((regs & REG_D) != 0) && dest != REG_D) {
        addPop(mem, REG_D);
    }
    if (((regs & REG_A) != 0) && dest != REG_A) {
        addPop(mem, REG_A);
    }
}

size_t addInstCondJmpRel(StackAllocator* mem, RegisterSet regs, JmpCondistions cond, Register a, Register b, int32_t to) {
    if(a < (1 << REG_COUNT)) {
        addCmp(mem, a, b);
        switch(cond) {
            case COND_EQ:
                addJmpEQ(mem, to);
                break;
            case COND_NE:
                addJmpNE(mem, to);
                break;
            case COND_GT:
                addJmpGT(mem, to);
                break;
            case COND_LT:
                addJmpLT(mem, to);
                break;
            case COND_GE:
                addJmpGE(mem, to);
                break;
            case COND_LE:
                addJmpLE(mem, to);
                break;
        }
    } else {
        addFCom(mem, a, b);
        switch(cond) {
            case COND_EQ:
                addJmpEQ(mem, to);
                break;
            case COND_NE:
                addJmpNE(mem, to);
                break;
            case COND_GT:
                addJmpA(mem, to);
                break;
            case COND_LT:
                addJmpB(mem, to);
                break;
            case COND_GE:
                addJmpAE(mem, to);
                break;
            case COND_LE:
                addJmpBE(mem, to);
                break;
        }
    }
    return mem->occupied - 4;
}

void addInstFAdd(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b) {
    if(a == dest) {
        addFAdd(mem, dest, b);
    } else if(b == dest) {
        addFAdd(mem, dest, a);
    } else {
        addMovFRegToFReg(mem, dest, a);
        addFAdd(mem, dest, b);
    }
}

void addInstFSub(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b) {
    if(a == dest) {
        addFSub(mem, dest, b);
    } else if(b == dest) {
        int free_reg = getFreeFRegister(regs);
        if (free_reg == 0) {
            addPush(mem, FREG_0);
            addMovFRegToFReg(mem, FREG_0, b);
            addMovFRegToFReg(mem, dest, a);
            addFSub(mem, dest, FREG_0);
            addPop(mem, FREG_0);
        } else {
            addMovFRegToFReg(mem, free_reg, b);
            addMovFRegToFReg(mem, dest, a);
            addFSub(mem, dest, free_reg);
        }
    } else {
        addMovFRegToFReg(mem, dest, a);
        addFSub(mem, dest, b);
    }
}

void addInstFMul(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b) {
    if(a == dest) {
        addFMul(mem, dest, b);
    } else if(b == dest) {
        addFMul(mem, dest, a);
    } else {
        addMovFRegToFReg(mem, dest, a);
        addFMul(mem, dest, b);
    }
}

void addInstFDiv(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b) {
    if(a == dest) {
        addFDiv(mem, dest, b);
    } else if(b == dest) {
        addFDiv(mem, dest, a);
    } else {
        addMovFRegToFReg(mem, dest, a);
        addFDiv(mem, dest, b);
    }
}

void addInstMovImmToFReg(StackAllocator* mem, RegisterSet regs, Register reg, double value) {
    union {
        int64_t i;
        double d;
    } double_to_uint64;
    double_to_uint64.d = value;
    int free_reg = getFreeRegister(regs);
    if(free_reg == 0) {
        addPush(mem, REG_A);
        addMovImm64ToReg(mem, REG_A, double_to_uint64.i);
        addMovRegToFReg(mem, reg, REG_A);
        addPop(mem, REG_A);
    } else {
        addMovImm64ToReg(mem, free_reg, double_to_uint64.i);
        addMovRegToFReg(mem, reg, free_reg);
    }
}

void addInstMovMemToFReg(StackAllocator* mem, RegisterSet regs, Register reg, void* addr) {
    int free_reg = getFreeRegister(regs);
    if(free_reg == 0) {
        addPush(mem, REG_A);
        addMovImm64ToReg(mem, REG_A, (uint64_t)addr);
        addMovMemRegToFReg(mem, reg, REG_A);
        addPop(mem, REG_A);
    } else {
        addMovImm64ToReg(mem, free_reg, (uint64_t)addr);
        addMovMemRegToFReg(mem, reg, free_reg);
    }
}

void addInstMovFRegToMem(StackAllocator* mem, RegisterSet regs, Register reg, void* addr) {
    int free_reg = getFreeRegister(regs);
    if(free_reg == 0) {
        addPush(mem, REG_A);
        addMovImm64ToReg(mem, REG_A, (uint64_t)addr);
        addMovFRegToMemReg(mem, REG_A, reg);
        addPop(mem, REG_A);
    } else {
        addMovImm64ToReg(mem, free_reg, (uint64_t)addr);
        addMovFRegToMemReg(mem, free_reg, reg);
    }
}

void addInstMovFRegToFReg(StackAllocator* mem, RegisterSet regs, Register dest, Register src) {
    addMovFRegToFReg(mem, dest, src);
}

void addInstMovRegToFReg(StackAllocator* mem, RegisterSet regs, Register dest, Register src) {
    addRegCvtToFReg(mem, dest, src);
}

void addInstMovFRegToReg(StackAllocator* mem, RegisterSet regs, Register dest, Register src) {
    addFRegCvtToReg(mem, dest, src);
}

void addInstMovMemRegToFReg(StackAllocator* mem, RegisterSet regs, Register reg, Register addr) {
    addMovMemRegToFReg(mem, reg, addr);
}

void addInstMovFRegToMemReg(StackAllocator* mem, RegisterSet regs, Register addr, Register reg) {
    addMovFRegToMemReg(mem, addr, reg);
}

void addInstFunctionCallUnary(StackAllocator* mem, RegisterSet regs, Register ret, Register a, void* func) {
    addInstPushAll(mem, regs & ~ret);
    if(a >= (1 << REG_COUNT)) {
        if(a != FREG_0) {
            addMovFRegToFReg(mem, FREG_0, a);
        }
    } else {
        if(a != REG_DI) {
            addMovRegToReg(mem, REG_DI, a);
        }
    }
    addInstCall(mem, regs, func);
    if(ret >= (1 << REG_COUNT)) {
        if(ret != FREG_0) {
            addMovFRegToFReg(mem, ret, FREG_0);
        }
    } else {
        if(ret != REG_A) {
            addMovRegToReg(mem, ret, REG_A);
        }
    }
    addInstPopAll(mem, regs & ~ret);
}

void addInstFunctionCallBinary(StackAllocator* mem, RegisterSet regs, Register ret, Register a, Register b, void* func) {
    addInstPushAll(mem, regs & ~ret);
    if(a >= (1 << REG_COUNT)) {
        if(a != FREG_0) {
            if(b == FREG_0) {
                if(a == FREG_1) {
                    addMovFRegToFReg(mem, FREG_2, b);
                    b = FREG_2;
                } else {
                    addMovFRegToFReg(mem, FREG_1, b);
                    b = FREG_1;
                }
            }
            addMovFRegToFReg(mem, FREG_0, a);
        }
    } else {
        if(a != REG_DI) {
            if(b == REG_DI) {
                if(a == REG_SI) {
                    addMovFRegToFReg(mem, REG_D, b);
                    b = REG_D;
                } else {
                    addMovFRegToFReg(mem, REG_SI, b);
                    b = REG_SI;
                }
            }
            addMovRegToReg(mem, REG_DI, a);
        }
    }
    if(b >= (1 << REG_COUNT)) {
        if(b != FREG_1) {
            addMovFRegToFReg(mem, FREG_1, b);
        }
    } else {
        if(b != REG_SI) {
            addMovRegToReg(mem, REG_SI, b);
        }
    }
    addInstCall(mem, regs, func);
    if(ret >= (1 << REG_COUNT)) {
        if(ret != FREG_0) {
            addMovFRegToFReg(mem, ret, FREG_0);
        }
    } else {
        if(ret != REG_A) {
            addMovRegToReg(mem, ret, REG_A);
        }
    }
    addInstPopAll(mem, regs & ~ret);
}

void addInstFunctionCallUnaryNoRet(StackAllocator* mem, RegisterSet regs, Register a, void* func) {
    addInstPushAll(mem, regs);
    if(a >= (1 << REG_COUNT)) {
        if(a != FREG_0) {
            addMovFRegToFReg(mem, FREG_0, a);
        }
    } else {
        if(a != REG_DI) {
            addMovRegToReg(mem, REG_DI, a);
        }
    }
    addInstCall(mem, regs, func);
    addInstPopAll(mem, regs);
}

void addInstFunctionCallRetOnly(StackAllocator* mem, RegisterSet regs, Register ret, void* func) {
    addInstPushAll(mem, regs & ~ret);
    addInstCall(mem, regs, func);
    if(ret >= (1 << REG_COUNT)) {
        if(ret != FREG_0) {
            addMovFRegToFReg(mem, ret, FREG_0);
        }
    } else {
        if(ret != REG_A) {
            addMovRegToReg(mem, ret, REG_A);
        }
    }
    addInstPopAll(mem, regs & ~ret);
}

void addInstFunctionCallSimple(StackAllocator* mem, RegisterSet regs, void* func) {
    addInstPushAll(mem, regs);
    addInstCall(mem, regs, func);
    addInstPopAll(mem, regs);
}

void addInstPushCallerRegs(StackAllocator* mem, RegisterSet regs) {
    addInstPush(mem, regs, REG_B);
    addInstPush(mem, regs, REG_12);
    addInstPush(mem, regs, REG_13);
    addInstPush(mem, regs, REG_14);
    addInstPush(mem, regs, REG_15);
}

void addInstPopCallerRegs(StackAllocator* mem, RegisterSet regs) {
    addInstPop(mem, regs, REG_15);
    addInstPop(mem, regs, REG_14);
    addInstPop(mem, regs, REG_13);
    addInstPop(mem, regs, REG_12);
    addInstPop(mem, regs, REG_B);
}

void update32BitValue(StackAllocator* mem, size_t pos, int32_t value) {
    ((uint8_t*)mem->memory)[pos] = value & 0xff;
    ((uint8_t*)mem->memory)[pos + 1] = (value >> 8) & 0xff;
    ((uint8_t*)mem->memory)[pos + 2] = (value >> 16) & 0xff;
    ((uint8_t*)mem->memory)[pos + 3] = (value >> 24) & 0xff;
}

void update64BitValue(StackAllocator* mem, size_t pos, int64_t value) {
    ((uint8_t*)mem->memory)[pos] = value & 0xff;
    ((uint8_t*)mem->memory)[pos + 1] = (value >> 8) & 0xff;
    ((uint8_t*)mem->memory)[pos + 2] = (value >> 16) & 0xff;
    ((uint8_t*)mem->memory)[pos + 3] = (value >> 24) & 0xff;
    ((uint8_t*)mem->memory)[pos + 4] = (value >> 32) & 0xff;
    ((uint8_t*)mem->memory)[pos + 5] = (value >> 40) & 0xff;
    ((uint8_t*)mem->memory)[pos + 6] = (value >> 48) & 0xff;
    ((uint8_t*)mem->memory)[pos + 7] = (value >> 56) & 0xff;
}

#else

#error The target architecture is not supported

#endif