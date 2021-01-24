#ifndef _SYMBOLTABLE_H_
#define _SYMBOLTABLE_H_

#include <stdint.h>

#include "common/stackalloc.h"
#include "codegen/value.h"
#include "parser/ast.h"

#define VARIABLE_BASE VariableType type;

typedef enum {
    VARIABLE_INT,
    VARIABLE_FLOAT,
    VARIABLE_STRING,
    VARIABLE_BOOLEAN,
    VARIABLE_INT_ARRAY,
    VARIABLE_FLOAT_ARRAY,
    VARIABLE_STRING_ARRAY,
    VARIABLE_BOOLEAN_ARRAY,
    VARIABLE_LABEL,
    VARIABLE_FUNC,
} VariableType;

typedef struct {
    VARIABLE_BASE
} Variable;

typedef struct {
    VARIABLE_BASE
    int64_t value;
    size_t for_call_loc;
    size_t for_jmp_loc;
} VariableInt;

typedef struct {
    VARIABLE_BASE
    double value;
    size_t for_call_loc;
    size_t for_jmp_loc;
} VariableFloat;

typedef struct {
    VARIABLE_BASE
    char* str;
} VariableString;

typedef struct {
    VARIABLE_BASE
    int64_t value;
} VariableBoolean;

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
    VARIABLE_BASE
    int dim_count;
    int64_t* size;
    int64_t* value;
} VariableBooleanArray;

typedef struct {
    VARIABLE_BASE
    size_t pos;
    size_t data_pos;
} VariableLabel;

typedef struct {
    VARIABLE_BASE
    size_t pos;
    VarType param_type;
    ValueType return_type;
} VariableFunc;

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

void removeVariable(VariableTable* table, const char* name);

Variable* getVariable(VariableTable* table, const char* name);

void freeVariableTable(VariableTable* table);

void resetVariableTable(VariableTable* table);

#endif