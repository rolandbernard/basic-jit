#ifndef _AARCH64_H_
#define _AARCH64_H_

#include <stdint.h>
#include "common/stackalloc.h"

#define REG_COUNT 29
#define FREG_COUNT 32

#define REG_X(X) (1 << X)

#define REG_D(X) (1 << (REG_COUNT + X))

#define REG_SPECIAL 31
#define REG_LINK 30

#define PC_REL_CNST0 0b10000

#define ADD_IMM_CNST0 0b100010

#define ADD_IMM_OP_ADD 0b0
#define ADD_IMM_OP_SUB 0b1

#define ADD_IMM_TAG_CNST0 0b100011

#define LOGICAL_IMM_CNST0 0b100100

#define LOGICAL_IMM_OPC_AND  0b00
#define LOGICAL_IMM_OPC_ORR  0b01
#define LOGICAL_IMM_OPC_EOR  0b10
#define LOGICAL_IMM_OPC_ANDS 0b11

#define MOVE_IMM_CNST0 0b100101

#define MOVE_IMM_OPC_MOVN 0b00
#define MOVE_IMM_OPC_MOVZ 0b10
#define MOVE_IMM_OPC_MOVK 0b11

#define BITFIELD_CNST0 0b100110

#define BITFIELD_OPC_SBFM 0b00
#define BITFIELD_OPC_BFM  0b01
#define BITFIELD_OPC_UBFM 0b10

#define EXTRACT_CNST0 0b100111

#define COND_BRANCH_IMM_CNST0 0b0101010

#define COND_BRANCH_IMM_COND_EQ 0b0000
#define COND_BRANCH_IMM_COND_NE 0b0001
#define COND_BRANCH_IMM_COND_CS 0b0010
#define COND_BRANCH_IMM_COND_CC 0b0011
#define COND_BRANCH_IMM_COND_MI 0b0100
#define COND_BRANCH_IMM_COND_PL 0b0101
#define COND_BRANCH_IMM_COND_VS 0b0110
#define COND_BRANCH_IMM_COND_VC 0b0111
#define COND_BRANCH_IMM_COND_HI 0b1000
#define COND_BRANCH_IMM_COND_LS 0b1001
#define COND_BRANCH_IMM_COND_GE 0b1010
#define COND_BRANCH_IMM_COND_LT 0b1011
#define COND_BRANCH_IMM_COND_GT 0b1100
#define COND_BRANCH_IMM_COND_LE 0b1101
#define COND_BRANCH_IMM_COND_AL 0b1110

#define EXCEPTION_CNST0 0b11010100

#define HINTS_CNST0 0b11010101000000110010
#define HINTS_CNST1 0b11111

#define BARRIERS_CNST0 0b11010101000000110011

#define PSTATE_CNST0 0b1101010100000
#define PSTATE_CNST1 0b0100

#define SYSTEM_INSTR_CNST0 0b1101010100
#define SYSTEM_INSTR_CNST1 0b01

#define SYSTEM_MOVE_CNST0 0b1101010100
#define SYSTEM_MOVE_CNST1 0b1

#define UNCOND_BRANCH_REG_CNST0 0b1101011

#define UNCOND_BRANCH_REG_OPC_BR  0b0000
#define UNCOND_BRANCH_REG_OPC_BLR 0b0001
#define UNCOND_BRANCH_REG_OPC_RET 0b0010

#define UNCOND_BRANCH_REG_OP2_BR_BLR_RET 0b11111
#define UNCOND_BRANCH_REG_OP3_BR_BLR_RET 0b000000
#define UNCOND_BRANCH_REG_OP4_BR_BLR_RET 0b00000

#define UNCOND_BRANCH_IMM_CNST0 0b00101

#define UNCOND_BRANCH_IMM_OP_B  0b0
#define UNCOND_BRANCH_IMM_OP_BL 0b1

#define COMP_BRANCH_IMM_CNST0 0b011010

#define COMP_BRANCH_IMM_OP_CBZ  0b0
#define COMP_BRANCH_IMM_OP_CBNZ 0b1

#define TEST_BRANCH_IMM_CNST0 0b011011

#define TEST_BRANCH_IMM_OP_TBZ  0b0
#define TEST_BRANCH_IMM_OP_TBNZ 0b1

#define LOAD_REG_LIT_CNST0 0b011
#define LOAD_REG_LIT_CNST1 0b00

#define LOAD_STORE_REG_UNSC_IMM_CNST0 0b111
#define LOAD_STORE_REG_UNSC_IMM_CNST1 0b00
#define LOAD_STORE_REG_UNSC_IMM_CNST2 0b0
#define LOAD_STORE_REG_UNSC_IMM_CNST3 0b00

#define LOAD_STORE_REG_UNSC_IMM_OPC_STUR  0b00
#define LOAD_STORE_REG_UNSC_IMM_OPC_LDUR  0b01
#define LOAD_STORE_REG_UNSC_IMM_OPC_LDURS 0b10

#define LOAD_STORE_REG_UNSC_IMM_SIZE_BYTE   0b00
#define LOAD_STORE_REG_UNSC_IMM_SIZE_HALF   0b01
#define LOAD_STORE_REG_UNSC_IMM_SIZE_WORD   0b10
#define LOAD_STORE_REG_UNSC_IMM_SIZE_DOUBLE 0b11

#define LOAD_STORE_REG_IMM_POST_CNST0 0b111
#define LOAD_STORE_REG_IMM_POST_CNST1 0b00
#define LOAD_STORE_REG_IMM_POST_CNST2 0b0
#define LOAD_STORE_REG_IMM_POST_CNST3 0b01

#define LOAD_STORE_REG_IMM_POST_OPC_STR  0b00
#define LOAD_STORE_REG_IMM_POST_OPC_LDR  0b01
#define LOAD_STORE_REG_IMM_POST_OPC_LDRS 0b10

#define LOAD_STORE_REG_IMM_POST_SIZE_BYTE   0b00
#define LOAD_STORE_REG_IMM_POST_SIZE_HALF   0b01
#define LOAD_STORE_REG_IMM_POST_SIZE_WORD   0b10
#define LOAD_STORE_REG_IMM_POST_SIZE_DOUBLE 0b11

#define LOAD_STORE_REG_IMM_PRE_CNST0 0b111
#define LOAD_STORE_REG_IMM_PRE_CNST1 0b00
#define LOAD_STORE_REG_IMM_PRE_CNST2 0b0
#define LOAD_STORE_REG_IMM_PRE_CNST3 0b11

#define LOAD_STORE_REG_IMM_PRE_OPC_STR  0b00
#define LOAD_STORE_REG_IMM_PRE_OPC_LDR  0b01
#define LOAD_STORE_REG_IMM_PRE_OPC_LDRS 0b10

#define LOAD_STORE_REG_IMM_PRE_SIZE_BYTE   0b00
#define LOAD_STORE_REG_IMM_PRE_SIZE_HALF   0b01
#define LOAD_STORE_REG_IMM_PRE_SIZE_WORD   0b10
#define LOAD_STORE_REG_IMM_PRE_SIZE_DOUBLE 0b11

#define LOAD_STORE_REG_REG_CNST0 0b111
#define LOAD_STORE_REG_REG_CNST1 0b00
#define LOAD_STORE_REG_REG_CNST2 0b1
#define LOAD_STORE_REG_REG_CNST3 0b10

#define LOAD_STORE_REG_REG_OPC_STR  0b00
#define LOAD_STORE_REG_REG_OPC_LDR  0b01
#define LOAD_STORE_REG_REG_OPC_LDRS 0b10

#define LOAD_STORE_REG_REG_SIZE_BYTE   0b00
#define LOAD_STORE_REG_REG_SIZE_HALF   0b01
#define LOAD_STORE_REG_REG_SIZE_WORD   0b10
#define LOAD_STORE_REG_REG_SIZE_DOUBLE 0b11

#define LOAD_STORE_REG_UNSI_IMM_CNST0 0b111
#define LOAD_STORE_REG_UNSI_IMM_CNST1 0b01

#define LOAD_STORE_REG_UNSI_IMM_OPC_STR  0b00
#define LOAD_STORE_REG_UNSI_IMM_OPC_LDR  0b01
#define LOAD_STORE_REG_UNSI_IMM_OPC_LDRS 0b10

#define LOAD_STORE_REG_UNSI_IMM_SIZE_BYTE   0b00
#define LOAD_STORE_REG_UNSI_IMM_SIZE_HALF   0b01
#define LOAD_STORE_REG_UNSI_IMM_SIZE_WORD   0b10
#define LOAD_STORE_REG_UNSI_IMM_SIZE_DOUBLE 0b11

#define PROC_REG_TWO_SOURCE_CNST0 0b0
#define PROC_REG_TWO_SOURCE_CNST1 0b11010110

#define PROC_REG_TWO_SOURCE_OPCODE_UDIV 0b000010
#define PROC_REG_TWO_SOURCE_OPCODE_SDIV 0b000011
#define PROC_REG_TWO_SOURCE_OPCODE_LSLV 0b001000
#define PROC_REG_TWO_SOURCE_OPCODE_LSRV 0b001001
#define PROC_REG_TWO_SOURCE_OPCODE_ASRV 0b001010
#define PROC_REG_TWO_SOURCE_OPCODE_RORV 0b001011

#define PROC_REG_ONE_SOURCE_CNST0 0b1
#define PROC_REG_ONE_SOURCE_CNST1 0b11010110

#define LOGICAL_SHIFT_REG_CNST0 0b01010

#define LOGICAL_SHIFT_REG_OPC_AND 0b00
#define LOGICAL_SHIFT_REG_OPC_ORR 0b01
#define LOGICAL_SHIFT_REG_OPC_EOR 0b10
#define LOGICAL_SHIFT_REG_OPC_ANDS 0b10

#define ADD_SHIFT_REG_CNST0 0b01011
#define ADD_SHIFT_REG_CNST1 0b0

#define ADD_SHIFT_REG_OP_ADD 0b0
#define ADD_SHIFT_REG_OP_SUB 0b1

#define ADD_EXT_REG_CNST0 0b01011
#define ADD_EXT_REG_CNST1 0b1

#define ADD_EXT_REG_OP_ADD 0b0
#define ADD_EXT_REG_OP_SUB 0b1

#define ADD_CARRY_CNST0 0b11010000
#define ADD_CARRY_CNST1 0b000000

#define ADD_CARRY_OP_ADC 0b0
#define ADD_CARRY_OP_SBC 0b1

#define PROC_REG_THREE_SOURCE_CNST0 0b11011

#define PROC_REG_THREE_SOURCE_O0_ADD 0b0
#define PROC_REG_THREE_SOURCE_O0_SUB 0b1

#define PROC_REG_THREE_SOURCE_OP31_MADD_MSUB 0b000

#define PROC_REG_THREE_SOURCE_OP54_ANY 0b00

#define CNVT_FP_FIXED_CNST0 0b0
#define CNVT_FP_FIXED_CNST1 0b11110
#define CNVT_FP_FIXED_CNST2 0b0

#define CNVT_FP_FIXED_OPCODE_S2F 0b010
#define CNVT_FP_FIXED_OPCODE_U2F 0b011
#define CNVT_FP_FIXED_OPCODE_F2S 0b000
#define CNVT_FP_FIXED_OPCODE_F2U 0b001

#define CNVT_FP_FIXED_PTYPE_SINGLE 0b00
#define CNVT_FP_FIXED_PTYPE_DOUBLE 0b01
#define CNVT_FP_FIXED_PTYPE_HALF   0b11

#define CNVT_FP_FIXED_RMODE_FPCR 0b00
#define CNVT_FP_FIXED_RMODE_ZERO 0b11

#define CNVT_FP_INT_CNST0 0b0
#define CNVT_FP_INT_CNST1 0b11110
#define CNVT_FP_INT_CNST2 0b1
#define CNVT_FP_INT_CNST3 0b000000

#define CNVT_FP_INT_OPCODE_F2S   0b000
#define CNVT_FP_INT_OPCODE_F2U   0b001
#define CNVT_FP_INT_OPCODE_S2F   0b010
#define CNVT_FP_INT_OPCODE_U2F   0b011
#define CNVT_FP_INT_OPCODE_MOV2R 0b110
#define CNVT_FP_INT_OPCODE_MOV2F 0b111

#define CNVT_FP_INT_PTYPE_SINGLE 0b00
#define CNVT_FP_INT_PTYPE_DOUBLE 0b01
#define CNVT_FP_INT_PTYPE_HALF   0b11

#define CNVT_FP_FIXED_RMODE_NEAR 0b00
#define CNVT_FP_FIXED_RMODE_NEG  0b10
#define CNVT_FP_FIXED_RMODE_POS  0b01
#define CNVT_FP_FIXED_RMODE_ZERO 0b11

#define PROC_FP_ONE_SOURCE_CNST0 0b0
#define PROC_FP_ONE_SOURCE_CNST1 0b11110
#define PROC_FP_ONE_SOURCE_CNST2 0b1
#define PROC_FP_ONE_SOURCE_CNST3 0b10000

#define PROC_FP_ONE_SOURCE_OPCODE_FMOV 0b000000
#define PROC_FP_ONE_SOURCE_OPCODE_FABS 0b000001
#define PROC_FP_ONE_SOURCE_OPCODE_FNEG 0b000010
#define PROC_FP_ONE_SOURCE_OPCODE_FSQRT 0b000011
#define PROC_FP_ONE_SOURCE_OPCODE_FTOS 0b000100
#define PROC_FP_ONE_SOURCE_OPCODE_FTOD 0b000101
#define PROC_FP_ONE_SOURCE_OPCODE_FTOH 0b000111

#define PROC_FP_ONE_SOURCE_PTYPE_SINGLE 0b00
#define PROC_FP_ONE_SOURCE_PTYPE_DOUBLE 0b01
#define PROC_FP_ONE_SOURCE_PTYPE_HALF   0b11

#define FP_CMP_CNST0 0b0
#define FP_CMP_CNST1 0b11110
#define FP_CMP_CNST2 0b1
#define FP_CMP_CNST3 0b1000

#define FP_CMP_OPCODE2_FCMP   0b00000
#define FP_CMP_OPCODE2_FCMPZ  0b01000
#define FP_CMP_OPCODE2_FCMPE  0b10000
#define FP_CMP_OPCODE2_FCMPZE 0b11000

#define FP_CMP_PTYPE_SINGLE 0b00
#define FP_CMP_PTYPE_DOUBLE 0b01
#define FP_CMP_PTYPE_HALF   0b11

#define FP_IMM_CNST0 0b0
#define FP_IMM_CNST1 0b11110
#define FP_IMM_CNST2 0b1
#define FP_IMM_CNST3 0b100

#define FP_IMM_PTYPE_SINGLE 0b00
#define FP_IMM_PTYPE_DOUBLE 0b01
#define FP_IMM_PTYPE_HALF   0b11

#define PROC_FP_TWO_SOURCE_CNST0 0b0
#define PROC_FP_TWO_SOURCE_CNST1 0b11110
#define PROC_FP_TWO_SOURCE_CNST2 0b1
#define PROC_FP_TWO_SOURCE_CNST3 0b10

#define PROC_FP_TWO_SOURCE_OPCODE_FMUL   0b0000
#define PROC_FP_TWO_SOURCE_OPCODE_FDIV   0b0001
#define PROC_FP_TWO_SOURCE_OPCODE_FADD   0b0010
#define PROC_FP_TWO_SOURCE_OPCODE_FSUB   0b0011
#define PROC_FP_TWO_SOURCE_OPCODE_FMAX   0b0100
#define PROC_FP_TWO_SOURCE_OPCODE_FMIN   0b0101
#define PROC_FP_TWO_SOURCE_OPCODE_FMAXNM 0b0110
#define PROC_FP_TWO_SOURCE_OPCODE_FMINNM 0b0111
#define PROC_FP_TWO_SOURCE_OPCODE_FNMUL  0b0111

#define PROC_FP_TWO_SOURCE_PTYPE_SINGLE 0b00
#define PROC_FP_TWO_SOURCE_PTYPE_DOUBLE 0b01
#define PROC_FP_TWO_SOURCE_PTYPE_HALF   0b11

#define PROC_FP_THREE_SOURCE_CNST0 0b0
#define PROC_FP_THREE_SOURCE_CNST1 0b11111

#define PROC_FP_THREE_SOURCE_PTYPE_SINGLE 0b00
#define PROC_FP_THREE_SOURCE_PTYPE_DOUBLE 0b01
#define PROC_FP_THREE_SOURCE_PTYPE_HALF   0b11

#define PROC_FP_THREE_SOURCE_O0_FMADD 0b0
#define PROC_FP_THREE_SOURCE_O0_FMSUB 0b1

typedef union {
    struct {
        uint32_t pad1 : 25;
        uint32_t op0 : 4;
        uint32_t pad0 : 3;
    } main;
    struct {
        uint32_t pad0 : 16;
        uint32_t op1 : 9;
        uint32_t cnst0 : 4;
        uint32_t op0 : 3;
    } reserved;
    struct {
        uint32_t pad1 : 23;
        uint32_t op0 : 3;
        uint32_t cnst1 : 3;
        uint32_t pad0 : 3;
    } proc_imm;
    struct {
        uint32_t rd : 5;
        uint32_t immhi : 19;
        uint32_t cnst0 : 5;
        uint32_t immlo : 2;
        uint32_t op : 1;
    } pc_rel;
    struct {
        uint32_t rd : 5;
        uint32_t rn : 5;
        uint32_t imm12 : 12;
        uint32_t sh : 1;
        uint32_t cnst0 : 6;
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
        uint32_t cnst0 : 6;
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
        uint32_t cnst0 : 6;
        uint32_t opc : 2;
        uint32_t sf : 1;
    } logical_imm;
    struct {
        uint32_t rd : 5;
        uint32_t imm16 : 16;
        uint32_t hw : 2;
        uint32_t cnst0 : 6;
        uint32_t opc : 2;
        uint32_t sf : 1;
    } move_imm;
    struct {
        uint32_t rd : 5;
        uint32_t rn : 5;
        uint32_t imms : 6;
        uint32_t immr : 6;
        uint32_t n : 1;
        uint32_t cnst0 : 6;
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
        uint32_t cnst0 : 6;
        uint32_t op21 : 2;
        uint32_t sf : 1;
    } extract;
    struct {
        uint32_t op2 : 5;
        uint32_t pad1 : 7;
        uint32_t op1 : 14;
        uint32_t cnst0 : 3;
        uint32_t op0 : 3;
    } branches;
    struct {
        uint32_t cond : 4;
        uint32_t o0 : 1;
        uint32_t imm19 : 19;
        uint32_t o1 : 1;
        uint32_t cnst0 : 7;
    } cond_branch_imm;
    struct {
        uint32_t ll : 2;
        uint32_t op2 : 3;
        uint32_t imm16 : 16;
        uint32_t opc : 3;
        uint32_t cnst0 : 8;
    } exception;
    struct {
        uint32_t cnst1 : 5;
        uint32_t op2 : 3;
        uint32_t crm : 4;
        uint32_t cnst0 : 20;
    } hints;
    struct {
        uint32_t rt : 5;
        uint32_t op2 : 3;
        uint32_t crm : 4;
        uint32_t cnst0 : 20;
    } barriers;
    struct {
        uint32_t rt : 5;
        uint32_t op2 : 3;
        uint32_t crm : 4;
        uint32_t cnst1 : 4;
        uint32_t op1 : 3;
        uint32_t cnst0 : 13;
    } pstate;
    struct {
        uint32_t rt : 5;
        uint32_t op2 : 3;
        uint32_t crm : 4;
        uint32_t crn : 4;
        uint32_t op1 : 3;
        uint32_t cnst1 : 2;
        uint32_t l : 1;
        uint32_t cnst0 : 10;
    } system_instr;
    struct {
        uint32_t rt : 5;
        uint32_t op2 : 3;
        uint32_t crm : 4;
        uint32_t crn : 4;
        uint32_t op1 : 3;
        uint32_t o0 : 1;
        uint32_t cnst1 : 1;
        uint32_t l : 1;
        uint32_t cnst0 : 10;
    } system_move;
    struct {
        uint32_t op4 : 5;
        uint32_t rn : 5;
        uint32_t op3 : 6;
        uint32_t op2 : 5;
        uint32_t opc : 4;
        uint32_t cnst0 : 7;
    } uncond_branch_reg;
    struct {
        uint32_t imm26 : 26;
        uint32_t cnst0 : 5;
        uint32_t op : 1;
    } uncond_branch_imm;
    struct {
        uint32_t rt : 5;
        uint32_t imm19 : 19;
        uint32_t op : 1;
        uint32_t cnst0 : 6;
        uint32_t sf : 1;
    } comp_branch_imm;
    struct {
        uint32_t rt : 5;
        uint32_t imm14 : 14;
        uint32_t b40 : 5;
        uint32_t op : 1;
        uint32_t cnst0 : 6;
        uint32_t b5 : 1;
    } test_branch_imm;
    struct {
        uint32_t pad2 : 10;
        uint32_t op4 : 2;
        uint32_t pad1 : 4;
        uint32_t op3 : 6;
        uint32_t pad0 : 1;
        uint32_t op2 : 2;
        uint32_t cnst1 : 1;
        uint32_t op1 : 1;
        uint32_t cnst0 : 1;
        uint32_t op0 : 4;
    } load_store;
    struct {
        uint32_t rt : 5;
        uint32_t imm19 : 19;
        uint32_t cnst1 : 2;
        uint32_t v : 1;
        uint32_t cnst0 : 3;
        uint32_t opc : 2;
    } load_reg_lit;
    struct {
        uint32_t rt : 5;
        uint32_t rn : 5;
        uint32_t cnst3 : 2;
        uint32_t imm9 : 9;
        uint32_t cnst2 : 1;
        uint32_t opc : 2;
        uint32_t cnst1 : 2;
        uint32_t v : 1;
        uint32_t cnst0 : 3;
        uint32_t size : 2;
    } load_store_reg_unsc_imm;
    struct {
        uint32_t rt : 5;
        uint32_t rn : 5;
        uint32_t cnst3 : 2;
        uint32_t imm9 : 9;
        uint32_t cnst2 : 1;
        uint32_t opc : 2;
        uint32_t cnst1 : 2;
        uint32_t v : 1;
        uint32_t cnst0 : 3;
        uint32_t size : 2;
    } load_store_reg_imm_post;
    struct {
        uint32_t rt : 5;
        uint32_t rn : 5;
        uint32_t cnst3 : 2;
        uint32_t imm9 : 9;
        uint32_t cnst2 : 1;
        uint32_t opc : 2;
        uint32_t cnst1 : 2;
        uint32_t v : 1;
        uint32_t cnst0 : 3;
        uint32_t size : 2;
    } load_store_reg_imm_pre;
    struct {
        uint32_t rt : 5;
        uint32_t rn : 5;
        uint32_t cnst3 : 2;
        uint32_t s : 1;
        uint32_t option : 3;
        uint32_t rm : 5;
        uint32_t cnst2 : 1;
        uint32_t opc : 2;
        uint32_t cnst1 : 2;
        uint32_t v : 1;
        uint32_t cnst0 : 3;
        uint32_t size : 2;
    } load_store_reg_reg;
    struct {
        uint32_t rt : 5;
        uint32_t rn : 5;
        uint32_t imm12 : 12;
        uint32_t opc : 2;
        uint32_t cnst1 : 2;
        uint32_t v : 1;
        uint32_t cnst0 : 3;
        uint32_t size : 2;
    } load_store_reg_unsi_imm;
    struct {
        uint32_t pad3 : 10;
        uint32_t op3 : 6;
        uint32_t pad2 : 5;
        uint32_t op2 : 4;
        uint32_t cnst0 : 3;
        uint32_t op1 : 1;
        uint32_t pad1 : 1;
        uint32_t op0 : 1;
        uint32_t pad0 : 1;
    } proc_reg;
    struct {
        uint32_t rd : 5;
        uint32_t rn : 5;
        uint32_t opcode : 6;
        uint32_t rm : 5;
        uint32_t cnst1 : 8;
        uint32_t s : 1;
        uint32_t cnst0 : 1;
        uint32_t sf : 1;
    } proc_reg_two_source;
    struct {
        uint32_t rd : 5;
        uint32_t rn : 5;
        uint32_t opcode : 6;
        uint32_t opcode2 : 6;
        uint32_t cnst1 : 8;
        uint32_t s : 1;
        uint32_t cnst0 : 1;
        uint32_t sf : 1;
    } proc_reg_one_source;
    struct {
        uint32_t rd : 5;
        uint32_t rn : 5;
        uint32_t imm6 : 6;
        uint32_t rm : 5;
        uint32_t n : 1;
        uint32_t shift : 2;
        uint32_t cnst0 : 5;
        uint32_t opc : 2;
        uint32_t sf : 1;
    } logical_shift_reg;
    struct {
        uint32_t rd : 5;
        uint32_t rn : 5;
        uint32_t imm6 : 6;
        uint32_t rm : 5;
        uint32_t cnst1 : 1;
        uint32_t shift : 2;
        uint32_t cnst0 : 5;
        uint32_t s : 1;
        uint32_t op : 1;
        uint32_t sf : 1;
    } add_shift_reg;
    struct {
        uint32_t rd : 5;
        uint32_t rn : 5;
        uint32_t imm3 : 3;
        uint32_t option : 3;
        uint32_t rm : 5;
        uint32_t cnst1 : 1;
        uint32_t opt : 2;
        uint32_t cnst0 : 5;
        uint32_t s : 1;
        uint32_t op : 1;
        uint32_t sf : 1;
    } add_ext_reg;
    struct {
        uint32_t rd : 5;
        uint32_t rn : 5;
        uint32_t cnst1 : 6;
        uint32_t rm : 5;
        uint32_t cnst0 : 8;
        uint32_t s : 1;
        uint32_t op : 1;
        uint32_t sf : 1;
    } add_carry;
    struct {
        uint32_t rd : 5;
        uint32_t rn : 5;
        uint32_t ra : 5;
        uint32_t o0 : 1;
        uint32_t rm : 5;
        uint32_t op31 : 3;
        uint32_t cnst0 : 5;
        uint32_t op54 : 2;
        uint32_t sf : 1;
    } proc_reg_three_source;
    struct {
        uint32_t pad0 : 10;
        uint32_t op3 : 9;
        uint32_t op2 : 4;
        uint32_t op1 : 2;
        uint32_t cnst0 : 3;
        uint32_t op0 : 4;
    } proc_fp;
    struct {
        uint32_t rd : 5;
        uint32_t rn : 5;
        uint32_t scale : 6;
        uint32_t opcode : 3;
        uint32_t rmode : 2;
        uint32_t cnst2 : 1;
        uint32_t ptyoe : 2;
        uint32_t cnst1 : 5;
        uint32_t s : 1;
        uint32_t cnst0 : 1;
        uint32_t sf : 1;
    } cnvt_fp_fixed;
    struct {
        uint32_t rd : 5;
        uint32_t rn : 5;
        uint32_t cnst3 : 6;
        uint32_t opcode : 3;
        uint32_t rmode : 2;
        uint32_t cnst2 : 1;
        uint32_t ptype : 2;
        uint32_t cnst1 : 5;
        uint32_t s : 1;
        uint32_t cnst0 : 1;
        uint32_t sf : 1;
    } cnvt_fp_int;
    struct {
        uint32_t rd : 5;
        uint32_t rn : 5;
        uint32_t cnst3 : 5;
        uint32_t opcode : 6;
        uint32_t cnst2 : 1;
        uint32_t ptype : 2;
        uint32_t cnst1 : 5;
        uint32_t s : 1;
        uint32_t cnst0 : 1;
        uint32_t m : 1;
    } proc_fp_one_source;
    struct {
        uint32_t opcode2 : 5;
        uint32_t rn : 5;
        uint32_t cnst3 : 4;
        uint32_t op : 2;
        uint32_t rm : 5;
        uint32_t cnst2 : 1;
        uint32_t ptype : 2;
        uint32_t cnst1 : 5;
        uint32_t s : 1;
        uint32_t cnst0 : 1;
        uint32_t m : 1;
    } fp_cmp;
    struct {
        uint32_t rd : 5;
        uint32_t imm5 : 5;
        uint32_t cnst3 : 3;
        uint32_t imm8 : 8;
        uint32_t cnst2 : 1;
        uint32_t ptype : 2;
        uint32_t cnst1 : 5;
        uint32_t s : 1;
        uint32_t cnst0 : 1;
        uint32_t m : 1;
    } fp_imm;
    struct {
        uint32_t rd : 5;
        uint32_t rn : 5;
        uint32_t cnst3 : 2;
        uint32_t opcode : 4;
        uint32_t rm : 5;
        uint32_t cnst2 : 1;
        uint32_t ptype : 2;
        uint32_t cnst1 : 5;
        uint32_t s : 1;
        uint32_t cnst0 : 1;
        uint32_t m : 1;
    } proc_fp_two_source;
    struct {
        uint32_t rd : 5;
        uint32_t rn : 5;
        uint32_t ra : 5;
        uint32_t o0 : 1;
        uint32_t rm : 5;
        uint32_t o1 : 1;
        uint32_t ptype : 2;
        uint32_t cnst1 : 5;
        uint32_t s : 1;
        uint32_t cnst0 : 1;
        uint32_t m : 1;
    } proc_fp_three_source;
    uint32_t instruction;
} Aarch64Instruction;

void addInstruction(StackAllocator* mem, Aarch64Instruction instr);

void updateInstruction(StackAllocator* mem, size_t pos, Aarch64Instruction instr);

Aarch64Instruction getInstruction(StackAllocator* mem, size_t pos);

#endif