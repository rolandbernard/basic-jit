
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "exec/fileexec.h"
#include "common/stackalloc.h"
#include "codegen/codegen.h"
#include "codegen/datalist.h"
#include "codegen/labellist.h"
#include "codegen/variabletable.h"
#include "parser/parser.h"
#include "exec/executil.h"
#include "exec/execalloc.h"

#define MAX_LINE_BUFFER (1 << 20)
static char line_buffer[MAX_LINE_BUFFER];

int executeFile(const char* filename) {
    int exit_code = EXIT_SUCCESS;
    FILE* file = fopen(filename, "r");
    if(file == NULL) {
        fprintf(stderr, "error: Failed to open the file '%s': %s\n", filename, strerror(errno));
        return EXIT_FAILURE;
    } else {
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
            .line = 1,
        };
        addInstPushCallerRegs(data.inst_mem, data.registers);
        bool had_error = false;
        while(!had_error && fgets(line_buffer, MAX_LINE_BUFFER, file) != NULL) {
            size_t len = strlen(line_buffer);
            if(line_buffer[len - 1] == '\n') {
                line_buffer[len - 1] = 0;
            }
            Ast* ast = parseLine(line_buffer, &ast_memory);
            if(ast != NULL) {
                if(ast->type == AST_ERROR) {
                    AstError* error = (AstError*)ast;
                    fprintf(stderr, "error: Syntax error at line %i:%i\n", data.line, error->offset + 1);
                    fprintf(stderr, " | %s\n", line_buffer);
                    fprintf(stderr, " | ");
                    for(size_t i = 0; i < error->offset; i++) {
                        if(line_buffer[i] == '\t') {
                            fputc('\t', stderr);
                        } else {
                            fputc(' ', stderr);
                        }
                    }
                    fputc('^', stderr);
                    fputc('\n', stderr);
                    had_error = true;
                    exit_code = EXIT_FAILURE;
                } else {
                    Error error = generateMC(ast, &data);
                    if(error != ERROR_NONE) {
                        fprintf(stderr, "error: %s at line %i\n", getErrorName(error), data.line);
                        had_error = true;
                        exit_code = EXIT_FAILURE;
                    }
                }
            }
            resetStack(&ast_memory);
            data.line++;
        }
        fclose(file);
        if(!had_error) {
            addInstPopCallerRegs(data.inst_mem, data.registers);
            addInstReturn(data.inst_mem, data.registers);
            int err = fillUnhandledLabelLocations(data.label_list, data.label_table, data.inst_mem);
            if(err >= 0) {
                fprintf(stderr, "error: Unresolved label %s at line %i\n", label_list.data[err].name, label_list.data[err].line);
                exit_code = EXIT_FAILURE;
            } else {
                int ret;
#ifdef DEBUG
                printMemoryContent(stderr, jit_memory.memory, jit_memory.occupied);
#endif
                if(executeFunctionInMemory(jit_memory.memory, jit_memory.occupied, &ret)) {
                    perror("error: Failed to execute");
                    exit_code = EXIT_FAILURE;
                } else if (ret != EXIT_NORMAL) {
                    exit_code = ret;
                }
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
        return exit_code;
    }
}
