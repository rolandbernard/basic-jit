#ifndef _INSTRUCTIONS_H_
#define _INSTRUCTIONS_H_

#include <stdint.h>

typedef enum {
    REG_A = 0,
    REG_C = 1,
    REG_D = 2,
    REG_B = 3,
} Register;

int addJmpRelative32(uint8_t* mem, int32_t value);

int addMovImm32ToReg(uint8_t* mem, Register reg, int32_t value);

int addRetN(uint8_t* mem);

int addPush(uint8_t* mem, Register reg);

int addPop(uint8_t* mem, Register reg);

#endif