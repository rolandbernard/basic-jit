
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "exec/fileexec.h"
#include "common/stackalloc.h"
#include "codegen/datalist.h"
#include "codegen/labellist.h"
#include "codegen/variabletable.h"
#include "parser/parser.h"
#include "exec/executil.h"
#include "exec/execalloc.h"

#define INITIAL_LINE_BUFFER 32
static char* line_buffer;
static int line_buffer_capacity = 0;

int generateMcIntoData(const char* filename, MCGenerationData* data) {
    FILE* file = fopen(filename, "r");
    if(file == NULL) {
        fprintf(stderr, "error: Failed to open the file '%s': %s\n", filename, strerror(errno));
        return EXIT_FAILURE;
    } else {
        StackAllocator ast_memory = STACK_ALLOCATOR_INITIALIZER;
        int exit_code = EXIT_SUCCESS;
        bool had_error = false;
        bool end_of_file = false;
        while(!had_error && !end_of_file) {
            int next_char; 
            int len = 0;
            do {
                next_char = fgetc(file);
                if (len == line_buffer_capacity) {
                    if (line_buffer_capacity == 0) {
                        line_buffer_capacity = INITIAL_LINE_BUFFER;
                        line_buffer = (char*)malloc(sizeof(char) * line_buffer_capacity);
                    } else {
                        line_buffer_capacity *= 2;
                        line_buffer = (char*)realloc(line_buffer, sizeof(char) * line_buffer_capacity);
                    }
                }
                if (next_char == '\n' && len > 0 && line_buffer[len - 1] == '\\') {
                    line_buffer[len - 1] = next_char;
                    next_char = '\\';
                } else {
                    line_buffer[len] = next_char;
                    len++;
                }
            } while (next_char != '\n' && next_char != EOF);
            len--;
            line_buffer[len] = 0;
            if (next_char == EOF) {
                end_of_file = true;
            } else {
                Ast* ast = parseLine(line_buffer, &ast_memory);
                if (ast != NULL) {
                    if (ast->type == AST_ERROR) {
                        AstError* error = (AstError*)ast;
                        fprintf(stderr, "error: Syntax error at %s:%i:%i\n", filename, data->line, error->offset + 1);
                        fprintf(stderr, " | %s\n", line_buffer);
                        fprintf(stderr, " | ");
                        for (int i = 0; i < error->offset; i++) {
                            if (line_buffer[i] == '\t') {
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
                        Error error= generateMC(ast, data);
                        if (error != ERROR_NONE) {
                            fprintf(stderr, "error: %s at %s:%i\n", getErrorName(error), filename, data->line);
                            had_error = true;
                            exit_code = EXIT_FAILURE;
                        }
                    }
                }
                resetStack(&ast_memory);
                data->line++;
            }
        }
        fclose(file);
        freeStack(&ast_memory);
        return exit_code;
    }
}

int executeFile(const char* filename) {
    DataList data_list = DATA_LIST_INITIALIZER;
    StackAllocator var_memory = STACK_ALLOCATOR_INITIALIZER;
    VariableTable var_table = VARIABLE_TABLE_INITIALIZER;
    StackAllocator jit_memory = STACK_ALLOCATOR_INITIALIZER;
    VariableTable label_table = VARIABLE_TABLE_INITIALIZER;
    UnhandeledLabelList label_list = UNHANDLED_LABEL_LIST_INITIALIZER;
    VariableTable func_table = VARIABLE_TABLE_INITIALIZER;
    MCGenerationData data = {
        .inst_mem = &jit_memory,
        .variable_mem = &var_memory,
        .variable_table = &var_table,
        .label_table = &label_table,
        .label_list = &label_list,
        .data_mem = &data_list,
        .func_table = &func_table,
        .registers = 0,
        .line = 1,
    };
    addInstPushCallerRegs(data.inst_mem, data.registers);
    int exit_code = generateMcIntoData(filename, &data);
    if(exit_code == EXIT_SUCCESS) {
        addInstPopCallerRegs(data.inst_mem, data.registers);
        addInstReturn(data.inst_mem, data.registers);
        int err = fillUnhandledLabelLocations(data.label_list, data.label_table, data.inst_mem);
        if(err >= 0) {
            fprintf(stderr, "error: Unresolved label %s at line %i\n", data.label_list->data[err].name, data.label_list->data[err].line);
            exit_code = EXIT_FAILURE;
        } else {
            int ret;
#ifdef DEBUG
            printMemoryContent(stderr, jit_memory.memory, jit_memory.occupied);
#endif
            executeFunctionInMemory(data.inst_mem->memory, data.inst_mem->occupied, &ret);
            if (ret > EXIT_SIGNAL_ERROR_START) {
                fprintf(stderr, "error: Child terminated: %s\n", strsignal(ret - EXIT_SIGNAL_ERROR_START));
            } else {
                switch (ret) {
                    case EXIT_FORK_ERROR:
                        perror("error: Failed to fork");
                        break;
                    case EXIT_MEM_ERROR:
                        perror("error: Failed to set memory protection");
                        break;
                    case EXIT_OTHER_ERROR:
                        fprintf(stderr, "error: Child terminated unexpectedly\n");
                        break;
                    case EXIT_NORMAL:
                        ret = 0;
                        break;
                    default:
                        break;
                }
            }
            exit_code = ret;
        }
    }
    freeLabelList(&label_list);
    freeDataList(&data_list);
    freeVariableTable(&var_table);
    freeVariableTable(&label_table);
    freeVariableTable(&func_table);
    freeStack(&var_memory);
    freeStack(&jit_memory);
    freeStack(&global_exec_alloc);
    return exit_code;
}
