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
} UnhandledLabelEntry;

typedef struct {
    size_t count;
    size_t capacity;
    UnhandledLabelEntry* data;
} UnhandledLabelList;

#define UNHANDLED_LABEL_LIST_INITIALIZER { .count = 0, .capacity = 0, .data = NULL }

void addLabelToList(UnhandledLabelList* list, UnhandledLabelEntry entry);

void freeLabelList(UnhandledLabelList* list);

int fillUnhandledLabelLocations(UnhandledLabelList* list, VariableTable* table, StackAllocator* inst_mem);

void resetLabelList(UnhandledLabelList* list);

#endif
