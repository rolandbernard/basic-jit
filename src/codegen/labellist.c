
#include <stdlib.h>

#include "codegen/labellist.h"
#include "codegen/instructions.h"

void addLabelToList(UnhandledLabelList* list, UnhandledLabelEntry entry) {
    if(list->count == list->capacity) {
        if(list->capacity == 0) {
            list->capacity = 16;
        } else {
            list->capacity *= 2;
        }
        list->data = (UnhandledLabelEntry*)realloc(list->data, sizeof(UnhandledLabelEntry) * list->capacity);
    }
    list->data[list->count] = entry;
    list->count++;
}

void freeLabelList(UnhandledLabelList* list) {
    if(list != NULL) {
        free(list->data);
    }
}

int fillUnhandledLabelLocations(UnhandledLabelList* list, VariableTable* table, StackAllocator* inst_mem) {
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

void resetLabelList(UnhandledLabelList* list) {
    list->count = 0;
}
