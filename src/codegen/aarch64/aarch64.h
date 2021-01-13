#ifndef _AARCH64_H_
#define _AARCH64_H_

#include <stdint.h>

#define PC_REL_PAD0 0b10000

#define ADD_IMM_PAD0 0b100010

#define ADD_IMM_OP_ADD 0b0
#define ADD_IMM_OP_SUB 0b1

#define ADD_IMM_TAG_PAD0 0b100011

#define LOGICAL_IMM_PAD0 0b100100

#define LOGICAL_IMM_OPC_AND  0b00
#define LOGICAL_IMM_OPC_ORR  0b01
#define LOGICAL_IMM_OPC_EOR  0b10
#define LOGICAL_IMM_OPC_ANDS 0b11

#define MOVE_IMM_PAD0 0b100101

#define MOVE_IMM_OPC_MOVN 0b00
#define MOVE_IMM_OPC_MOVZ 0b10
#define MOVE_IMM_OPC_MOVK 0b11

#define BITFIELD_PAD0 0b100110

#define BITFIELD_OPC_SBFM 0b00
#define BITFIELD_OPC_BFM  0b01
#define BITFIELD_OPC_UBFM 0b10

#define EXTRACT_PAD0 0b100111

#define COND_BRANCH_IMM_PAD0 0b0101010

#define EXCEPTION_PAD0 0b11010100

#define HINTS_PAD0 0b11010101000000110010
#define HINTS_PAD1 0b11111

#define BARRIERS_PAD0 0b11010101000000110011

#define PSTATE_PAD0 0b1101010100000
#define PSTATE_PAD1 0b0100

#define SYSTEM_INSTR_PAD0 0b1101010100
#define SYSTEM_INSTR_PAD1 0b01

#define SYSTEM_MOVE_PAD0 0b1101010100
#define SYSTEM_MOVE_PAD1 0b1

#define UNCOND_BRANCH_REG_PAD0 0b1101011

#define UNCOND_BRANCH_REG_OPC_BR  0b0000
#define UNCOND_BRANCH_REG_OPC_BLR 0b0001
#define UNCOND_BRANCH_REG_OPC_RET 0b0010

#define UNCOND_BRANCH_REG_OP2_BR_BLR_RET 0b11111
#define UNCOND_BRANCH_REG_OP3_BR_BLR_RET 0b000000
#define UNCOND_BRANCH_REG_OP4_BR_BLR_RET 0b00000

#define UNCOND_BRANCH_IMM_PAD0 0b00101

#define UNCOND_BRANCH_IMM_OP_B  0b0
#define UNCOND_BRANCH_IMM_OP_BL 0b1


typedef union {
    struct {
        uint32_t pad1 : 25;
        uint32_t op0 : 4;
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
        uint32_t op0 : 3;
        uint32_t pad1 : 3;
        uint32_t pad0 : 3;
    } proc_imm;
    struct {
        uint32_t rd : 5;
        uint32_t immhi : 19;
        uint32_t pad0 : 5;
        uint32_t immlo : 2;
        uint32_t op : 1;
    } pc_rel;
    struct {
        uint32_t rd : 5;
        uint32_t rn : 5;
        uint32_t imm12 : 12;
        uint32_t sh : 1;
        uint32_t pad0 : 6;
        uint32_t s : 1;
        uint32_t op : 1;
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
        uint32_t op : 1;
        uint32_t sf : 1;
    } add_imm_tag;
    struct {
        uint32_t rd : 5;
        uint32_t rn : 5;
        uint32_t imms : 6;
        uint32_t immr : 6;
        uint32_t n : 1;
        uint32_t pad0 : 6;
        uint32_t opc : 2;
        uint32_t sf : 1;
    } logical_imm;
    struct {
        uint32_t rd : 5;
        uint32_t imm16 : 16;
        uint32_t hw : 2;
        uint32_t pad0 : 6;
        uint32_t opc : 2;
        uint32_t sf : 1;
    } move_imm;
    struct {
        uint32_t rd : 5;
        uint32_t rn : 5;
        uint32_t imms : 6;
        uint32_t immr : 6;
        uint32_t n : 1;
        uint32_t pad0 : 6;
        uint32_t opc : 2;
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
        uint32_t op21 : 2;
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
    struct {
        uint32_t ll : 2;
        uint32_t op2 : 3;
        uint32_t imm16 : 16;
        uint32_t opc : 3;
        uint32_t pad0 : 8;
    } exception;
    struct {
        uint32_t pad1 : 5;
        uint32_t op2 : 3;
        uint32_t crm : 4;
        uint32_t pad0 : 20;
    } hints;
    struct {
        uint32_t rt : 5;
        uint32_t op2 : 3;
        uint32_t crm : 4;
        uint32_t pad0 : 20;
    } barriers;
    struct {
        uint32_t rt : 5;
        uint32_t op2 : 3;
        uint32_t crm : 4;
        uint32_t pad1 : 4;
        uint32_t op1 : 3;
        uint32_t pad0 : 13;
    } pstate;
    struct {
        uint32_t rt : 5;
        uint32_t op2 : 3;
        uint32_t crm : 4;
        uint32_t crn : 4;
        uint32_t op1 : 3;
        uint32_t pad1 : 2;
        uint32_t l : 1;
        uint32_t pad0 : 10;
    } system_instr;
    struct {
        uint32_t rt : 5;
        uint32_t op2 : 3;
        uint32_t crm : 4;
        uint32_t crn : 4;
        uint32_t op1 : 3;
        uint32_t o0 : 1;
        uint32_t pad1 : 1;
        uint32_t l : 1;
        uint32_t pad0 : 10;
    } system_move;
    struct {
        uint32_t op4 : 5;
        uint32_t rn : 5;
        uint32_t op3 : 6;
        uint32_t op2 : 5;
        uint32_t opc : 4;
        uint32_t pad0 : 7;
    } uncond_branch_reg;
    struct {
        uint32_t imm26 : 26;
        uint32_t pad0 : 5;
        uint32_t op : 1;
    } uncond_branch_imm;
    uint32_t instruction;
} Aarch64Instruction;

#endif