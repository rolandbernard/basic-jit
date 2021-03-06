#ifdef __aarch64__

#include "codegen/instructions.h"
#include "codegen/aarch64/aarch64.h"

static int regToNo(Register reg) {
    for(int i = 0; i < REG_COUNT; i++) {
        if((reg & REG_X(i)) != 0) {
            return i;
        }
    }
    for(int i = 0; i < FREG_COUNT; i++) {
        if((reg & REG_D(i)) != 0) {
            return i;
        }
    }
    return 31;
}

uint64_t getFreeRegister(RegisterSet regs) {
    for(int i = 0; i < USER_REG_COUNT; i++) {
        if((regs & REG_X(i)) == 0) {
            return REG_X(i);
        }
    }
    return 0;
}

uint64_t getFreeFRegister(RegisterSet regs) {
    for(int i = 0; i < FREG_COUNT; i++) {
        if((regs & REG_D(i)) == 0) {
            return REG_D(i);
        }
    }
    return 0;
}

uint64_t getUsedRegister(RegisterSet regs) {
    for(int i = 0; i < USER_REG_COUNT; i++) {
        if((regs & REG_X(i)) != 0) {
            return REG_X(i);
        }
    }
    return 0;
}

uint64_t getUsedFRegister(RegisterSet regs) {
    for(int i = 0; i < FREG_COUNT; i++) {
        if((regs & REG_D(i)) != 0) {
            return REG_D(i);
        }
    }
    return 0;
}

int countFreeRegister(RegisterSet regs) {
    int ret = 0;
    for(int i = 0; i < USER_REG_COUNT; i++) {
        if((regs & REG_X(i)) == 0) {
            ret++;
        }
    }
    return ret;
}

int countFreeFRegister(RegisterSet regs) {
    int ret = 0;
    for(int i = 0; i < FREG_COUNT; i++) {
        if((regs & REG_D(i)) == 0) {
            ret++;
        }
    }
    return ret;
}

uint64_t getFirstRegister() {
    return REG_X(0);
}

uint64_t getFirstFRegister() {
    return REG_D(0);
}

void addInstMovRegToReg(StackAllocator* mem, RegisterSet regs, Register dest, Register src) {
    if (dest != src) {
        Aarch64Instruction instr = { .instruction = 0, };
        instr.logical_shift_reg.cnst0 = LOGICAL_SHIFT_REG_CNST0;
        instr.logical_shift_reg.opc = LOGICAL_SHIFT_REG_OPC_ORR;
        instr.logical_shift_reg.sf = 1;
        instr.logical_shift_reg.rd = regToNo(dest);
        instr.logical_shift_reg.rn = REG_SPECIAL;
        instr.logical_shift_reg.rm = regToNo(src);
        addInstruction(mem, instr);
    }
}

size_t addInstMovImmToReg(StackAllocator* mem, RegisterSet regs, Register reg, int64_t value) {
    size_t ret = mem->occupied;
    for (int i = 0; i < 4; i++) {
        Aarch64Instruction instr = { .instruction = 0, };
        instr.move_imm.cnst0 = MOVE_IMM_CNST0;
        instr.move_imm.opc = MOVE_IMM_OPC_MOVK;
        instr.move_imm.sf = 1;
        instr.move_imm.hw = i;
        instr.move_imm.rd = regToNo(reg);
        instr.move_imm.imm16 = (value >> (16 * i)) & 0xffff;
        addInstruction(mem, instr);
    }
    return ret;
}

void addInstMovMemToReg(StackAllocator* mem, RegisterSet regs, Register reg, void* addr) {
    addInstMovImmToReg(mem, regs, reg, (int64_t)addr);
    addInstMovMemRegToReg(mem, regs, reg, reg);
}

void addInstMovRegToMem(StackAllocator* mem, RegisterSet regs, Register reg, void* addr) {
    int free_reg = getFreeRegister(regs);
    if(free_reg == 0) {
        if (reg == REG_X(0)) {
            free_reg = REG_X(1);
        } else {
            free_reg = REG_X(0);
        }
        addInstPush(mem, regs, free_reg);
        addInstMovImmToReg(mem, regs, free_reg, (int64_t)addr);
        addInstMovRegToMemReg(mem, regs, free_reg, reg);
        addInstPop(mem, regs, free_reg);
    } else {
        addInstMovImmToReg(mem, regs, free_reg, (int64_t)addr);
        addInstMovRegToMemReg(mem, regs, free_reg, reg);
    }
}

void addInstMovMemRegToReg(StackAllocator* mem, RegisterSet regs, Register reg, Register addr) {
    Aarch64Instruction instr = { .instruction = 0, };
    instr.load_store_reg_unsi_imm.cnst0 = LOAD_STORE_REG_UNSI_IMM_CNST0;
    instr.load_store_reg_unsi_imm.cnst1 = LOAD_STORE_REG_UNSI_IMM_CNST1;
    instr.load_store_reg_unsi_imm.opc = LOAD_STORE_REG_UNSI_IMM_OPC_LDR;
    instr.load_store_reg_unsi_imm.size = LOAD_STORE_REG_UNSI_IMM_SIZE_DOUBLE;
    instr.load_store_reg_unsi_imm.v = 0;
    instr.load_store_reg_unsi_imm.imm12 = 0;
    instr.load_store_reg_unsi_imm.rn = regToNo(addr);
    instr.load_store_reg_unsi_imm.rt = regToNo(reg);
    addInstruction(mem, instr);
}

static void addInstMovRegToMemRegWithOff(StackAllocator* mem, RegisterSet regs, Register addr, Register reg, int off) {
    Aarch64Instruction instr = { .instruction = 0, };
    instr.load_store_reg_unsi_imm.cnst0 = LOAD_STORE_REG_UNSI_IMM_CNST0;
    instr.load_store_reg_unsi_imm.cnst1 = LOAD_STORE_REG_UNSI_IMM_CNST1;
    instr.load_store_reg_unsi_imm.opc = LOAD_STORE_REG_UNSI_IMM_OPC_STR;
    instr.load_store_reg_unsi_imm.size = LOAD_STORE_REG_UNSI_IMM_SIZE_DOUBLE;
    instr.load_store_reg_unsi_imm.v = 0;
    instr.load_store_reg_unsi_imm.imm12 = off;
    instr.load_store_reg_unsi_imm.rn = regToNo(addr);
    instr.load_store_reg_unsi_imm.rt = regToNo(reg);
    addInstruction(mem, instr);
}

void addInstMovRegToMemReg(StackAllocator* mem, RegisterSet regs, Register addr, Register reg) {
    addInstMovRegToMemRegWithOff(mem, regs, addr, reg, 0);
}

void addInstJmpReg(StackAllocator* mem, RegisterSet regs, Register reg) {
    Aarch64Instruction instr = { .instruction = 0, };
    instr.uncond_branch_reg.cnst0 = UNCOND_BRANCH_REG_CNST0;
    instr.uncond_branch_reg.op2 = UNCOND_BRANCH_REG_OP2_BR_BLR_RET;
    instr.uncond_branch_reg.op3 = UNCOND_BRANCH_REG_OP3_BR_BLR_RET;
    instr.uncond_branch_reg.op4 = UNCOND_BRANCH_REG_OP4_BR_BLR_RET;
    instr.uncond_branch_reg.opc = UNCOND_BRANCH_REG_OPC_BR;
    instr.uncond_branch_reg.rn = regToNo(reg);
    addInstruction(mem, instr);
}

void addInstJmp(StackAllocator* mem, RegisterSet regs, void* to) {
    int free_reg = getFreeRegister(regs);
    if(free_reg == 0) {
        free_reg = getFirstRegister();
        addInstPush(mem, regs, free_reg);
        addInstMovImmToReg(mem, regs, free_reg, (int64_t)to);
        addInstJmpReg(mem, regs, free_reg);
        addInstPop(mem, regs, free_reg);
    } else {
        addInstMovImmToReg(mem, regs, free_reg, (int64_t)to);
        addInstJmpReg(mem, regs, free_reg);
    }
}

size_t addInstJmpRel(StackAllocator* mem, RegisterSet regs, size_t to) {
    size_t ret = mem->occupied;
    uint32_t rel = to - mem->occupied;
    Aarch64Instruction instr = { .instruction = 0, };
    instr.uncond_branch_imm.cnst0 = UNCOND_BRANCH_IMM_CNST0;
    instr.uncond_branch_imm.op = UNCOND_BRANCH_IMM_OP_B;
    instr.uncond_branch_imm.imm26 = rel >> 2;
    addInstruction(mem, instr);
    return ret;
}

void addInstPush(StackAllocator* mem, RegisterSet regs, Register reg) {
    Aarch64Instruction instr = { .instruction = 0, };
    instr.load_store_reg_imm_pre.cnst0 = LOAD_STORE_REG_IMM_PRE_CNST0;
    instr.load_store_reg_imm_pre.cnst1 = LOAD_STORE_REG_IMM_PRE_CNST1;
    instr.load_store_reg_imm_pre.cnst2 = LOAD_STORE_REG_IMM_PRE_CNST2;
    instr.load_store_reg_imm_pre.cnst3 = LOAD_STORE_REG_IMM_PRE_CNST3;
    instr.load_store_reg_imm_pre.opc = LOAD_STORE_REG_IMM_PRE_OPC_STR;
    instr.load_store_reg_imm_pre.size = LOAD_STORE_REG_IMM_PRE_SIZE_DOUBLE;
    instr.load_store_reg_imm_pre.rn = REG_SPECIAL;
    instr.load_store_reg_imm_pre.rt = regToNo(reg);
    if(reg >= REG_D(0)) {
        instr.load_store_reg_imm_post.v = 1;
    } else {
        instr.load_store_reg_imm_post.v = 0;
    }
    instr.load_store_reg_imm_pre.imm9 = -16;
    addInstruction(mem, instr);
}

void addInstPop(StackAllocator* mem, RegisterSet regs, Register reg) {
    Aarch64Instruction instr = { .instruction = 0, };
    instr.load_store_reg_imm_post.cnst0 = LOAD_STORE_REG_IMM_POST_CNST0;
    instr.load_store_reg_imm_post.cnst1 = LOAD_STORE_REG_IMM_POST_CNST1;
    instr.load_store_reg_imm_post.cnst2 = LOAD_STORE_REG_IMM_POST_CNST2;
    instr.load_store_reg_imm_post.cnst3 = LOAD_STORE_REG_IMM_POST_CNST3;
    instr.load_store_reg_imm_post.opc = LOAD_STORE_REG_IMM_POST_OPC_LDR;
    instr.load_store_reg_imm_post.size = LOAD_STORE_REG_IMM_POST_SIZE_DOUBLE;
    instr.load_store_reg_imm_post.rn = REG_SPECIAL;
    instr.load_store_reg_imm_post.rt = regToNo(reg);
    if(reg >= REG_D(0)) {
        instr.load_store_reg_imm_post.v = 1;
    } else {
        instr.load_store_reg_imm_post.v = 0;
    }
    instr.load_store_reg_imm_post.imm9 = 16;
    addInstruction(mem, instr);
}

void addInstPushAll(StackAllocator* mem, RegisterSet regs, RegisterSet to_push) {
    for(int i = 0; i < REG_COUNT; i++) {
        if((to_push & REG_X(i)) != 0) {
            addInstPush(mem, regs, REG_X(i));
        }
    }
    for(int i = 0; i < FREG_COUNT; i++) {
        if((to_push & REG_D(i)) != 0) {
            addInstPush(mem, regs, REG_D(i));
        }
    }
}

void addInstPopAll(StackAllocator* mem, RegisterSet regs, RegisterSet to_pop) {
    for(int i = FREG_COUNT - 1; i >= 0; i--) {
        if((to_pop & REG_D(i)) != 0) {
            addInstPop(mem, regs, REG_D(i));
        }
    }
    for(int i = REG_COUNT - 1; i >= 0; i--) {
        if((to_pop & REG_X(i)) != 0) {
            addInstPop(mem, regs, REG_X(i));
        }
    }
}

void addInstCallBasicReg(StackAllocator* mem, RegisterSet regs, Register reg) {
    Aarch64Instruction instr = { .instruction = 0, };
    instr.uncond_branch_reg.cnst0 = UNCOND_BRANCH_REG_CNST0;
    instr.uncond_branch_reg.op2 = UNCOND_BRANCH_REG_OP2_BR_BLR_RET;
    instr.uncond_branch_reg.op3 = UNCOND_BRANCH_REG_OP3_BR_BLR_RET;
    instr.uncond_branch_reg.op4 = UNCOND_BRANCH_REG_OP4_BR_BLR_RET;
    instr.uncond_branch_reg.opc = UNCOND_BRANCH_REG_OPC_BLR;
    instr.uncond_branch_reg.rn = regToNo(reg);
    addInstruction(mem, instr);
}

void addInstCallBasic(StackAllocator* mem, RegisterSet regs, void* func) {
    int free_reg = getFreeRegister(regs);
    if(free_reg == 0) {
        free_reg = getFirstRegister();
        addInstPush(mem, regs, free_reg);
        addInstMovImmToReg(mem, regs, free_reg, (int64_t)func);
        addInstCallBasicReg(mem, regs, free_reg);
        addInstPop(mem, regs, free_reg);
    } else {
        addInstMovImmToReg(mem, regs, free_reg, (int64_t)func);
        addInstCallBasicReg(mem, regs, free_reg);
    }
}

void addInstCallReg(StackAllocator* mem, RegisterSet regs, Register reg) {
    addInstPush(mem, regs, REG_X(REG_LINK));
    Aarch64Instruction instr = { .instruction = 0, };
    instr.uncond_branch_reg.cnst0 = UNCOND_BRANCH_REG_CNST0;
    instr.uncond_branch_reg.op2 = UNCOND_BRANCH_REG_OP2_BR_BLR_RET;
    instr.uncond_branch_reg.op3 = UNCOND_BRANCH_REG_OP3_BR_BLR_RET;
    instr.uncond_branch_reg.op4 = UNCOND_BRANCH_REG_OP4_BR_BLR_RET;
    instr.uncond_branch_reg.opc = UNCOND_BRANCH_REG_OPC_BLR;
    instr.uncond_branch_reg.rn = regToNo(reg);
    addInstruction(mem, instr);
    addInstPop(mem, regs, REG_X(REG_LINK));
}

void addInstCall(StackAllocator* mem, RegisterSet regs, void* func) {
    int free_reg = getFreeRegister(regs);
    if(free_reg == 0) {
        free_reg = getFirstRegister();
        addInstPush(mem, regs, free_reg);
        addInstMovImmToReg(mem, regs, free_reg, (int64_t)func);
        addInstCallReg(mem, regs, free_reg);
        addInstPop(mem, regs, free_reg);
    } else {
        addInstMovImmToReg(mem, regs, free_reg, (int64_t)func);
        addInstCallReg(mem, regs, free_reg);
    }
}

size_t addInstCallRel(StackAllocator* mem, RegisterSet regs, size_t to) {
    addInstPush(mem, regs, REG_X(REG_LINK));
    size_t ret = mem->occupied;
    uint32_t rel = to - mem->occupied;
    Aarch64Instruction instr = { .instruction = 0, };
    instr.uncond_branch_imm.cnst0 = UNCOND_BRANCH_IMM_CNST0;
    instr.uncond_branch_imm.op = UNCOND_BRANCH_IMM_OP_BL;
    instr.uncond_branch_imm.imm26 = rel >> 2;
    addInstruction(mem, instr);
    addInstPop(mem, regs, REG_X(REG_LINK));
    return ret;
}

void addInstReturn(StackAllocator* mem, RegisterSet regs) {
    Aarch64Instruction instr = { .instruction = 0, };
    instr.uncond_branch_reg.cnst0 = UNCOND_BRANCH_REG_CNST0;
    instr.uncond_branch_reg.op2 = UNCOND_BRANCH_REG_OP2_BR_BLR_RET;
    instr.uncond_branch_reg.op3 = UNCOND_BRANCH_REG_OP3_BR_BLR_RET;
    instr.uncond_branch_reg.op4 = UNCOND_BRANCH_REG_OP4_BR_BLR_RET;
    instr.uncond_branch_reg.opc = UNCOND_BRANCH_REG_OPC_RET;
    instr.uncond_branch_reg.rn = REG_LINK;
    addInstruction(mem, instr);
}

void addInstAdd(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b) {
    Aarch64Instruction instr = { .instruction = 0, };
    instr.add_shift_reg.cnst0 = ADD_SHIFT_REG_CNST0;
    instr.add_shift_reg.cnst1 = ADD_SHIFT_REG_CNST1;
    instr.add_shift_reg.op = ADD_SHIFT_REG_OP_ADD;
    instr.add_shift_reg.s = 0;
    instr.add_shift_reg.sf = 1;
    instr.add_shift_reg.imm6 = 0;
    instr.add_shift_reg.shift = 0;
    instr.add_shift_reg.rn = regToNo(a);
    instr.add_shift_reg.rm = regToNo(b);
    instr.add_shift_reg.rd = regToNo(dest);
    addInstruction(mem, instr);
}

void addInstSub(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b) {
    Aarch64Instruction instr = { .instruction = 0, };
    instr.add_shift_reg.cnst0 = ADD_SHIFT_REG_CNST0;
    instr.add_shift_reg.cnst1 = ADD_SHIFT_REG_CNST1;
    instr.add_shift_reg.op = ADD_SHIFT_REG_OP_SUB;
    instr.add_shift_reg.s = 0;
    instr.add_shift_reg.sf = 1;
    instr.add_shift_reg.imm6 = 0;
    instr.add_shift_reg.shift = 0;
    instr.add_shift_reg.rn = regToNo(a);
    instr.add_shift_reg.rm = regToNo(b);
    instr.add_shift_reg.rd = regToNo(dest);
    addInstruction(mem, instr);
}

void addInstAnd(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b) {
    Aarch64Instruction instr = { .instruction = 0, };
    instr.logical_shift_reg.cnst0 = LOGICAL_SHIFT_REG_CNST0;
    instr.logical_shift_reg.opc = LOGICAL_SHIFT_REG_OPC_AND;
    instr.logical_shift_reg.sf = 1;
    instr.logical_shift_reg.shift = 0;
    instr.logical_shift_reg.imm6 = 0;
    instr.logical_shift_reg.n = 0;
    instr.logical_shift_reg.rn = regToNo(a);
    instr.logical_shift_reg.rm = regToNo(b);
    instr.logical_shift_reg.rd = regToNo(dest);
    addInstruction(mem, instr);
}

void addInstXor(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b) {
    Aarch64Instruction instr = { .instruction = 0, };
    instr.logical_shift_reg.cnst0 = LOGICAL_SHIFT_REG_CNST0;
    instr.logical_shift_reg.opc = LOGICAL_SHIFT_REG_OPC_EOR;
    instr.logical_shift_reg.sf = 1;
    instr.logical_shift_reg.shift = 0;
    instr.logical_shift_reg.imm6 = 0;
    instr.logical_shift_reg.n = 0;
    instr.logical_shift_reg.rn = regToNo(a);
    instr.logical_shift_reg.rm = regToNo(b);
    instr.logical_shift_reg.rd = regToNo(dest);
    addInstruction(mem, instr);
}

void addInstOr(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b) {
    Aarch64Instruction instr = { .instruction = 0, };
    instr.logical_shift_reg.cnst0 = LOGICAL_SHIFT_REG_CNST0;
    instr.logical_shift_reg.opc = LOGICAL_SHIFT_REG_OPC_ORR;
    instr.logical_shift_reg.sf = 1;
    instr.logical_shift_reg.shift = 0;
    instr.logical_shift_reg.imm6 = 0;
    instr.logical_shift_reg.n = 0;
    instr.logical_shift_reg.rn = regToNo(a);
    instr.logical_shift_reg.rm = regToNo(b);
    instr.logical_shift_reg.rd = regToNo(dest);
    addInstruction(mem, instr);
}

void addInstNot(StackAllocator* mem, RegisterSet regs, Register dest, Register a) {
    Aarch64Instruction instr = { .instruction = 0, };
    instr.logical_shift_reg.cnst0 = LOGICAL_SHIFT_REG_CNST0;
    instr.logical_shift_reg.opc = LOGICAL_SHIFT_REG_OPC_ORR;
    instr.logical_shift_reg.sf = 1;
    instr.logical_shift_reg.shift = 0;
    instr.logical_shift_reg.imm6 = 0;
    instr.logical_shift_reg.n = 1;
    instr.logical_shift_reg.rn = REG_SPECIAL;
    instr.logical_shift_reg.rm = regToNo(a);
    instr.logical_shift_reg.rd = regToNo(dest);
    addInstruction(mem, instr);
}

void addInstMul(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b) {
    Aarch64Instruction instr = { .instruction = 0, };
    instr.proc_reg_three_source.cnst0 = PROC_REG_THREE_SOURCE_CNST0;
    instr.proc_reg_three_source.op54 = PROC_REG_THREE_SOURCE_OP54_ANY;
    instr.proc_reg_three_source.op31 = PROC_REG_THREE_SOURCE_OP31_MADD_MSUB;
    instr.proc_reg_three_source.o0 = PROC_REG_THREE_SOURCE_O0_ADD;
    instr.proc_reg_three_source.sf = 1;
    instr.proc_reg_three_source.rd = regToNo(dest);
    instr.proc_reg_three_source.rn = regToNo(a);
    instr.proc_reg_three_source.rm = regToNo(b);
    instr.proc_reg_three_source.ra = REG_SPECIAL;
    addInstruction(mem, instr);
}

void addInstDiv(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b) {
    Aarch64Instruction instr = { .instruction = 0, };
    instr.proc_reg_two_source.cnst0 = PROC_REG_TWO_SOURCE_CNST0;
    instr.proc_reg_two_source.cnst1 = PROC_REG_TWO_SOURCE_CNST1;
    instr.proc_reg_two_source.opcode = PROC_REG_TWO_SOURCE_OPCODE_SDIV;
    instr.proc_reg_two_source.sf = 1;
    instr.proc_reg_two_source.s = 0;
    instr.proc_reg_two_source.rd = regToNo(dest);
    instr.proc_reg_two_source.rn = regToNo(a);
    instr.proc_reg_two_source.rm = regToNo(b);
    addInstruction(mem, instr);
}

void addInstMSub(StackAllocator* mem, RegisterSet regs, Register dest, Register n, Register m, Register a) {
    Aarch64Instruction instr = { .instruction = 0, };
    instr.proc_reg_three_source.cnst0 = PROC_REG_THREE_SOURCE_CNST0;
    instr.proc_reg_three_source.op54 = PROC_REG_THREE_SOURCE_OP54_ANY;
    instr.proc_reg_three_source.op31 = PROC_REG_THREE_SOURCE_OP31_MADD_MSUB;
    instr.proc_reg_three_source.o0 = PROC_REG_THREE_SOURCE_O0_SUB;
    instr.proc_reg_three_source.sf = 1;
    instr.proc_reg_three_source.rd = regToNo(dest);
    instr.proc_reg_three_source.rn = regToNo(n);
    instr.proc_reg_three_source.rm = regToNo(m);
    instr.proc_reg_three_source.ra = regToNo(a);
    addInstruction(mem, instr);
}

void addInstRem(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b) {
    int free_reg = getFreeRegister(regs);
    if(free_reg == 0) {
        if (a != REG_X(3) && b != REG_X(3) && dest != REG_X(3)) {
            free_reg = REG_X(3);
        } else if (a != REG_X(2) && b != REG_X(2) && dest != REG_X(2)) {
            free_reg = REG_X(2);
        } else if (a != REG_X(1) && b != REG_X(1) && dest != REG_X(1)) {
            free_reg = REG_X(1);
        } else {
            free_reg = REG_X(0);
        }
        addInstPush(mem, regs, free_reg);
        addInstDiv(mem, regs, free_reg, a, b);
        addInstMSub(mem, regs, dest, free_reg, b, a);
        addInstPop(mem, regs, free_reg);
    } else {
        addInstDiv(mem, regs, free_reg, a, b);
        addInstMSub(mem, regs, dest, free_reg, b, a);
    }
}


void addInstCmp(StackAllocator* mem, RegisterSet regs, Register a, Register b) {
    Aarch64Instruction instr = { .instruction = 0, };
    instr.add_shift_reg.cnst0 = ADD_SHIFT_REG_CNST0;
    instr.add_shift_reg.cnst1 = ADD_SHIFT_REG_CNST1;
    instr.add_shift_reg.op = ADD_SHIFT_REG_OP_SUB;
    instr.add_shift_reg.s = 1;
    instr.add_shift_reg.sf = 1;
    instr.add_shift_reg.imm6 = 0;
    instr.add_shift_reg.shift = 0;
    instr.add_shift_reg.rn = regToNo(a);
    instr.add_shift_reg.rm = regToNo(b);
    instr.add_shift_reg.rd = REG_SPECIAL;
    addInstruction(mem, instr);
}

void addInstFCmp(StackAllocator* mem, RegisterSet regs, Register a, Register b) {
    Aarch64Instruction instr = { .instruction = 0, };
    instr.fp_cmp.cnst0 = FP_CMP_CNST0;
    instr.fp_cmp.cnst1 = FP_CMP_CNST1;
    instr.fp_cmp.cnst2 = FP_CMP_CNST2;
    instr.fp_cmp.cnst3 = FP_CMP_CNST3;
    instr.fp_cmp.opcode2 = FP_CMP_OPCODE2_FCMP;
    instr.fp_cmp.ptype = FP_CMP_PTYPE_DOUBLE;
    instr.fp_cmp.m = 0;
    instr.fp_cmp.op = 0;
    instr.fp_cmp.s = 0;
    instr.fp_cmp.rn = regToNo(a);
    instr.fp_cmp.rm = regToNo(b);
    addInstruction(mem, instr);
}

size_t addInstCondJmpRel(StackAllocator* mem, RegisterSet regs, JmpCondistions cond, Register a, Register b, size_t to) {
    if(a < REG_D(0)) {
        addInstCmp(mem, regs, a, b);
    } else {
        addInstFCmp(mem, regs, a, b);
    }
    size_t ret = mem->occupied;
    uint32_t rel = to - mem->occupied;
    Aarch64Instruction instr = { .instruction = 0, };
    instr.cond_branch_imm.cnst0 = COND_BRANCH_IMM_CNST0;
    instr.cond_branch_imm.o0 = 0;
    instr.cond_branch_imm.o1 = 0;
    instr.cond_branch_imm.imm19 = rel >> 2;
    switch(cond) {
        case COND_EQ:
            instr.cond_branch_imm.cond = COND_BRANCH_IMM_COND_EQ;
            break;
        case COND_NE:
            instr.cond_branch_imm.cond = COND_BRANCH_IMM_COND_NE;
            break;
        case COND_GT:
            instr.cond_branch_imm.cond = COND_BRANCH_IMM_COND_GT;
            break;
        case COND_LT:
            instr.cond_branch_imm.cond = COND_BRANCH_IMM_COND_LT;
            break;
        case COND_GE:
            instr.cond_branch_imm.cond = COND_BRANCH_IMM_COND_GE;
            break;
        case COND_LE:
            instr.cond_branch_imm.cond = COND_BRANCH_IMM_COND_LE;
            break;
    }
    addInstruction(mem, instr);
    return ret;
}

void addInstFAdd(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b) {
    Aarch64Instruction instr = { .instruction = 0, };
    instr.proc_fp_two_source.cnst0 = PROC_FP_TWO_SOURCE_CNST0;
    instr.proc_fp_two_source.cnst1 = PROC_FP_TWO_SOURCE_CNST1;
    instr.proc_fp_two_source.cnst2 = PROC_FP_TWO_SOURCE_CNST2;
    instr.proc_fp_two_source.cnst3 = PROC_FP_TWO_SOURCE_CNST3;
    instr.proc_fp_two_source.m = 0;
    instr.proc_fp_two_source.s = 0;
    instr.proc_fp_two_source.ptype = PROC_FP_TWO_SOURCE_PTYPE_DOUBLE;
    instr.proc_fp_two_source.opcode = PROC_FP_TWO_SOURCE_OPCODE_FADD;
    instr.proc_fp_two_source.rd = regToNo(dest);
    instr.proc_fp_two_source.rn = regToNo(a);
    instr.proc_fp_two_source.rm = regToNo(b);
    addInstruction(mem, instr);
}

void addInstFSub(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b) {
    Aarch64Instruction instr = { .instruction = 0, };
    instr.proc_fp_two_source.cnst0 = PROC_FP_TWO_SOURCE_CNST0;
    instr.proc_fp_two_source.cnst1 = PROC_FP_TWO_SOURCE_CNST1;
    instr.proc_fp_two_source.cnst2 = PROC_FP_TWO_SOURCE_CNST2;
    instr.proc_fp_two_source.cnst3 = PROC_FP_TWO_SOURCE_CNST3;
    instr.proc_fp_two_source.m = 0;
    instr.proc_fp_two_source.s = 0;
    instr.proc_fp_two_source.ptype = PROC_FP_TWO_SOURCE_PTYPE_DOUBLE;
    instr.proc_fp_two_source.opcode = PROC_FP_TWO_SOURCE_OPCODE_FSUB;
    instr.proc_fp_two_source.rd = regToNo(dest);
    instr.proc_fp_two_source.rn = regToNo(a);
    instr.proc_fp_two_source.rm = regToNo(b);
    addInstruction(mem, instr);
}

void addInstFMul(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b) {
    Aarch64Instruction instr = { .instruction = 0, };
    instr.proc_fp_two_source.cnst0 = PROC_FP_TWO_SOURCE_CNST0;
    instr.proc_fp_two_source.cnst1 = PROC_FP_TWO_SOURCE_CNST1;
    instr.proc_fp_two_source.cnst2 = PROC_FP_TWO_SOURCE_CNST2;
    instr.proc_fp_two_source.cnst3 = PROC_FP_TWO_SOURCE_CNST3;
    instr.proc_fp_two_source.m = 0;
    instr.proc_fp_two_source.s = 0;
    instr.proc_fp_two_source.ptype = PROC_FP_TWO_SOURCE_PTYPE_DOUBLE;
    instr.proc_fp_two_source.opcode = PROC_FP_TWO_SOURCE_OPCODE_FMUL;
    instr.proc_fp_two_source.rd = regToNo(dest);
    instr.proc_fp_two_source.rn = regToNo(a);
    instr.proc_fp_two_source.rm = regToNo(b);
    addInstruction(mem, instr);
}

void addInstFDiv(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b) {
    Aarch64Instruction instr = { .instruction = 0, };
    instr.proc_fp_two_source.cnst0 = PROC_FP_TWO_SOURCE_CNST0;
    instr.proc_fp_two_source.cnst1 = PROC_FP_TWO_SOURCE_CNST1;
    instr.proc_fp_two_source.cnst2 = PROC_FP_TWO_SOURCE_CNST2;
    instr.proc_fp_two_source.cnst3 = PROC_FP_TWO_SOURCE_CNST3;
    instr.proc_fp_two_source.m = 0;
    instr.proc_fp_two_source.s = 0;
    instr.proc_fp_two_source.ptype = PROC_FP_TWO_SOURCE_PTYPE_DOUBLE;
    instr.proc_fp_two_source.opcode = PROC_FP_TWO_SOURCE_OPCODE_FDIV;
    instr.proc_fp_two_source.rd = regToNo(dest);
    instr.proc_fp_two_source.rn = regToNo(a);
    instr.proc_fp_two_source.rm = regToNo(b);
    addInstruction(mem, instr);
}

void addInstMovDirectRegToFReg(StackAllocator* mem, RegisterSet regs, Register freg, Register reg) {
    Aarch64Instruction instr = { .instruction = 0, };
    instr.cnvt_fp_int.cnst0 = CNVT_FP_INT_CNST0;
    instr.cnvt_fp_int.cnst1 = CNVT_FP_INT_CNST1;
    instr.cnvt_fp_int.cnst2 = CNVT_FP_INT_CNST2;
    instr.cnvt_fp_int.cnst3 = CNVT_FP_INT_CNST3;
    instr.cnvt_fp_int.ptype = CNVT_FP_INT_PTYPE_DOUBLE;
    instr.cnvt_fp_int.opcode = CNVT_FP_INT_OPCODE_MOV2F;
    instr.cnvt_fp_int.rmode = 0;
    instr.cnvt_fp_int.s = 0;
    instr.cnvt_fp_int.sf = 1;
    instr.cnvt_fp_int.rd = regToNo(freg);
    instr.cnvt_fp_int.rn = regToNo(reg);
    addInstruction(mem, instr);
}

void addInstMovImmToFReg(StackAllocator* mem, RegisterSet regs, Register reg, double value) {
    union {
        int64_t i;
        double d;
    } double_to_int64;
    double_to_int64.d = value;
    int free_reg = getFreeRegister(regs);
    if(free_reg == 0) {
        if (reg == REG_X(0)) {
            free_reg = REG_X(1);
        } else {
            free_reg = REG_X(0);
        }
        addInstPush(mem, regs, free_reg);
        addInstMovImmToReg(mem, regs, free_reg, double_to_int64.i);
        addInstMovDirectRegToFReg(mem, regs, reg, free_reg);
        addInstPop(mem, regs, free_reg);
    } else {
        addInstMovImmToReg(mem, regs, free_reg, double_to_int64.i);
        addInstMovDirectRegToFReg(mem, regs, reg, free_reg);
    }
}

void addInstMovMemToFReg(StackAllocator* mem, RegisterSet regs, Register reg, void* addr) {
    int free_reg = getFreeRegister(regs);
    if(free_reg == 0) {
        if (reg == REG_X(0)) {
            free_reg = REG_X(1);
        } else {
            free_reg = REG_X(0);
        }
        addInstPush(mem, regs, free_reg);
        addInstMovImmToReg(mem, regs, free_reg, (int64_t)addr);
        addInstMovMemRegToFReg(mem, regs, reg, free_reg);
        addInstPop(mem, regs, free_reg);
    } else {
        addInstMovImmToReg(mem, regs, free_reg, (int64_t)addr);
        addInstMovMemRegToFReg(mem, regs, reg, free_reg);
    }
}

void addInstMovFRegToMem(StackAllocator* mem, RegisterSet regs, Register reg, void* addr) {
    int free_reg = getFreeRegister(regs);
    if(free_reg == 0) {
        free_reg = getFirstRegister();
        addInstPush(mem, regs, free_reg);
        addInstMovImmToReg(mem, regs, free_reg, (int64_t)addr);
        addInstMovFRegToMemReg(mem, regs, free_reg, reg);
        addInstPop(mem, regs, free_reg);
    } else {
        addInstMovImmToReg(mem, regs, free_reg, (int64_t)addr);
        addInstMovFRegToMemReg(mem, regs, free_reg, reg);
    }
}

void addInstMovFRegToFReg(StackAllocator* mem, RegisterSet regs, Register dest, Register src) {
    if (dest != src) {
        Aarch64Instruction instr = { .instruction = 0, };
        instr.proc_fp_one_source.cnst0 = PROC_FP_ONE_SOURCE_CNST0;
        instr.proc_fp_one_source.cnst1 = PROC_FP_ONE_SOURCE_CNST1;
        instr.proc_fp_one_source.cnst2 = PROC_FP_ONE_SOURCE_CNST2;
        instr.proc_fp_one_source.cnst3 = PROC_FP_ONE_SOURCE_CNST3;
        instr.proc_fp_one_source.ptype = PROC_FP_ONE_SOURCE_PTYPE_DOUBLE;
        instr.proc_fp_one_source.opcode = PROC_FP_ONE_SOURCE_OPCODE_FMOV;
        instr.proc_fp_one_source.m = 0;
        instr.proc_fp_one_source.s = 0;
        instr.proc_fp_one_source.rd = regToNo(dest);
        instr.proc_fp_one_source.rn = regToNo(src);
        addInstruction(mem, instr);
    }
}

void addInstMovRegToFReg(StackAllocator* mem, RegisterSet regs, Register dest, Register src) {
    Aarch64Instruction instr = { .instruction = 0, };
    instr.cnvt_fp_int.cnst0 = CNVT_FP_INT_CNST0;
    instr.cnvt_fp_int.cnst1 = CNVT_FP_INT_CNST1;
    instr.cnvt_fp_int.cnst2 = CNVT_FP_INT_CNST2;
    instr.cnvt_fp_int.cnst3 = CNVT_FP_INT_CNST3;
    instr.cnvt_fp_int.ptype = CNVT_FP_INT_PTYPE_DOUBLE;
    instr.cnvt_fp_int.opcode = CNVT_FP_INT_OPCODE_S2F;
    instr.cnvt_fp_int.rmode = 0;
    instr.cnvt_fp_int.s = 0;
    instr.cnvt_fp_int.sf = 1;
    instr.cnvt_fp_int.rd = regToNo(dest);
    instr.cnvt_fp_int.rn = regToNo(src);
    addInstruction(mem, instr);
}

void addInstMovFRegToReg(StackAllocator* mem, RegisterSet regs, Register dest, Register src) {
    Aarch64Instruction instr = { .instruction = 0, };
    instr.cnvt_fp_int.cnst0 = CNVT_FP_INT_CNST0;
    instr.cnvt_fp_int.cnst1 = CNVT_FP_INT_CNST1;
    instr.cnvt_fp_int.cnst2 = CNVT_FP_INT_CNST2;
    instr.cnvt_fp_int.cnst3 = CNVT_FP_INT_CNST3;
    instr.cnvt_fp_int.ptype = CNVT_FP_INT_PTYPE_DOUBLE;
    instr.cnvt_fp_int.opcode = CNVT_FP_INT_OPCODE_F2S;
    instr.cnvt_fp_int.rmode = CNVT_FP_INT_RMODE_ZERO;
    instr.cnvt_fp_int.s = 0;
    instr.cnvt_fp_int.sf = 1;
    instr.cnvt_fp_int.rd = regToNo(dest);
    instr.cnvt_fp_int.rn = regToNo(src);
    addInstruction(mem, instr);
}

void addInstMovMemRegToFReg(StackAllocator* mem, RegisterSet regs, Register reg, Register addr) {
    Aarch64Instruction instr = { .instruction = 0, };
    instr.load_store_reg_unsi_imm.cnst0 = LOAD_STORE_REG_UNSI_IMM_CNST0;
    instr.load_store_reg_unsi_imm.cnst1 = LOAD_STORE_REG_UNSI_IMM_CNST1;
    instr.load_store_reg_unsi_imm.opc = LOAD_STORE_REG_UNSI_IMM_OPC_LDR;
    instr.load_store_reg_unsi_imm.size = LOAD_STORE_REG_UNSI_IMM_SIZE_DOUBLE;
    instr.load_store_reg_unsi_imm.v = 1;
    instr.load_store_reg_unsi_imm.imm12 = 0;
    instr.load_store_reg_unsi_imm.rn = regToNo(addr);
    instr.load_store_reg_unsi_imm.rt = regToNo(reg);
    addInstruction(mem, instr);
}

static void addInstMovFRegToMemRegWithOff(StackAllocator* mem, RegisterSet regs, Register addr, Register reg, int off) {
    Aarch64Instruction instr = { .instruction = 0, };
    instr.load_store_reg_unsi_imm.cnst0 = LOAD_STORE_REG_UNSI_IMM_CNST0;
    instr.load_store_reg_unsi_imm.cnst1 = LOAD_STORE_REG_UNSI_IMM_CNST1;
    instr.load_store_reg_unsi_imm.opc = LOAD_STORE_REG_UNSI_IMM_OPC_STR;
    instr.load_store_reg_unsi_imm.size = LOAD_STORE_REG_UNSI_IMM_SIZE_DOUBLE;
    instr.load_store_reg_unsi_imm.v = 1;
    instr.load_store_reg_unsi_imm.imm12 = off;
    instr.load_store_reg_unsi_imm.rn = regToNo(addr);
    instr.load_store_reg_unsi_imm.rt = regToNo(reg);
    addInstruction(mem, instr);
}

void addInstMovFRegToMemReg(StackAllocator* mem, RegisterSet regs, Register addr, Register reg) {
    addInstMovFRegToMemRegWithOff(mem, regs, addr, reg, 0);
}

static void addInstAddImm(StackAllocator* mem, RegisterSet regs, Register dest, Register src, int imm) {
    Aarch64Instruction instr = { .instruction = 0, };
    instr.add_sub_imm.cnst0 = ADD_SUB_IMM_CNST0;
    instr.add_sub_imm.imm12 = imm;
    instr.add_sub_imm.op = ADD_SUB_IMM_OP_ADD;
    instr.add_sub_imm.sf = 1;
    instr.add_sub_imm.sh = 0;
    instr.add_sub_imm.s = 0;
    instr.add_sub_imm.rd = regToNo(dest);
    instr.add_sub_imm.rn = regToNo(src);
    addInstruction(mem, instr);
}

static void addInstSubImm(StackAllocator* mem, RegisterSet regs, Register dest, Register src, int imm) {
    Aarch64Instruction instr = { .instruction = 0, };
    instr.add_sub_imm.cnst0 = ADD_SUB_IMM_CNST0;
    instr.add_sub_imm.imm12 = imm;
    instr.add_sub_imm.op = ADD_SUB_IMM_OP_SUB;
    instr.add_sub_imm.sf = 1;
    instr.add_sub_imm.sh = 0;
    instr.add_sub_imm.s = 0;
    instr.add_sub_imm.rd = regToNo(dest);
    instr.add_sub_imm.rn = regToNo(src);
    addInstruction(mem, instr);
}

#define MAX_REG_ARGS 8

void addInstFunctionCall(StackAllocator* mem, RegisterSet regs, Register ret, int arg_count, Register* args, void* func) {
    bool in_register[arg_count + 1];
    addInstPushAll(mem, regs, regs & ~ret);
    addInstPush(mem, regs, REG_X(REG_LINK));
    RegisterSet all_regs = 0;
    RegisterSet tmp_regs = 0;
    size_t int_count = 0;
    size_t float_count = 0;
    int stack_size = 0;
    for (int i = 0; i < arg_count; i++) {
        if (args[i] >= REG_D(0)) {
            in_register[i] = (float_count < MAX_REG_ARGS);
            float_count++;
        } else {
            in_register[i] = (int_count < MAX_REG_ARGS);
            int_count++;
        }
        if (in_register[i]) {
            tmp_regs |= args[i];
        } else {
            stack_size++;
        }
        all_regs |= args[i];
    }
    if (stack_size > 0) {
        Register tmp = getFreeRegister(tmp_regs);
        tmp_regs |= tmp;
        addInstSubImm(mem, regs, REG_X(REG_SPECIAL), REG_X(REG_SPECIAL), (1 + stack_size)/2 * 16);
        addInstAddImm(mem, regs, tmp, REG_X(REG_SPECIAL), 0);
        stack_size = 0;
        for (int i = 0; i < arg_count; i++) {
            if (!in_register[i]) {
                if (args[i] >= REG_D(0)) {
                    addInstMovFRegToMemRegWithOff(mem, tmp_regs, tmp, args[i], stack_size);
                } else {
                    addInstMovRegToMemRegWithOff(mem, tmp_regs, tmp, args[i], stack_size);
                }
                stack_size++;
            }
        }
        tmp_regs &= ~tmp;
    }
    float_count = 0;
    int_count = 0;
    for (int i = 0; i < arg_count; i++) {
        if (in_register[i]) {
            if (args[i] >= REG_D(0)) {
                Register to_use = REG_D(float_count);
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
                Register to_use = REG_X(int_count);
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
    addInstCallBasic(mem, tmp_regs, func);
    if (ret != 0) {
        if(ret >= REG_D(0)) {
            if(ret != REG_D(0)) {
                addInstMovFRegToFReg(mem, regs, ret, REG_D(0));
            }
        } else {
            if(ret != REG_X(0)) {
                addInstMovRegToReg(mem, regs, ret, REG_X(0));
            }
        }
    }
    if (stack_size > 0) {
        addInstAddImm(mem, regs, REG_X(REG_SPECIAL), REG_X(REG_SPECIAL), 16*(1 + stack_size)/2);
    }
    addInstPop(mem, regs, REG_X(REG_LINK));
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
    for (int i = 19; i <= 28; i++) {
        addInstPush(mem, regs, REG_X(i));
    }
    for (int i = 8; i <= 15; i++) {
        addInstPush(mem, regs, REG_D(i));
    }
}

void addInstPopCallerRegs(StackAllocator* mem, RegisterSet regs) {
    for (int i = 15; i >= 8; i--) {
        addInstPop(mem, regs, REG_D(i));
    }
    for (int i = 28; i >= 19; i--) {
        addInstPop(mem, regs, REG_X(i));
    }
}

void updateRelativeJumpTarget(StackAllocator* mem, size_t pos, size_t to) {
    uint32_t rel = to - pos;
    Aarch64Instruction instr = getInstruction(mem, pos);
    if (instr.uncond_branch_imm.cnst0 == UNCOND_BRANCH_IMM_CNST0) {
        instr.uncond_branch_imm.imm26 = rel >> 2;
    } else if (instr.cond_branch_imm.cnst0 == COND_BRANCH_IMM_CNST0) {
        instr.cond_branch_imm.imm19 = rel >> 2;
    }
    updateInstruction(mem, pos, instr);
}

void updateImmediateValue(StackAllocator* mem, size_t pos, int64_t value) {
    for (int i = 0; i < 4; i++) {
        Aarch64Instruction instr = getInstruction(mem, pos + 4 * i);
        instr.move_imm.imm16 = (value >> (16 * i)) & 0xffff;
        updateInstruction(mem, pos + 4 * i, instr);
    }
}

#endif
