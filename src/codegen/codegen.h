#ifndef _CODEGEN_H_
#define _CODEGEN_H_

#include "common/stackalloc.h"
#include "parser/ast.h"
#include "codegen/variabletable.h"
#include "codegen/labellist.h"
#include "codegen/instructions.h"
#include "codegen/datalist.h"
#include "codegen/value.h"

typedef struct {
    StackAllocator* inst_mem;
    StackAllocator* variable_mem;
    VariableTable* variable_table;
    VariableTable* label_table;
    VariableTable* func_table;
    UnhandeledLabelList* label_list;
    DataList* data_mem;
    RegisterSet registers;
    int line;
    const char* filename;
} MCGenerationData;

typedef Value (*GenerateMCFunction)(Ast*, MCGenerationData*);

Value generateMCForAst(Ast* ast, MCGenerationData* data);

Error generateMC(Ast* ast, MCGenerationData* data);

Value withFreeRegister(Ast* ast, MCGenerationData* data, GenerateMCFunction func, int num_regs, int num_fregs);

#endif
