
#include <string.h>

#include "codegen/variabletable.h"

static unsigned long hashString(const char* str) {
    unsigned long hash = 6151;
    while(*str) {
        hash = hash * 769 + *str;
        str++;
    }
    return hash;
}

static void insertIntoData(VariableTableEntry* data, int capacity, char* key, Variable* value) {
    // capacity will always be a power of two, meaning `& (capacity - 1)` is equal to `% capacity`
    int index = hashString(key) & (capacity - 1);
    while(data[index].key != NULL) {
        index = (index + 1) & (capacity - 1);
    }
    data[index].key = key;
    data[index].value = value;
}

static int findEntry(const VariableTableEntry* data, int capacity, const char* key) {
    if(capacity != 0) {
        int index = hashString(key) & (capacity - 1);
        while (data[index].key != NULL) {
            if (strcmp(data[index].key, key) == 0) {
                return index;
            }
            index = (index + 1) & (capacity - 1);
        }
    }
    return -1;
}

static void rehashEntrys(VariableTable* ht, VariableTableEntry* new_data, int new_capacity) {
    for(int i = 0; i < ht->capacity; i++) {
        if(ht->data[i].key != NULL) {
            insertIntoData(new_data, new_capacity, ht->data[i].key, ht->data[i].value);
        }
    }
}

void addVariable(VariableTable* table, const char* name, Variable* variable, StackAllocator* mem) {
    if (table->count * 3 >= table->capacity * 2) {
        if (table->capacity == 0) {
            table->capacity = 32;
            table->data = (VariableTableEntry*)calloc(table->capacity, sizeof(VariableTableEntry));
        } else {
            int new_capacity = table->capacity * 2;
            VariableTableEntry* new_data = (VariableTableEntry*)calloc(new_capacity, sizeof(VariableTableEntry));
            rehashEntrys(table, new_data, new_capacity);
            free(table->data);
            table->capacity = new_capacity;
            table->data = new_data;
        }
    }
    int index = findEntry(table->data, table->capacity, name);
    if (index == -1) {
        int len = strlen(name);
        char* key_copy = (char*)alloc_aligned(mem, len + 1);
        memcpy(key_copy, name, len + 1);
        insertIntoData(table->data, table->capacity, key_copy, variable);
        table->count++;
    } else {
        table->data[index].value = variable;
    }
}

Variable* getVariable(VariableTable* table, const char* name) {
    int index = findEntry(table->data, table->capacity, name);
    Variable* ret = NULL;
    if (index != -1) {
        ret = table->data[index].value;
    }
    return ret;
}

void freeVariableTable(VariableTable* table) {
    free(table->data);
}
