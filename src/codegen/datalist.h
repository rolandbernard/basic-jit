#ifndef _DATA_LIST_H_
#define _DATA_LIST_H_

#include <stdint.h>
#include <stddef.h>

typedef struct {
    int64_t integer;
    double real;
    const char* string;
} DataElement;

typedef struct {
    size_t count;
    size_t capacity;
    DataElement* data;
} DataList;

#define DATA_LIST_INITIALIZER { .count = 0, .capacity = 0, .data = NULL }

void addDataToList(DataList* list, DataElement entry);

void freeDataList(DataList* list);

void resetDataList(DataList* list);

#endif
