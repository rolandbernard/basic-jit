
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>

#ifndef NOREADLINE
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include "exec/cli.h"
#include "common/stackalloc.h"
#include "codegen/codegen.h"
#include "codegen/datalist.h"
#include "codegen/labellist.h"
#include "codegen/variabletable.h"
#include "parser/parser.h"
#include "exec/executil.h"
#include "exec/execalloc.h"

#define INITIAL_LINE_BUFFER 32
static char* line_buffer;
static int line_buffer_capacity = 0;

#define INITIAL_LINE_COUNT 32
static int64_t* line_numbers;
static char** lines;
static int num_lines = 0;
static int line_capacity = 0;
    
static StackAllocator ast_memory = STACK_ALLOCATOR_INITIALIZER;
static DataList data_list = DATA_LIST_INITIALIZER;
static StackAllocator var_memory = STACK_ALLOCATOR_INITIALIZER;
static VariableTable var_table = VARIABLE_TABLE_INITIALIZER;
static StackAllocator jit_memory = STACK_ALLOCATOR_INITIALIZER;
static VariableTable label_table = VARIABLE_TABLE_INITIALIZER;
static UnhandeledLabelList label_list = UNHANDLED_LABEL_LIST_INITIALIZER;
static VariableTable func_table = VARIABLE_TABLE_INITIALIZER;
            
static int exit_code = EXIT_SUCCESS;

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
        if (num_lines == line_capacity) {
            if (line_capacity == 0) {
                line_capacity = INITIAL_LINE_COUNT;
                line_numbers = (int64_t*)malloc(sizeof(int64_t) * line_capacity);
                lines = (char**)malloc(sizeof(char*) * line_capacity);
            } else {
                line_capacity *= 2;
                line_numbers = (int64_t*)realloc(line_numbers, sizeof(int64_t) * line_capacity);
                lines = (char**)realloc(lines, sizeof(char*) * line_capacity);
            }
        }
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
    free(line_numbers);
    free(lines);
    line_numbers = NULL;
    lines = NULL;
    num_lines = 0;
    line_capacity = 0;
}

void intHandler(int dummy) {
    fprintf(stdout, "\n");
}

static void runProgram() {
    resetLabelList(&label_list);
    resetDataList(&data_list);
    resetVariableTable(&var_table);
    resetVariableTable(&label_table);
    resetVariableTable(&func_table);
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
        .func_table = &func_table,
        .registers = 0,
        .line = 1,
    };
    addInstPushCallerRegs(data.inst_mem, data.registers);
    bool had_error = false;
    for(int i = 0; i < num_lines && !had_error; i++) {
        data.line = line_numbers[i];
        Ast* ast = parseLine(lines[i], &ast_memory);
        if(ast != NULL) {
            if(ast->type == AST_ERROR) {
                AstError* error = (AstError*)ast;
                fprintf(stderr, "error: Syntax error at line %i:%i\n", data.line, error->offset + 1);
                fprintf(stderr, " | %s\n", lines[i]);
                fprintf(stderr, " | ");
                for(int j = 0; j < error->offset; j++) {
                    if(lines[i][j] == '\t') {
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
#ifdef DEBUG
            printMemoryContent(stderr, jit_memory.memory, jit_memory.occupied);
#endif
            executeFunctionInMemory(jit_memory.memory, jit_memory.occupied, &ret);
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
                default:
                    break;
                }
            }
            signal(SIGINT, SIG_DFL);
        }
    }
}

static bool executeLine(const char* line);

#ifndef NOREADLINE
static const char* readline_init = NULL;
static int readlineHook() {
    if (readline_init != NULL) {
        rl_insert_text(readline_init);
    }
    return 0;
}
#endif

static bool inputLine(FILE* input) {
    bool end = false;
    int next_char = 0;
    int len = 0;
#ifndef NOREADLINE
    if (isatty(fileno(input))) {
        char* line = readline(">");
        if (line == NULL) {
            next_char = EOF;
            len = 0;
        } else {
            len = strlen(line);
            while (len >= line_buffer_capacity) {
                if (line_buffer_capacity == 0) {
                    line_buffer_capacity = INITIAL_LINE_BUFFER;
                    line_buffer = (char*)malloc(sizeof(char) * line_buffer_capacity);
                } else {
                    line_buffer_capacity *= 2;
                    line_buffer = (char*)realloc(line_buffer, sizeof(char) * line_buffer_capacity);
                }
            }
            memcpy(line_buffer, line, len);
            add_history(line);
            free(line);
        }
    } else {
#endif
        do {
            next_char = fgetc(input);
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
#ifndef NOREADLINE
    }
    readline_init = NULL;
#endif
    line_buffer[len] = 0;
    if (next_char != EOF || len != 0) {
        int i = 0;
        while (isspace(line_buffer[i])) {
            i++;
        }
        if (line_buffer[i] >= '0' && line_buffer[i] <= '9') {
            int64_t line_num = 0;
            for (; line_buffer[i] >= '0' && line_buffer[i] <= '9'; i++) {
                line_num *= 10;
                line_num += line_buffer[i] - '0';
            }
            while (isspace(line_buffer[i])) {
                i++;
            }
            if (line_buffer[i] == 0) {
                removeLine(line_num);
            } else {
                addLine(line_buffer, line_num);
            }
        } else {
            if (line_buffer[i] == '>') {
                if (executeLine(line_buffer + i + 1)) {
                    end = true;
                }
            } else {
                if (executeLine(line_buffer)) {
                    end = true;
                }
            }
        }
    } else {
        end = true;
    }
    return end;
}

static bool executeLine(const char* line) {
    resetLabelList(&label_list);
    resetDataList(&data_list);
    resetVariableTable(&var_table);
    resetVariableTable(&label_table);
    resetVariableTable(&func_table);
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
        .func_table = &func_table,
        .registers = 0,
        .line = 1,
    };
    bool end = false;
    Ast* ast = parseExpressionLine(line, &ast_memory);
    if(ast != NULL) {
        if(ast->type == AST_ERROR) {
            AstError* error = (AstError*)ast;
            fprintf(stderr, "error: Syntax error at line %i:%i\n", data.line, error->offset + 1);
            fprintf(stderr, " | %s\n", line);
            fprintf(stderr, " | ");
            for(int i = 0; i < error->offset; i++) {
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
        } else if(ast->type == AST_SAVE) {
            AstUnary* save_ast = (AstUnary*)ast;
            AstString* filename_ast = (AstString*)save_ast->value;
            FILE* file = fopen(filename_ast->str, "w");
            if (file == NULL) {
                fprintf(stderr, "error: Failed to open the file '%s': %s\n", filename_ast->str, strerror(errno));
            } else {
                for (int i = 0; i < num_lines; i++) {
                    fwrite(lines[i], 1, strlen(lines[i]), file);
                    putc('\n', file);
                }
                fclose(file);
            }
        } else if(ast->type == AST_LOAD) {
            AstUnary* load_ast = (AstUnary*)ast;
            AstString* filename_ast = (AstString*)load_ast->value;
            FILE* file = fopen(filename_ast->str, "r");
            if (file == NULL) {
                fprintf(stderr, "error: Failed to open the file '%s': %s\n", filename_ast->str, strerror(errno));
            } else {
                removeAllLines();
                while (!inputLine(file)) { }
                fclose(file);
            }
        } else if(ast->type == AST_LIST) {
            AstUnary* list = (AstUnary*)ast;
            if(list->value == NULL) {
                listAllLines();
            } else {
                AstInt* line = (AstInt*)list->value;
                listLine(line->value);
            }
        } else if(ast->type == AST_EDIT) {
#ifndef NOREADLINE
            AstUnary* list = (AstUnary*)ast;
            AstInt* line = (AstInt*)list->value;
            int i = findLine(line->value);
            if(i < num_lines && line_numbers[i] == line->value) {
                readline_init = lines[i];
            }
#else
            fprintf(stderr, "error: Readline support is disabled\n");
#endif
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
#ifdef DEBUG
                    printMemoryContent(stderr, jit_memory.memory, jit_memory.occupied);
#endif
                    executeFunctionInMemory(jit_memory.memory, jit_memory.occupied, &ret);
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
                            end = true;
                            break;
                        }
                    }
                    exit_code = ret;
                }
            }
        }
    }
    return end;
}

int executeCli() {
#ifndef NOREADLINE
    rl_startup_hook = readlineHook;
#endif
    bool end = false;
    while(!end) {
#ifdef NOREADLINE
        fprintf(stdout, ">");
#endif
        end = inputLine(stdin);
    }
    freeLabelList(&label_list);
    freeDataList(&data_list);
    freeVariableTable(&var_table);
    freeVariableTable(&label_table);
    freeVariableTable(&func_table);
    freeStack(&var_memory);
    freeStack(&ast_memory);
    freeStack(&jit_memory);
    freeStack(&global_exec_alloc);
    removeAllLines();
    free(line_buffer);
#ifndef NOREADLINE
    clear_history();
#endif
    line_buffer = NULL;
    line_buffer_capacity = 0;
    return exit_code;
}
