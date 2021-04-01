#ifdef __x86_64__

#include <stdbool.h>

#include "codegen/instructions.h"

#include "codegen/x86_64/x86_64.h"

#define ARRAY_LEN(X) (sizeof(X)/sizeof(X[0]))

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
    if (dest != src) {
        addMovRegToReg(mem, dest, src);
    }
}

size_t addInstMovImmToReg(StackAllocator* mem, RegisterSet regs, Register reg, int64_t value) {
    addMovImm64ToReg(mem, reg, value);
    return mem->occupied - 8;
}

void addInstMovMemToReg(StackAllocator* mem, RegisterSet regs, Register reg, void* addr) {
    addMovImm64ToReg(mem, reg, (uint64_t)addr);
    addMovMemRegToReg(mem, reg, reg);
}

void addInstMovRegToMem(StackAllocator* mem, RegisterSet regs, Register reg, void* addr) {
    int free_reg = getFreeRegister(regs);
    if(free_reg == 0) {
        if (reg == REG_A) {
            free_reg = REG_B;
        } else {
            free_reg = REG_A;
        }
        addPush(mem, free_reg);
        addMovImm64ToReg(mem, REG_A, (uint64_t)addr);
        addMovRegToMemReg(mem, reg, reg);
        addPop(mem, free_reg);
    } else {
        addMovImm64ToReg(mem, free_reg, (uint64_t)addr);
        addMovRegToMemReg(mem, free_reg, reg);
    }
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

size_t addInstJmpRel(StackAllocator* mem, RegisterSet regs, size_t to) {
    uint32_t rel = to - (mem->occupied + 5);
    addJmpRelative32(mem, rel);
    return mem->occupied - 4;
}

void addInstPush(StackAllocator* mem, RegisterSet regs, Register reg) {
    static uint64_t scratch;
    if(reg >= (1 << REG_COUNT)) {
        int free_reg = getFreeRegister(regs);
        if(free_reg == 0) {
            addInstMovRegToMem(mem, regs, REG_A, &scratch);
            addMovFRegToReg(mem, REG_A, reg);
            addPush(mem, REG_A);
            addInstMovMemToReg(mem, regs, REG_A, &scratch);
        } else {
            addMovFRegToReg(mem, free_reg, reg);
            addPush(mem, free_reg);
        }
    } else {
        addPush(mem, reg);
    }
}

void addInstPop(StackAllocator* mem, RegisterSet regs, Register reg) {
    if (reg == 0) {
        reg = getFreeRegister(regs);
    }
    static uint64_t scratch;
    if(reg >= (1 << REG_COUNT)) {
        int free_reg = getFreeRegister(regs);
        if(free_reg == 0) {
            addInstMovRegToMem(mem, regs, REG_A, &scratch);
            addPop(mem, REG_A);
            addMovRegToFReg(mem, reg, REG_A);
            addInstMovMemToReg(mem, regs, REG_A, &scratch);
        } else {
            addPop(mem, free_reg);
            addMovRegToFReg(mem, reg, free_reg);
        }
    } else {
        addPop(mem, reg);
    }
}

void addInstPushAll(StackAllocator* mem, RegisterSet regs, RegisterSet to_push) {
    int push_cnt = 0;
    Register double_pop = 0;
    for(int i = 0; i < REG_COUNT + FREG_COUNT; i++) {
        if((to_push & (1 << i)) != 0) {
            push_cnt++;
            double_pop = (1 << i); 
            addInstPush(mem, regs, (1 << i));
        }
    }
    if(push_cnt % 2 == 1) {
        addPush(mem, double_pop);
    }
}

void addInstPopAll(StackAllocator* mem, RegisterSet regs, RegisterSet to_pop) {
    int pop_cnt = 0;
    Register double_pop = 0;
    for(int i = REG_COUNT + FREG_COUNT - 1; i >= 0; i--) {
        if((to_pop & (1 << i)) != 0) {
            pop_cnt++;
            double_pop = (1 << i); 
        }
    }
    if(pop_cnt % 2 == 1) {
        addPop(mem, double_pop);
    }
    for(int i = REG_COUNT + FREG_COUNT - 1; i >= 0; i--) {
        if((to_pop & (1 << i)) != 0) {
            addInstPop(mem, regs, (1 << i));
        }
    }
}

size_t addInstCallRel(StackAllocator* mem, RegisterSet regs, size_t to) {
    uint32_t rel = to - (mem->occupied + 5);
    addCallRel(mem, rel);
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
    if (a == dest) {
        addSub(mem, dest, b);
    } else if (b == dest) {
        int free_reg = getFreeRegister(regs);
        if (free_reg == 0) {
            if (a != REG_A && b != REG_A && dest != REG_A) {
                free_reg = REG_A;
            } else if (a != REG_B && b != REG_B && dest != REG_B) {
                free_reg = REG_B;
            } else if (a != REG_C && b != REG_C && dest != REG_C) {
                free_reg = REG_B;
            } else {
                free_reg = REG_D;
            }
            addPush(mem, free_reg);
            addMovRegToReg(mem, free_reg, b);
            addMovRegToReg(mem, dest, a);
            addSub(mem, dest, REG_A);
            addPop(mem, free_reg);
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
    if (a == REG_D) {
        addPush(mem, a);
        addPush(mem, b);
        Register tmp = a;
        a = b;
        b = tmp;
        addPop(mem, b);
        addPop(mem, a);
    }
    RegisterSet internal_regs = regs | REG_A | REG_D;
    if(b == REG_D || b == REG_A) {
        int free_reg = getFreeRegister(internal_regs);
        if(free_reg == 0) {
            if (a != REG_B && b != REG_B && dest != REG_B) {
                free_reg = REG_B;
            } else if (a != REG_C && b != REG_C && dest != REG_C) {
                free_reg = REG_C;
            } else if (a != REG_DI && b != REG_DI && dest != REG_DI) {
                free_reg = REG_DI;
            } else {
                free_reg = REG_SI;
            }
            addPush(mem, free_reg);
            addMovRegToReg(mem, free_reg, b);
            addInstMovImmToReg(mem, internal_regs, REG_D, 1);
            if(a != REG_A) {
                addMovRegToReg(mem, REG_A, a);
            }
            addIMulRax(mem, REG_D);
            addIDiv(mem, free_reg);
            if(dest != REG_A) {
                addMovRegToReg(mem, dest, REG_A);
            }
            addPop(mem, free_reg);
        } else {
            internal_regs |= free_reg;
            addMovRegToReg(mem, free_reg, b);
            addInstMovImmToReg(mem, internal_regs, REG_D, 1);
            if(a != REG_A) {
                addMovRegToReg(mem, REG_A, a);
            }
            addIMulRax(mem, REG_D);
            addIDiv(mem, free_reg);
            if(dest != REG_A) {
                addMovRegToReg(mem, dest, REG_A);
            }
        }
    } else {
        addInstMovImmToReg(mem, internal_regs, REG_D, 1);
        if(a != REG_A) {
            addMovRegToReg(mem, REG_A, a);
        }
        addIMulRax(mem, REG_D);
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
    if (a == REG_D) {
        addPush(mem, a);
        addPush(mem, b);
        Register tmp = a;
        a = b;
        b = tmp;
        addPop(mem, b);
        addPop(mem, a);
    }
    RegisterSet internal_regs = regs | REG_A | REG_D;
    if(b == REG_D || b == REG_A) {
        int free_reg = getFreeRegister(internal_regs);
        if(free_reg == 0) {
            if (a != REG_B && b != REG_B && dest != REG_B) {
                free_reg = REG_B;
            } else if (a != REG_C && b != REG_C && dest != REG_C) {
                free_reg = REG_C;
            } else if (a != REG_DI && b != REG_DI && dest != REG_DI) {
                free_reg = REG_DI;
            } else {
                free_reg = REG_SI;
            }
            addPush(mem, free_reg);
            addMovRegToReg(mem, free_reg, b);
            addInstMovImmToReg(mem, internal_regs, REG_D, 1);
            if(a != REG_A) {
                addMovRegToReg(mem, REG_A, a);
            }
            addIMulRax(mem, REG_D);
            addIDiv(mem, free_reg);
            if(dest != REG_D) {
                addMovRegToReg(mem, dest, REG_D);
            }
            addPop(mem, free_reg);
        } else {
            internal_regs |= free_reg;
            addMovRegToReg(mem, free_reg, b);
            addInstMovImmToReg(mem, internal_regs, REG_D, 1);
            if(a != REG_A) {
                addMovRegToReg(mem, REG_A, a);
            }
            addIMulRax(mem, REG_D);
            addIDiv(mem, free_reg);
            if(dest != REG_D) {
                addMovRegToReg(mem, dest, REG_D);
            }
        }
    } else {
        addInstMovImmToReg(mem, internal_regs, REG_D, 1);
        if(a != REG_A) {
            addMovRegToReg(mem, REG_A, a);
        }
        addIMulRax(mem, REG_D);
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

void addInstAnd(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b) {
    if(a == dest) {
        addAnd(mem, dest, b);
    } else if(b == dest) {
        addAnd(mem, dest, a);
    } else {
        addMovRegToReg(mem, dest, a);
        addAnd(mem, dest, b);
    }
}

void addInstXor(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b) {
    if(a == dest) {
        addXor(mem, dest, b);
    } else if(b == dest) {
        addXor(mem, dest, a);
    } else {
        addMovRegToReg(mem, dest, a);
        addXor(mem, dest, b);
    }
}

void addInstOr(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b) {
    if(a == dest) {
        addOr(mem, dest, b);
    } else if(b == dest) {
        addOr(mem, dest, a);
    } else {
        addMovRegToReg(mem, dest, a);
        addOr(mem, dest, b);
    }
}

void addInstNot(StackAllocator* mem, RegisterSet regs, Register dest, Register a) {
    if(a == dest) {
        addNot(mem, dest);
    } else {
        addMovRegToReg(mem, dest, a);
        addNot(mem, dest);
    }
}

size_t addInstCondJmpRel(StackAllocator* mem, RegisterSet regs, JmpCondistions cond, Register a, Register b, size_t to) {
    if(a < (1 << REG_COUNT)) {
        addCmp(mem, a, b);
        uint32_t rel = to - (mem->occupied + 6);
        switch(cond) {
            case COND_EQ:
                addJmpEQ(mem, rel);
                break;
            case COND_NE:
                addJmpNE(mem, rel);
                break;
            case COND_GT:
                addJmpGT(mem, rel);
                break;
            case COND_LT:
                addJmpLT(mem, rel);
                break;
            case COND_GE:
                addJmpGE(mem, rel);
                break;
            case COND_LE:
                addJmpLE(mem, rel);
                break;
        }
    } else {
        addFCom(mem, a, b);
        uint32_t rel = to - (mem->occupied + 6);
        switch(cond) {
            case COND_EQ:
                addJmpEQ(mem, rel);
                break;
            case COND_NE:
                addJmpNE(mem, rel);
                break;
            case COND_GT:
                addJmpA(mem, rel);
                break;
            case COND_LT:
                addJmpB(mem, rel);
                break;
            case COND_GE:
                addJmpAE(mem, rel);
                break;
            case COND_LE:
                addJmpBE(mem, rel);
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
    if (dest != src) {
        addMovFRegToFReg(mem, dest, src);
    }
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

static const Register call_regs[] = {
    REG_DI, REG_SI, REG_D, REG_C, REG_8, REG_9,
};

static const Register call_fregs[] = {
    FREG_0, FREG_1, FREG_2, FREG_3, FREG_4, FREG_5, FREG_6, FREG_7,
};

void addInstFunctionCall(StackAllocator* mem, RegisterSet regs, Register ret, int arg_count, Register* args, void* func) {
    bool in_register[arg_count + 1];
    addInstPushAll(mem, regs, regs & ~ret);
    RegisterSet all_regs = 0;
    RegisterSet tmp_regs = 0;
    size_t int_count = 0;
    size_t float_count = 0;
    int stack_size = 0;
    for (int i = 0; i < arg_count; i++) {
        if (args[i] >= (1 << REG_COUNT)) {
            in_register[i] = (float_count < ARRAY_LEN(call_fregs));
            float_count++;
        } else {
            in_register[i] = (int_count < ARRAY_LEN(call_regs));
            int_count++;
        }
        if (in_register[i]) {
            tmp_regs |= args[i];
        } else {
            stack_size++;
        }
        all_regs |= args[i];
    }
    if (stack_size % 2 == 1) {
        addInstPush(mem, all_regs, REG_A);
    }
    for (int i = arg_count - 1; i >= 0; i--) {
        if (!in_register[i]) {
            addInstPush(mem, all_regs, args[i]);
        }
    }
    float_count = 0;
    int_count = 0;
    for (int i = 0; i < arg_count; i++) {
        if (in_register[i]) {
            if (args[i] >= (1 << REG_COUNT)) {
                Register to_use = call_fregs[float_count];
                tmp_regs |= to_use;
                if (args[i] != to_use) {
                    Register tmp = getFreeFRegister(tmp_regs);
                    for (int j = i + 1; j < arg_count; j++) {
                        if (args[j] == to_use) {
                            addInstMovFRegToFReg(mem, tmp_regs, tmp, args[j]);
                            args[j] = tmp;
                            tmp_regs |= tmp;
                        }
                    }
                    addInstMovFRegToFReg(mem, tmp_regs, to_use, args[i]);
                    tmp_regs &= ~args[i];
                }
                float_count++;
            } else {
                Register to_use = call_regs[int_count];
                tmp_regs |= to_use;
                if (args[i] != to_use) {
                    Register tmp = getFreeRegister(tmp_regs);
                    for (int j = i + 1; j < arg_count; j++) {
                        if (args[j] == to_use) {
                            addInstMovRegToReg(mem, tmp_regs, tmp, args[j]);
                            args[j] = tmp;
                            tmp_regs |= tmp;
                        }
                    }
                    addInstMovRegToReg(mem, tmp_regs, to_use, args[i]);
                    tmp_regs &= ~args[i];
                }
                int_count++;
            }
        }
    }
    addInstMovImmToReg(mem, tmp_regs, REG_A, float_count);
    addInstCall(mem, tmp_regs | REG_A, func);
    if (ret != 0) {
        if (ret >= (1 << REG_COUNT)) {
            if (ret != FREG_0) {
                addMovFRegToFReg(mem, ret, FREG_0);
            }
        } else {
            if (ret != REG_A) {
                addMovRegToReg(mem, ret, REG_A);
            }
        }
    }
    if (stack_size % 2 == 1) {
        addInstPop(mem, ret, 0);
    }
    for (int i = arg_count - 1; i >= 0; i--) {
        if (!in_register[i]) {
            addInstPop(mem, ret, 0);
        }
    }
    addInstPopAll(mem, regs, regs & ~ret);
}

void addInstFunctionCallUnary(StackAllocator* mem, RegisterSet regs, Register ret, Register a, void* func) {
    Register args[1] = { a };
    addInstFunctionCall(mem, regs, ret, 1, args, func);
}

void addInstFunctionCallBinary(StackAllocator* mem, RegisterSet regs, Register ret, Register a, Register b, void* func) {
    Register args[2] = { a, b };
    addInstFunctionCall(mem, regs, ret, 2, args, func);
}

void addInstFunctionCallUnaryNoRet(StackAllocator* mem, RegisterSet regs, Register a, void* func) {
    Register args[1] = { a };
    addInstFunctionCall(mem, regs, 0, 1, args, func);
}

void addInstFunctionCallRetOnly(StackAllocator* mem, RegisterSet regs, Register ret, void* func) {
    addInstFunctionCall(mem, regs, ret, 0, NULL, func);
}

void addInstFunctionCallSimple(StackAllocator* mem, RegisterSet regs, void* func) {
    addInstFunctionCall(mem, regs, 0, 0, NULL, func);
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

void updateRelativeJumpTarget(StackAllocator* mem, size_t pos, size_t to) {
    uint32_t rel = to - (pos + 4);
    ((uint8_t*)mem->memory)[pos] = rel & 0xff;
    ((uint8_t*)mem->memory)[pos + 1] = (rel >> 8) & 0xff;
    ((uint8_t*)mem->memory)[pos + 2] = (rel >> 16) & 0xff;
    ((uint8_t*)mem->memory)[pos + 3] = (rel >> 24) & 0xff;
}

void updateImmediateValue(StackAllocator* mem, size_t pos, int64_t value) {
    ((uint8_t*)mem->memory)[pos] = value & 0xff;
    ((uint8_t*)mem->memory)[pos + 1] = (value >> 8) & 0xff;
    ((uint8_t*)mem->memory)[pos + 2] = (value >> 16) & 0xff;
    ((uint8_t*)mem->memory)[pos + 3] = (value >> 24) & 0xff;
    ((uint8_t*)mem->memory)[pos + 4] = (value >> 32) & 0xff;
    ((uint8_t*)mem->memory)[pos + 5] = (value >> 40) & 0xff;
    ((uint8_t*)mem->memory)[pos + 6] = (value >> 48) & 0xff;
    ((uint8_t*)mem->memory)[pos + 7] = (value >> 56) & 0xff;
}

#endif
