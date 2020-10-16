
#include <stdlib.h>

#include "codegen/labellist.h"

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
