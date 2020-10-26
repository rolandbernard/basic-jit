
#include <stdlib.h>

#include "codegen/datalist.h"

void addDataToList(DataList* list, DataElement entry) {
    if(list->count == list->capacity) {
        if(list->capacity == 0) {
            list->capacity = 16;
        } else {
            list->capacity *= 2;
        }
        list->data = (DataElement*)realloc(list->data, sizeof(DataElement) * list->capacity);
    }
    list->data[list->count] = entry;
    list->count++;
}

void freeDataList(DataList* list) {
    if(list != NULL) {
        free(list->data);
    }
}

void resetDataList(DataList* list) {
    list->count = 0;
}
