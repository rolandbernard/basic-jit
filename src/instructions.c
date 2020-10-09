
#include "instructions.h"
    
int addJmpRelative32(uint8_t* mem, int32_t value) {
    mem[0] = 0xe9;
    mem[1] = (value) & 0xff;
    mem[2] = (value >> 8) & 0xff;
    mem[3] = (value >> 16) & 0xff;
    mem[4] = (value >> 24) & 0xff;
    return 5;
}

int addMovImm32ToReg(uint8_t* mem, Register reg, int32_t value) {
    mem[0] = 0xb8 + reg;
    mem[1] = (value) & 0xff;
    mem[2] = (value >> 8) & 0xff;
    mem[3] = (value >> 16) & 0xff;
    mem[4] = (value >> 24) & 0xff;
    return 5;
}

int addRetN(uint8_t* mem) {
    mem[0] = 0xc3;
    return 1;
}
