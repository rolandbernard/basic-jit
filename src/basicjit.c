
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "common/stackalloc.h"
#include "codegen/codegen.h"
#include "codegen/datalist.h"
#include "codegen/labellist.h"
#include "codegen/variabletable.h"
#include "parser/parser.h"
#include "exec/executil.h"
#include "exec/execalloc.h"
#include "codegen/x86-64.h"

int main(int argc, char** argv) {
    StackAllocator ast_memory = STACK_ALLOCATOR_INITIALIZER;

    DataList data_list = DATA_LIST_INITIALIZER;
    
    StackAllocator var_memory = STACK_ALLOCATOR_INITIALIZER;
    VariableTable var_table = VARIABLE_TABLE_INITIALIZER;

    StackAllocator jit_memory = STACK_ALLOCATOR_INITIALIZER;

    VariableTable label_table = VARIABLE_TABLE_INITIALIZER;
    UnhandeledLabelList label_list = UNHANDLED_LABEL_LIST_INITIALIZER;

    MCGenerationData data = {
        .inst_mem = &jit_memory,
        .variable_mem = &var_memory,
        .variable_table = &var_table,
        .label_table = &label_table,
        .label_list = &label_list,
        .data_mem = &data_list,
        .registers = 0,
        .line = 10,
    };
    Ast* ast = parseLine("Print Left(\"Test\", 2)", &ast_memory);
    addInstPushCallerRegs(data.inst_mem, data.registers);
    Error error = generateMC(ast, &data);
    fprintf(stderr, "Error: %s\n", getErrorName(error));
    addInstMovImmToReg(data.inst_mem, data.registers, REG_A, 0, false);
    addInstPopCallerRegs(data.inst_mem, data.registers);
    addInstReturn(data.inst_mem, data.registers);

    printMemoryContent(stderr, jit_memory.memory, jit_memory.occupied);

    if(error == ERROR_NONE) {
        int ret = 0;
        if (executeFunctionInMemory(jit_memory.memory, jit_memory.occupied, &ret)) {
            perror("Failed to execute");
        }
    }
    
    freeLabelList(&label_list);
    freeDataList(&data_list);
    freeVariableTable(&var_table);
    freeVariableTable(&label_table);
    freeStack(&var_memory);
    freeStack(&ast_memory);
    freeStack(&jit_memory);
    freeStack(&global_exec_alloc);
    return 0;
}