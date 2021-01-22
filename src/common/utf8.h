#ifndef _UTF8_H_
#define _UTF8_H_

#include <stdint.h>

int writeUTF8(uint64_t c, char* out);

int getLengthUTF8(uint64_t c);

int parseUTF8(char* str, uint64_t* out);

#endif