#ifndef _SYMBOLTABLE_H_
#define _SYMBOLTABLE_H_

#include <stdint.h>

#include "common/stackalloc.h"

#define VARIABLE_BASE VariableType type;

typedef enum {
    VARIABLE_INT,
    VARIABLE_FLOAT,
    VARIABLE_STRING,
    VARIABLE_INT_ARRAY,
    VARIABLE_FLOAT_ARRAY,
    VARIABLE_STRING_ARRAY,
} VariableType;

typedef struct {
    VARIABLE_BASE
} Variable;

typedef struct {
    VARIABLE_BASE
    int64_t value;
} VariableInt;

typedef struct {
    VARIABLE_BASE
    double value;
} VariableFloat;

typedef struct {
    VARIABLE_BASE
    char* str;
} VariableString;

typedef struct {
    VARIABLE_BASE
    int dim_count;
    int64_t* size;
    int64_t* value;
} VariableIntArray;

typedef struct {
    VARIABLE_BASE
    int dim_count;
    int64_t* size;
    double* value;
} VariableFloatArray;

typedef struct {
    VARIABLE_BASE
    int dim_count;
    int64_t* size;
    char** str;
} VariableStringArray;

typedef struct {
    char* key;
    Variable* value;
} VariableTableEntry;

typedef struct {
    int count;
    int capacity;
    VariableTableEntry* data;
} VariableTable;

#define VARIABLE_TABLE_INITIALIZER { .count = 0, .capacity = 0, .data = NULL }

void addVariable(VariableTable* table, const char* name, Variable* variable, StackAllocator* mem);

Variable* getVariable(VariableTable* table, const char* name);

#endif