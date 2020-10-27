
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

#include "exec/cli.h"
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

#define MAX_LINES (1 << 20)
static int64_t line_numbers[MAX_LINES];
static char* lines[MAX_LINES];
static size_t num_lines = 0;
    
static StackAllocator ast_memory = STACK_ALLOCATOR_INITIALIZER;
static DataList data_list = DATA_LIST_INITIALIZER;
static StackAllocator var_memory = STACK_ALLOCATOR_INITIALIZER;
static VariableTable var_table = VARIABLE_TABLE_INITIALIZER;
static StackAllocator jit_memory = STACK_ALLOCATOR_INITIALIZER;
static VariableTable label_table = VARIABLE_TABLE_INITIALIZER;
static UnhandeledLabelList label_list = UNHANDLED_LABEL_LIST_INITIALIZER;

static int findLine(int64_t line_number) {
    int i = 0, j = num_lines;
    while (i != j) {
        int h = (i + j) / 2;
        if(line_numbers[h] > line_number) {
            j = h;
        } else if(line_numbers[h] < line_number) {
            i = h + 1;
        } else {
            i = h;
            j = h;
        }
    }
    return i;
}

static void addLine(const char* line, int64_t line_number) {
    int i = findLine(line_number);
    if(i < num_lines && line_numbers[i] == line_number) {
        free(lines[i]);
        size_t len = strlen(line);
        lines[i] = malloc(len + 1);
        memcpy(lines[i], line, len + 1);
    } else {
        memmove(lines + i + 1, lines + i, (num_lines - i) * sizeof(char*));
        memmove(line_numbers + i + 1, line_numbers + i, (num_lines - i) * sizeof(int64_t));
        num_lines++;
        size_t len = strlen(line);
        lines[i] = malloc(len + 1);
        memcpy(lines[i], line, len + 1);
        line_numbers[i] = line_number;
    }
}

static void removeLine(int64_t line_number) {
    int i = findLine(line_number);
    if(i < num_lines && line_numbers[i] == line_number) {
        free(lines[i]);
        num_lines--;
        memmove(lines + i, lines + i + 1, (num_lines - i) * sizeof(char*));
        memmove(line_numbers + i, line_numbers + i + 1, (num_lines - i) * sizeof(int64_t));
    }
}

static void listLine(int64_t line_number) {
    int i = findLine(line_number);
    if(i < num_lines && line_numbers[i] == line_number) {
        fprintf(stdout, "%s\n", lines[i]);
    }
}

static void listAllLines() {
    for(int i = 0; i < num_lines; i++) {
        fprintf(stdout, "%s\n", lines[i]);
    }
}

static void removeAllLines() {
    for(int i = 0; i < num_lines; i++) {
        free(lines[i]);
    }
    num_lines = 0;
}

void intHandler(int dummy) {
    fprintf(stdout, "\n");
}

static void runProgram() {
    resetLabelList(&label_list);
    resetDataList(&data_list);
    resetVariableTable(&var_table);
    resetVariableTable(&label_table);
    resetStack(&var_memory);
    resetStack(&ast_memory);
    resetStack(&jit_memory);
    resetStack(&global_exec_alloc);
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
    for(int i = 0; i < num_lines && !had_error; i++) {
        Ast* ast = parseLine(lines[i], &ast_memory);
        if(ast != NULL) {
            if(ast->type == AST_ERROR) {
                AstError* error = (AstError*)ast;
                fprintf(stderr, "error: Syntax error at line %i:%i\n", data.line, error->offset + 1);
                fprintf(stderr, " | %s\n", lines[i]);
                fprintf(stderr, " | ");
                for(size_t i = 0; i < error->offset; i++) {
                    if(lines[i][i] == '\t') {
                        fputc('\t', stderr);
                    } else {
                        fputc(' ', stderr);
                    }
                }
                fputc('^', stderr);
                fputc('\n', stderr);
                had_error = true;
            } else {
                Error error = generateMC(ast, &data);
                if(error != ERROR_NONE) {
                    fprintf(stderr, "error: %s at line %i\n", getErrorName(error), data.line);
                    had_error = true;
                }
            }
        }
        resetStack(&ast_memory);
        data.line++;
    }
    if(!had_error) {
        addInstPopCallerRegs(data.inst_mem, data.registers);
        addInstReturn(data.inst_mem, data.registers);
        int err = fillUnhandledLabelLocations(data.label_list, data.label_table, data.inst_mem);
        if(err >= 0) {
            fprintf(stderr, "error: Unresolved label %s at line %i\n", label_list.data[err].name, label_list.data[err].line);
        } else {
            int ret;
            signal(SIGINT, intHandler);
            if(executeFunctionInMemory(jit_memory.memory, jit_memory.occupied, &ret)) {
                perror("error: Failed to execute");
            }
            signal(SIGINT, SIG_DFL);
        }
    }
}

static bool executeLine(const char* line) {
    resetLabelList(&label_list);
    resetDataList(&data_list);
    resetVariableTable(&var_table);
    resetVariableTable(&label_table);
    resetStack(&var_memory);
    resetStack(&ast_memory);
    resetStack(&jit_memory);
    resetStack(&global_exec_alloc);
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
    bool end = false;
    Ast* ast = parseLine(line, &ast_memory);
    if(ast != NULL) {
        if(ast->type == AST_ERROR) {
            AstError* error = (AstError*)ast;
            fprintf(stderr, "error: Syntax error at line %i:%i\n", data.line, error->offset + 1);
            fprintf(stderr, " | %s\n", line);
            fprintf(stderr, " | ");
            for(size_t i = 0; i < error->offset; i++) {
                if(line[i] == '\t') {
                    fputc('\t', stderr);
                } else {
                    fputc(' ', stderr);
                }
            }
            fputc('^', stderr);
            fputc('\n', stderr);
        } else if(ast->type == AST_RUN) {
            runProgram();
        } else if(ast->type == AST_NEW) {
            removeAllLines();
        } else if(ast->type == AST_LIST) {
            AstUnary* list = (AstUnary*)ast;
            if(list->value == NULL) {
                listAllLines();
            } else {
                AstInt* line = (AstInt*)list->value;
                listLine(line->value);
            }
        } else {
            addInstPushCallerRegs(data.inst_mem, data.registers);
            Error error = generateMC(ast, &data);
            if(error != ERROR_NONE) {
                fprintf(stderr, "error: %s at line %i\n", getErrorName(error), data.line);
            } else {
                addInstPopCallerRegs(data.inst_mem, data.registers);
                addInstReturn(data.inst_mem, data.registers);
                int err = fillUnhandledLabelLocations(data.label_list, data.label_table, data.inst_mem);
                if(err >= 0) {
                    fprintf(stderr, "error: Unresolved label %s at line %i\n", label_list.data[err].name, label_list.data[err].line);
                } else {
                    int ret;
                    if(executeFunctionInMemory(jit_memory.memory, jit_memory.occupied, &ret)) {
                        perror("error: Failed to execute");
                    } else if(ret == 42) {
                        end = true;
                    }
                }
            }
        }
    }
    return end;
}

void executeCli() {
    bool end = false;
    while(!end) {
        fprintf(stdout, ">");
        if(fgets(line_buffer, MAX_LINE_BUFFER, stdin) == NULL) {
            fprintf(stdout, "\n");
            end = true;
        } else {
            size_t len = strlen(line_buffer);
            if(line_buffer[len - 1] == '\n') {
                line_buffer[len - 1] = 0;
            }
            int i = 0;
            while(isspace(line_buffer[i])) {
                i++;
            }
            if (line_buffer[i] >= '0' && line_buffer[i] <= '9') {
                int64_t line_num = 0;
                for (i = 0; line_buffer[i] >= '0' && line_buffer[i] <= '9'; i++) {
                    line_num *= 10;
                    line_num += line_buffer[i] - '0';
                }
                while(isspace(line_buffer[i])) {
                    i++;
                }
                if(line_buffer[i] == 0) {
                    removeLine(line_num);
                } else {
                    addLine(line_buffer, line_num);
                }
            } else {
                if(executeLine(line_buffer)) {
                    end = true;
                }
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
}
