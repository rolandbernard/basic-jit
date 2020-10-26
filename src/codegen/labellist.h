#ifndef _LABEL_LIST_H_
#define _LABEL_LIST_H_

#include <stddef.h>
#include <stdbool.h>

#include "codegen/variabletable.h"

typedef struct {
    char* name;
    int line;
    size_t position;
    bool for_restore;
} UnhandeledLabelEntry;

typedef struct {
    size_t count;
    size_t capacity;
    UnhandeledLabelEntry* data;
} UnhandeledLabelList;

#define UNHANDLED_LABEL_LIST_INITIALIZER { .count = 0, .capacity = 0, .data = NULL }

void addLabelToList(UnhandeledLabelList* list, UnhandeledLabelEntry entry);

void freeLabelList(UnhandeledLabelList* list);

void fillUnhandledLabelLocations(UnhandeledLabelList* list, VariableTable* table, StackAllocator* inst_mem);

#endif