
#include "common/utf8.h"

int writeUTF8(uint64_t c, char* out) {
    if(c <= 0x7f) {
        out[0] = (char)c;
        return 1;
    } else {
        int i = 1;
        while ((c >> ((i - 1) * 6 + (7 - i))) > 0 && i < 8) {
            i++;
        }
        out[0] = 0;
        for(int j = 0; j < i; j++) {
            out[0] |= (1 << (7 - j));
        }
        out[0] |= (c >> ((i - 1) * 6)); 
        for(int j = 1; j < i; j++) {
            out[j] = 0x80 | ((c >> ((i - j - 1) * 6)) & 0x3f);
        }
        return i;
    }
}

int getLengthUTF8(uint64_t c) {
    if(c <= 0x7f) {
        return 1;
    } else {
        int i = 1;
        while ((c >> ((i - 1) * 6 + (7 - i))) > 0 && i < 8) {
            i++;
        }
        return i;
    }
}

int parseUTF8(char* str, uint64_t* out) {
    if ((uint8_t)str[0] <= 0x7f) {
        *out = str[0];
        return 1;
    } else {
        int i = 1;
        while ((str[0] & (1 << (7 - i))) != 0 && i < 8) {
            i++;
        }
        *out = 0;
        *out |= (uint64_t)((uint8_t)str[0] & ((uint8_t)(~0) >> i)) << ((i - 1) * 6);
        for(int j = 1; j < i; j++) {
            *out |= ((uint64_t)str[j] & 0x3f) << ((i - j - 1) * 6);
        }
        return i;
    }
}

