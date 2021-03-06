
#include <stdlib.h>

#include "codegen/labellist.h"
#include "codegen/instructions.h"

void addLabelToList(UnhandeledLabelList* list, UnhandeledLabelEntry entry) {
    if(list->count == list->capacity) {
        if(list->capacity == 0) {
            list->capacity = 16;
        } else {
            list->capacity *= 2;
        }
        list->data = (UnhandeledLabelEntry*)realloc(list->data, sizeof(UnhandeledLabelEntry) * list->capacity);
    }
    list->data[list->count] = entry;
    list->count++;
}

void freeLabelList(UnhandeledLabelList* list) {
    if(list != NULL) {
        free(list->data);
    }
}

int fillUnhandledLabelLocations(UnhandeledLabelList* list, VariableTable* table, StackAllocator* inst_mem) {
    for(size_t i = 0; i < list->count; i++) {
        VariableLabel* label = (VariableLabel*)getVariable(table, list->data[i].name);
        if(label == NULL || label->type != VARIABLE_LABEL) {
            return i;
        } else {
            if(list->data[i].for_restore) {
                updateImmediateValue(inst_mem, list->data[i].position, label->data_pos);
            } else {
                updateRelativeJumpTarget(inst_mem, list->data[i].position, label->pos);
            }
        }
    }
    list->count = 0;
    return -1;
}

void resetLabelList(UnhandeledLabelList* list) {
    list->count = 0;
}
