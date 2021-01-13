#ifndef _AARCH64_H_
#define _AARCH64_H_

#include <stdint.h>

typedef enum {
    MAIN_OP0_RESERVED = 0b0000,
    MAIN_OP0_UNALLOCATED0 = 0b0001,
    MAIN_OP0_SVE = 0b0010,
    MAIN_OP0_UNALLOCATED1 = 0b0011,
    MAIN_OP0_PROC_IMM0 = 0b1000,
    MAIN_OP0_PROC_IMM1 = 0b1001,
    MAIN_OP0_BRANCHES0 = 0b1010,
    MAIN_OP0_BRANCHES1 = 0b1011,
    MAIN_OP0_LOAD_STORE00 = 0b0100,
    MAIN_OP0_LOAD_STORE01 = 0b0110,
    MAIN_OP0_LOAD_STORE10 = 0b1100,
    MAIN_OP0_LOAD_STORE11 = 0b1110,
    MAIN_OP0_PROC_REG0 = 0b0101,
    MAIN_OP0_PROC_REG1 = 0b1101,
    MAIN_OP0_PROC_FP0 = 0b0111,
    MAIN_OP0_PROC_FP1 = 0b1111,
} Aarch64MainOp0;

typedef enum {
    PROC_IMM_OP0_PC_REL0 = 0b000,
    PROC_IMM_OP0_PC_REL1 = 0b001,
    PROC_IMM_OP0_ADD = 0b010,
    PROC_IMM_OP0_ADD_TAG = 0b011,
    PROC_IMM_OP0_LOGICAL = 0b100,
    PROC_IMM_OP0_MOVE = 0b101,
    PROC_IMM_OP0_BITFIELD = 0b110,
    PROC_IMM_OP0_EXTRACT = 0b111,
} Aarch64ProcImmOp0;

typedef enum {
    PC_REL_OP_ADR = 0b0,
    PC_REL_OP_ADRP = 0b1,
} Aarch64PcRelOp;

typedef enum {
    ADD_IMM_OP_ADD = 0b0,
    ADD_IMM_OP_SUB = 0b1,
} Aarch64AddImmOp;

typedef enum {
    LOGICAL_IMM_OPC_AND = 0b00,
    LOGICAL_IMM_OPC_ORR = 0b01,
    LOGICAL_IMM_OPC_EOR = 0b10,
    LOGICAL_IMM_OPC_ANDS = 0b11,
} Aarch64LogicalImmOpc;

typedef enum {
    MOVE_IMM_OPC_MOVN = 0b00,
    MOVE_IMM_OPC_UNALLOCATED = 0b01,
    MOVE_IMM_OPC_MOVZ = 0b10,
    MOVE_IMM_OPC_MOVK = 0b11,
} Aarch64MoveImmOpc;

typedef enum {
    BITFIELD_OPC_SBFM = 0b00,
    BITFIELD_OPC_BFM = 0b01,
    BITFIELD_OPC_UBFM = 0b10,
    BITFIELD_OPC_UNALLOCATED = 0b11,
} Aarch64BitfieldOpc;

typedef enum {
    EXTACT_OP21_UNALLOCATED0 = 0b00,
    EXTACT_OP21_UNALLOCATED1 = 0b01,
    EXTACT_OP21_UNALLOCATED2 = 0b10,
    EXTACT_OP21_EXTR = 0b11,
} Aarch64ExtractOp21;

#define COND_BRANCH_IMM_PAD0 0b0101010

typedef union {
    struct {
        uint32_t pad1 : 25;
        Aarch64MainOp0 op0 : 4;
        uint32_t pad0 : 3;
    } main;
    struct {
        uint32_t pad1 : 16;
        uint32_t op1 : 9;
        uint32_t pad0 : 4;
        uint32_t op0 : 3;
    } reserved;
    struct {
        uint32_t pad2 : 23;
        Aarch64ProcImmOp0 op0 : 3;
        uint32_t pad1 : 3;
        uint32_t pad0 : 3;
    } proc_imm;
    struct {
        uint32_t rd : 5;
        uint32_t immhi : 19;
        uint32_t pad0 : 5;
        uint32_t immlo : 2;
        Aarch64PcRelOp op : 1;
    } pc_rel;
    struct {
        uint32_t rd : 5;
        uint32_t rn : 5;
        uint32_t imm12 : 12;
        uint32_t sh : 1;
        uint32_t pad0 : 6;
        uint32_t s : 1;
        Aarch64AddImmOp op : 1;
        uint32_t sf : 1;
    } add_imm;
    struct {
        uint32_t rd : 5;
        uint32_t rn : 5;
        uint32_t uimm4 : 4;
        uint32_t op3 : 2;
        uint32_t uimm6 : 6;
        uint32_t s2 : 1;
        uint32_t pad0 : 6;
        uint32_t s : 1;
        Aarch64AddImmOp op : 1;
        uint32_t sf : 1;
    } add_imm_tag;
    struct {
        uint32_t rd : 5;
        uint32_t rn : 5;
        uint32_t imms : 6;
        uint32_t immr : 6;
        uint32_t n : 1;
        uint32_t pad0 : 6;
        Aarch64LogicalImmOpc opc : 2;
        uint32_t sf : 1;
    } logical_imm;
    struct {
        uint32_t rd : 5;
        uint32_t imm16 : 16;
        uint32_t hw : 2;
        uint32_t pad0 : 6;
        Aarch64MoveImmOpc opc : 2;
        uint32_t sf : 1;
    } move_imm;
    struct {
        uint32_t rd : 5;
        uint32_t rn : 5;
        uint32_t imms : 6;
        uint32_t immr : 6;
        uint32_t n : 1;
        uint32_t pad0 : 6;
        Aarch64BitfieldOpc opc : 2;
        uint32_t sf : 1;
    } bitfield;
    struct {
        uint32_t rd : 5;
        uint32_t rn : 5;
        uint32_t imms : 6;
        uint32_t rm : 5;
        uint32_t o0 : 1;
        uint32_t n : 1;
        uint32_t pad0 : 6;
        Aarch64ExtractOp21 op21 : 2;
        uint32_t sf : 1;
    } extract;
    struct {
        uint32_t op2 : 5;
        uint32_t pad1 : 7;
        uint32_t op1 : 14;
        uint32_t pad0 : 3;
        uint32_t op0 : 3;
    } branches;
    struct {
        uint32_t cond : 4;
        uint32_t o0 : 1;
        uint32_t imm19 : 19;
        uint32_t o1 : 1;
        uint32_t pad0 : 7;
    } cond_branch_imm;
    uint32_t instruction;
} Aarch64Instruction;

#endif