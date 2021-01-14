
// TODO: remove temporary definition
#define __aarch64__

#ifdef __aarch64__

#include "codegen/instructions.h"
#include "codegen/aarch64/aarch64.h"

static int regToNo(Register reg) {
    for(int i = 0; i < REG_COUNT; i++) {
        if((reg & (1 << i)) != 0) {
            return i;
        }
    }
    for(int i = REG_COUNT; i < REG_COUNT + FREG_COUNT; i++) {
        if((reg & (1 << i)) != 0) {
            return i - REG_COUNT;
        }
    }
    return 31;
}

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
    Aarch64Instruction instr = { .instruction = 0, };
    instr.logical_shift_reg.cnst0 = LOGICAL_SHIFT_REG_CNST0;
    instr.logical_shift_reg.opc = LOGICAL_SHIFT_REG_OPC_ORR;
    instr.logical_shift_reg.sf = 1;
    instr.logical_shift_reg.rd = regToNo(dest);
    instr.logical_shift_reg.rn = REG_SPECIAL;
    instr.logical_shift_reg.rm = regToNo(src);
    addInstruction(mem, instr);
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
        instr.move_imm.imm16 = value >> (16 * i);
        addInstruction(mem, instr);
    }
    return ret;
}

void addInstMovMemToReg(StackAllocator* mem, RegisterSet regs, Register reg, void* addr) {
    addInstMovImmToReg(mem, regs, reg, (uint64_t)addr);
    addInstMovMemRegToReg(mem, regs, reg, reg);
}

void addInstMovRegToMem(StackAllocator* mem, RegisterSet regs, Register reg, void* addr) {
    addInstMovImmToReg(mem, regs, reg, (uint64_t)addr);
    addInstMovRegToMemReg(mem, regs, reg, reg);
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

void addInstMovRegToMemReg(StackAllocator* mem, RegisterSet regs, Register addr, Register reg) {
    Aarch64Instruction instr = { .instruction = 0, };
    instr.load_store_reg_unsi_imm.cnst0 = LOAD_STORE_REG_UNSI_IMM_CNST0;
    instr.load_store_reg_unsi_imm.cnst1 = LOAD_STORE_REG_UNSI_IMM_CNST1;
    instr.load_store_reg_unsi_imm.opc = LOAD_STORE_REG_UNSI_IMM_OPC_STR;
    instr.load_store_reg_unsi_imm.size = LOAD_STORE_REG_UNSI_IMM_SIZE_DOUBLE;
    instr.load_store_reg_unsi_imm.v = 0;
    instr.load_store_reg_unsi_imm.imm12 = 0;
    instr.load_store_reg_unsi_imm.rn = regToNo(addr);
    instr.load_store_reg_unsi_imm.rt = regToNo(reg);
    addInstruction(mem, instr);
}

void addInstJmpReg(StackAllocator* mem, RegisterSet regs, Register reg) {
    Aarch64Instruction instr = { .instruction = 0, };
    instr.uncond_branch_reg.cnst0 = UNCOND_BRANCH_IMM_CNST0;
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
        addInstMovImmToReg(mem, regs, free_reg, (uint64_t)to);
        addInstJmpReg(mem, regs, free_reg);
        addInstPop(mem, regs, free_reg);
    } else {
        addInstMovImmToReg(mem, regs, free_reg, (uint64_t)to);
        addInstJmpReg(mem, regs, free_reg);
    }
}

size_t addInstJmpRel(StackAllocator* mem, RegisterSet regs, size_t to) {
    size_t ret = mem->occupied;
    uint32_t rel = to - (mem->occupied + 4);
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
    if(reg >= (1 << REG_COUNT)) {
        instr.load_store_reg_imm_post.v = 1;
    } else {
        instr.load_store_reg_imm_post.v = 0;
    }
    instr.load_store_reg_imm_pre.imm9 = -8;
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
    if(reg >= (1 << REG_COUNT)) {
        instr.load_store_reg_imm_post.v = 1;
    } else {
        instr.load_store_reg_imm_post.v = 0;
    }
    instr.load_store_reg_imm_post.imm9 = 8;
    addInstruction(mem, instr);
}

void addInstPushAll(StackAllocator* mem, RegisterSet regs) {
    for(int i = 0; i < REG_COUNT + FREG_COUNT; i++) {
        if((regs & (1 << i)) != 0) {
            addInstPush(mem, regs, (1 << i));
        }
    }
}

void addInstPopAll(StackAllocator* mem, RegisterSet regs) {
    for(int i = REG_COUNT + FREG_COUNT - 1; i >= 0; i--) {
        if((regs & (1 << i)) != 0) {
            addInstPop(mem, regs, (1 << i));
        }
    }
}

void addInstCallReg(StackAllocator* mem, RegisterSet regs, Register reg) {
    addInstPush(mem, regs, (1 << REG_LINK));
    Aarch64Instruction instr = { .instruction = 0, };
    instr.uncond_branch_reg.cnst0 = UNCOND_BRANCH_IMM_CNST0;
    instr.uncond_branch_reg.op2 = UNCOND_BRANCH_REG_OP2_BR_BLR_RET;
    instr.uncond_branch_reg.op3 = UNCOND_BRANCH_REG_OP3_BR_BLR_RET;
    instr.uncond_branch_reg.op4 = UNCOND_BRANCH_REG_OP4_BR_BLR_RET;
    instr.uncond_branch_reg.opc = UNCOND_BRANCH_REG_OPC_BLR;
    instr.uncond_branch_reg.rn = regToNo(reg);
    addInstruction(mem, instr);
    addInstPop(mem, regs, (1 << REG_LINK));
}

void addInstCall(StackAllocator* mem, RegisterSet regs, void* func) {
    int free_reg = getFreeRegister(regs);
    if(free_reg == 0) {
        free_reg = getFirstRegister();
        addInstPush(mem, regs, free_reg);
        addInstMovImmToReg(mem, regs, free_reg, (uint64_t)func);
        addInstCallReg(mem, regs, free_reg);
        addInstPop(mem, regs, free_reg);
    } else {
        addInstMovImmToReg(mem, regs, free_reg, (uint64_t)func);
        addInstCallReg(mem, regs, free_reg);
    }
}

size_t addInstCallRel(StackAllocator* mem, RegisterSet regs, size_t to) {
    size_t ret = mem->occupied;
    uint32_t rel = to - (mem->occupied + 4);
    Aarch64Instruction instr = { .instruction = 0, };
    instr.uncond_branch_imm.cnst0 = UNCOND_BRANCH_IMM_CNST0;
    instr.uncond_branch_imm.op = UNCOND_BRANCH_IMM_OP_BL;
    instr.uncond_branch_imm.imm26 = rel >> 2;
    addInstruction(mem, instr);
    return ret;
}

void addInstReturn(StackAllocator* mem, RegisterSet regs) {
    Aarch64Instruction instr = { .instruction = 0, };
    instr.uncond_branch_reg.cnst0 = UNCOND_BRANCH_IMM_CNST0;
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
    instr.proc_reg_two_source.rn = regToNo(b);
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
    addInstDiv(mem, regs, dest, a, b);
    addInstMSub(mem, regs, dest, dest, b, a);
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
    if(a < (1 << REG_COUNT)) {
        addInstCmp(mem, regs, a, b);
    } else {
        addInstFCmp(mem, regs, a, b);
    }
    size_t ret = mem->occupied;
    uint32_t rel = to - (mem->occupied + 4);
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

void addInstFAdd(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b);

void addInstFSub(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b);

void addInstFMul(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b);

void addInstFDiv(StackAllocator* mem, RegisterSet regs, Register dest, Register a, Register b);

void addInstMovImmToFReg(StackAllocator* mem, RegisterSet regs, Register reg, double value);

void addInstMovMemToFReg(StackAllocator* mem, RegisterSet regs, Register reg, void* addr);

void addInstMovFRegToMem(StackAllocator* mem, RegisterSet regs, Register reg, void* addr);

void addInstMovFRegToFReg(StackAllocator* mem, RegisterSet regs, Register dest, Register src);

void addInstMovRegToFReg(StackAllocator* mem, RegisterSet regs, Register dest, Register src);

void addInstMovFRegToReg(StackAllocator* mem, RegisterSet regs, Register dest, Register src);

void addInstMovMemRegToFReg(StackAllocator* mem, RegisterSet regs, Register reg, Register addr);

void addInstMovFRegToMemReg(StackAllocator* mem, RegisterSet regs, Register addr, Register reg);

void addInstFunctionCallUnary(StackAllocator* mem, RegisterSet regs, Register ret, Register a, void* func);

void addInstFunctionCallBinary(StackAllocator* mem, RegisterSet regs, Register ret, Register a, Register b, void* func);

void addInstFunctionCallUnaryNoRet(StackAllocator* mem, RegisterSet regs, Register a, void* func);

void addInstFunctionCallRetOnly(StackAllocator* mem, RegisterSet regs, Register ret, void* func);

void addInstFunctionCallSimple(StackAllocator* mem, RegisterSet regs, void* func);

void addInstPushCallerRegs(StackAllocator* mem, RegisterSet regs);

void addInstPopCallerRegs(StackAllocator* mem, RegisterSet regs);

void updateRelativeJumpTarget(StackAllocator* mem, size_t pos, size_t value);

void updateImmediateValue(StackAllocator* mem, size_t pos, int64_t value);

#endif
