#ifndef _CODEGEN_H_
#define _CODEGEN_H_

#include "common/stackalloc.h"
#include "parser/ast.h"
#include "codegen/variabletable.h"
#include "codegen/labellist.h"
#include "codegen/instructions.h"
#include "codegen/datalist.h"

typedef struct {
    StackAllocator* inst_mem;
    StackAllocator* variable_mem;
    VariableTable* variable_table;
    VariableTable* label_table;
    UnhandeledLabelList* label_list;
    DataList* data_mem;
    RegisterSet registers;
    int line;
} MCGenerationData;

typedef enum {
    VALUE_ERROR,
    VALUE_NONE,
    VALUE_INT,
    VALUE_FLOAT,
    VALUE_STRING,
} ValueType;

typedef enum {
    ERROR_NONE,
    ERROR_SYNTAX,
    ERROR_TYPE,
    ERROR_VARIABLE_NOT_DEF,
    ERROR_NO_MATCHING_FOR,
    ERROR_ARRAY_NOT_DEF,
    ERROR_ARRAY_DIM_COUNT_MISMATCH,
    ERROR_DUBLICATE_LABEL,
    ERROR_UNINDEXED_ARRAY,
} Error;

typedef struct {
    ValueType type;
    union {
        Error error;
        Register reg;
    };
} Value;

Value generateMCForAst(Ast* ast, MCGenerationData* data);

Error generateMC(Ast* ast, MCGenerationData* data);

#endif