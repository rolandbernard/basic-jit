
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

void addInstMovRegToReg(StackAllocator* mem, RegisterSet regs, Register dest, Register src) {
    addMovRegToReg(mem, dest, src);
}

void addInstMovImmToReg(StackAllocator* mem, RegisterSet regs, Register reg, int64_t value) {
    if(value >> 32 == 0 || value >> 32 == -1) {
        addMovImm32ToReg(mem, reg, value);
    } else {
        addMovImm64ToReg(mem, reg, value);
    }
}

void addInstMovMemToReg(StackAllocator* mem, RegisterSet regs, Register reg, void* addr) {
    addMovImm64ToReg(mem, reg, (uint64_t)addr);
    addMovMemRegToReg(mem, reg, reg);
}

void addInstMovRegToMem(StackAllocator* mem, RegisterSet regs, Register reg, void* addr) {
    int free_reg = getFreeRegister(regs);
    if(free_reg == 0) {
        addPush(mem, REG_A);
        addMovImm64ToReg(mem, REG_A, (uint64_t)addr);
        addMovRegToMemReg(mem, reg, reg);
        addPop(mem, REG_A);
    } else {
        addMovImm64ToReg(mem, free_reg, (uint64_t)addr);
        addMovRegToMemReg(mem, free_reg, reg);
    }
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

void addInstJmpRel(StackAllocator* mem, RegisterSet regs, int32_t to) {
    addJmpRelative32(mem, to);
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
    for(int i = 0; i < REG_COUNT + FREG_COUNT; i++) {
        addInstPush(mem, regs, (1 << i));
    }
}

void addInstPopAll(StackAllocator* mem, RegisterSet regs) {
    for(int i = REG_COUNT + FREG_COUNT - 1; i >= 0; i--) {
        addInstPop(mem, regs, (1 << i));
    }
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
        addSub(mem, dest, a);
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
            addIDiv(mem, free_reg);
            if(dest != REG_A) {
                addMovRegToReg(mem, dest, REG_A);
            }
        }
    } else {
        if(a != REG_A) {
            addMovRegToReg(mem, REG_A, a);
        }
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
            addIDiv(mem, free_reg);
            if(dest != REG_D) {
                addMovRegToReg(mem, dest, REG_D);
            }
        }
    } else {
        if(a != REG_A) {
            addMovRegToReg(mem, REG_A, a);
        }
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

void addInstCondJmpRel(StackAllocator* mem, RegisterSet regs, JmpCondistions cond, Register a, Register b, int32_t to) {
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
        addFSub(mem, dest, a);
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

void addInstFFrac(StackAllocator* mem, RegisterSet regs, Register dest, Register src) {
    int free_reg = getFreeFRegister(regs);
    if(free_reg == 0) {
        addPush(mem, FREG_0);
        addFRegCvtToInt(mem, FREG_0, src);
        addFRegCvtToInt(mem, FREG_0, FREG_0);
        if(dest != src) {
            addMovFRegToFReg(mem, dest, src);
        }
        addFSub(mem, dest, FREG_0);
        addPop(mem, FREG_0);
    } else {
        addFRegCvtToInt(mem, free_reg, src);
        addFRegCvtToInt(mem, free_reg, free_reg);
        if(dest != src) {
            addMovFRegToFReg(mem, dest, src);
        }
        addFSub(mem, dest, free_reg);
    }
}

void addInstFTrunc(StackAllocator* mem, RegisterSet regs, Register dest, Register src) {
    addFRegCvtToInt(mem, dest, src);
    addFRegCvtToFlt(mem, dest, dest);
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
        addMovFRegToMemReg(mem, REG_A, reg);
    }
}

void addInstMovFRegToFReg(StackAllocator* mem, RegisterSet regs, Register dest, Register src) {
    addMovFRegToFReg(mem, dest, src);
}

#else

#error The target architecture is not supported

#endif