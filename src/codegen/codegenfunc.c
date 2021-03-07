
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "codegenfunc.h"
#include "exec/execalloc.h"
#include "common/utf8.h"

#define PI 3.14159265358979323

static int64_t powInt64(int64_t b, int64_t e) {
    if (e < 0) {
        if (b == 1) {
            return 1;
        } else {
            return 0;
        }
    } else {
        int64_t ret = 1;
        while (e > 1) {
            if (e % 2 == 0) {
                b *= b;
                e /= 2;
            } else {
                ret *= b;
                b *= b;
                e /= 2;
            }
        }
        return ret * b;
    }
}

static Value generateMCPowCallAfterFreeReg(AstBinary* ast, MCGenerationData* data) {
    Value a = generateMCForAst(ast->first, data);
    if (a.type == VALUE_ERROR) {
        return a;
    } else if (a.type == VALUE_NONE) {
        Value ret = {.type = VALUE_ERROR, .error = ERROR_SYNTAX};
        return ret;
    } else {
        Value b = generateMCForAst(ast->second, data);
        if (b.type == VALUE_ERROR) {
            return b;
        } else if (b.type == VALUE_NONE) {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_SYNTAX};
            return ret;
        } else {
            if (a.type != b.type) {
                if (a.type == VALUE_INT && b.type == VALUE_FLOAT) {
                    Register freg = getFreeFRegister(data->registers);
                    data->registers |= freg;
                    addInstMovRegToFReg(data->inst_mem, data->registers, freg, a.reg);
                    data->registers &= ~a.reg;
                    a.type = VALUE_FLOAT;
                    a.reg = freg;
                } else if (a.type == VALUE_FLOAT && b.type == VALUE_INT) {
                    Register freg = getFreeFRegister(data->registers);
                    data->registers |= freg;
                    addInstMovRegToFReg(data->inst_mem, data->registers, freg, b.reg);
                    data->registers &= ~b.reg;
                    b.type = VALUE_FLOAT;
                    b.reg = freg;
                } else {
                    Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
                    return ret;
                }
            }
            if (a.type == VALUE_INT) {
                addInstFunctionCallBinary(data->inst_mem, data->registers, a.reg, a.reg, b.reg, powInt64);
            } else if (a.type == VALUE_FLOAT) {
                addInstFunctionCallBinary(data->inst_mem, data->registers, a.reg, a.reg, b.reg, pow);
            } else {
                Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
                return ret;
            }
            data->registers &= ~(b.reg);
            return a;
        }
    }
}

static Value generateMCPowCall(AstBinary* ast, MCGenerationData* data) {
    return withFreeRegister((Ast*)ast, data, (GenerateMCFunction)generateMCPowCallAfterFreeReg, 2, 2);
}

static double frac(double x) {
    double integ;
    return modf(x, &integ);
}

static double toDeg(double x) {
    return x * 180.0 / PI;
}

static double toRad(double x) {
    return x * PI / 180.0;
}

static Value generateMCUnaryFloatCallAfterFreeReg(AstUnary* ast, MCGenerationData* data) {
    Value a = generateMCForAst(ast->value, data);
    if (a.type == VALUE_ERROR) {
        return a;
    } else if (a.type == VALUE_NONE) {
        Value ret = {.type = VALUE_ERROR, .error = ERROR_SYNTAX};
        return ret;
    } else {
        if (a.type == VALUE_INT) {
            Register freg = getFreeFRegister(data->registers);
            data->registers |= freg;
            addInstMovRegToFReg(data->inst_mem, data->registers, freg, a.reg);
            data->registers &= ~a.reg;
            a.type = VALUE_FLOAT;
            a.reg = freg;
        } else if (a.type != VALUE_FLOAT) {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
            return ret;
        }
        switch (ast->type) {
        case AST_SIN:
            addInstFunctionCallUnary(data->inst_mem, data->registers, a.reg, a.reg, sin);
            break;
        case AST_COS:
            addInstFunctionCallUnary(data->inst_mem, data->registers, a.reg, a.reg, cos);
            break;
        case AST_TAN:
            addInstFunctionCallUnary(data->inst_mem, data->registers, a.reg, a.reg, tan);
            break;
        case AST_ASN:
            addInstFunctionCallUnary(data->inst_mem, data->registers, a.reg, a.reg, asin);
            break;
        case AST_ACS:
            addInstFunctionCallUnary(data->inst_mem, data->registers, a.reg, a.reg, acos);
            break;
        case AST_ATN:
            addInstFunctionCallUnary(data->inst_mem, data->registers, a.reg, a.reg, atan);
            break;
        case AST_LOG:
            addInstFunctionCallUnary(data->inst_mem, data->registers, a.reg, a.reg, log10);
            break;
        case AST_LN:
            addInstFunctionCallUnary(data->inst_mem, data->registers, a.reg, a.reg, log);
            break;
        case AST_EXP:
            addInstFunctionCallUnary(data->inst_mem, data->registers, a.reg, a.reg, exp);
            break;
        case AST_SQR:
            addInstFunctionCallUnary(data->inst_mem, data->registers, a.reg, a.reg, sqrt);
            break;
        case AST_FRAC:
            addInstFunctionCallUnary(data->inst_mem, data->registers, a.reg, a.reg, frac);
            break;
        case AST_DEG:
            addInstFunctionCallUnary(data->inst_mem, data->registers, a.reg, a.reg, toDeg);
            break;
        case AST_RAD:
            addInstFunctionCallUnary(data->inst_mem, data->registers, a.reg, a.reg, toRad);
            break;
        case AST_RND:
            addInstFunctionCallUnary(data->inst_mem, data->registers, a.reg, a.reg, round);
            break;
        default:
            break;
        }
        return a;
    }
}

static Value generateMCUnaryFloatCall(AstUnary* ast, MCGenerationData* data) {
    return withFreeRegister((Ast*)ast, data, (GenerateMCFunction)generateMCUnaryFloatCallAfterFreeReg, 1, 1);
}

static char* tabFunction(int64_t x) {
    char out[25];
    size_t len = snprintf(out, 25, "\e[%liG", x);
    char* ret = (char*)allocAligned(&global_exec_alloc, len + 1);
    memcpy(ret, out, len + 1);
    return ret;
}

static char* spcFunction(int64_t x) {
    char* ret = (char*)allocAligned(&global_exec_alloc, x + 1);
    memset(ret, ' ', x);
    ret[x] = 0;
    return ret;
}

static char* chrFunction(int64_t x) {
    int len = getLengthUTF8(x);
    char* ret = (char*)allocAligned(&global_exec_alloc, len + 1);
    writeUTF8(x, ret);
    ret[len] = 0;
    return ret;
}

static Value generateMCUnaryIntToStringAfterFreeReg(AstUnary* ast, MCGenerationData* data) {
    Value a = generateMCForAst(ast->value, data);
    if (a.type == VALUE_ERROR) {
        return a;
    } else if (a.type == VALUE_NONE) {
        Value ret = {.type = VALUE_ERROR, .error = ERROR_SYNTAX};
        return ret;
    } else {
        if (a.type != VALUE_INT) {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
            return ret;
        }
        switch (ast->type) {
        case AST_TAB:
            addInstFunctionCallUnary(data->inst_mem, data->registers, a.reg, a.reg, tabFunction);
            break;
        case AST_SPC:
            addInstFunctionCallUnary(data->inst_mem, data->registers, a.reg, a.reg, spcFunction);
            break;
        case AST_CHR:
            addInstFunctionCallUnary(data->inst_mem, data->registers, a.reg, a.reg, chrFunction);
            break;
        default:
            break;
        }
        a.type = VALUE_STRING;
        return a;
    }
}

static Value generateMCUnaryIntToString(AstUnary* ast, MCGenerationData* data) {
    return withFreeRegister((Ast*)ast, data, (GenerateMCFunction)generateMCUnaryIntToStringAfterFreeReg, 1, 1);
}

static int64_t int64Sign(int64_t x) {
    if (x == 0) {
        return 0;
    } else if (x > 0) {
        return 1;
    } else {
        return -1;
    }
}

static int64_t int64Abs(int64_t x) {
    if (x < 0) {
        return -x;
    } else {
        return x;
    }
}

static double floatSign(double x) {
    if (x == 0) {
        return 0.0;
    } else if (x > 0) {
        return 1.0;
    } else {
        return -1.0;
    }
}

static double floatAbs(double x) {
    if (x < 0) {
        return -x;
    } else {
        return x;
    }
}

static Value generateMCUnaryIntOrFloatAfterFreeReg(AstUnary* ast, MCGenerationData* data) {
    Value a = generateMCForAst(ast->value, data);
    if (a.type == VALUE_ERROR) {
        return a;
    } else if (a.type == VALUE_NONE) {
        Value ret = {.type = VALUE_ERROR, .error = ERROR_SYNTAX};
        return ret;
    } else {
        if (a.type == VALUE_INT) {
            switch (ast->type) {
            case AST_ABS:
                addInstFunctionCallUnary(data->inst_mem, data->registers, a.reg, a.reg, int64Abs);
                break;
            case AST_SGN:
                addInstFunctionCallUnary(data->inst_mem, data->registers, a.reg, a.reg, int64Sign);
                break;
            default:
                break;
            }
            return a;
        } else if (a.type == VALUE_FLOAT) {
            switch (ast->type) {
            case AST_ABS:
                addInstFunctionCallUnary(data->inst_mem, data->registers, a.reg, a.reg, floatAbs);
                break;
            case AST_SGN:
                addInstFunctionCallUnary(data->inst_mem, data->registers, a.reg, a.reg, floatSign);
                break;
            default:
                break;
            }
            return a;
        } else {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
            return ret;
        }
    }
}

static Value generateMCUnaryIntOrFloat(AstUnary* ast, MCGenerationData* data) {
    return withFreeRegister((Ast*)ast, data, (GenerateMCFunction)generateMCUnaryIntOrFloatAfterFreeReg, 1, 1);
}

static double parseString(char* str) {
    double ret = 0.0;
    if (str != NULL) {
        sscanf(str, " %lf", &ret);
    }
    return ret;
}

static Value generateMCValAfterFreeReg(AstUnary* ast, MCGenerationData* data) {
    Value a = generateMCForAst(ast->value, data);
    if (a.type == VALUE_ERROR) {
        return a;
    } else if (a.type == VALUE_NONE) {
        Value ret = {.type = VALUE_ERROR, .error = ERROR_SYNTAX};
        return ret;
    } else {
        if (a.type != VALUE_STRING) {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
            return ret;
        } else {
            Register freg = getFreeFRegister(data->registers);
            data->registers |= freg;
            addInstFunctionCallUnary(data->inst_mem, data->registers, freg, a.reg, parseString);
            data->registers &= ~a.reg;
            a.type = VALUE_FLOAT;
            a.reg = freg;
            return a;
        }
    }
}

static Value generateMCVal(AstUnary* ast, MCGenerationData* data) {
    return withFreeRegister((Ast*)ast, data, (GenerateMCFunction)generateMCValAfterFreeReg, 1, 1);
}

static int64_t ascFunction(char* str) {
    uint64_t ret = 0;
    if (str != NULL) {
        parseUTF8(str, &ret);
    }
    return ret;
}

static Value generateMCAscAfterFreeReg(AstUnary* ast, MCGenerationData* data) {
    Value a = generateMCForAst(ast->value, data);
    if (a.type == VALUE_ERROR) {
        return a;
    } else if (a.type == VALUE_NONE) {
        Value ret = {.type = VALUE_ERROR, .error = ERROR_SYNTAX};
        return ret;
    } else {
        if (a.type != VALUE_STRING) {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
            return ret;
        } else {
            addInstFunctionCallUnary(data->inst_mem, data->registers, a.reg, a.reg, ascFunction);
            a.type = VALUE_INT;
            return a;
        }
    }
}

static Value generateMCAsc(AstUnary* ast, MCGenerationData* data) {
    return withFreeRegister((Ast*)ast, data, (GenerateMCFunction)generateMCAscAfterFreeReg, 1, 1);
}

static char* stringifyInt(int64_t x) {
    char out[128];
    size_t len = snprintf(out, 128, "%li", x);
    char* ret = (char*)allocAligned(&global_exec_alloc, len + 1);
    memcpy(ret, out, len + 1);
    return ret;
}

static char* stringifyFloat(double x) {
    char out[128];
    size_t len = snprintf(out, 128, "%lg", x);
    char* ret = (char*)allocAligned(&global_exec_alloc, len + 1);
    memcpy(ret, out, len + 1);
    return ret;
}

static char* stringifyBoolean(int64_t x) {
    if (x) {
        char* ret = (char*)allocAligned(&global_exec_alloc, 5);
        memcpy(ret, "True", 5);
        return ret;
    } else {
        char* ret = (char*)allocAligned(&global_exec_alloc, 6);
        memcpy(ret, "False", 6);
        return ret;
    }
}

static Value generateMCStrAfterFreeReg(AstUnary* ast, MCGenerationData* data) {
    Value a = generateMCForAst(ast->value, data);
    if (a.type == VALUE_ERROR) {
        return a;
    } else if (a.type == VALUE_NONE) {
        Value ret = {.type = VALUE_ERROR, .error = ERROR_SYNTAX};
        return ret;
    } else {
        if (a.type == VALUE_INT) {
            addInstFunctionCallUnary(data->inst_mem, data->registers, a.reg, a.reg, stringifyInt);
            a.type = VALUE_STRING;
            return a;
        } else if (a.type == VALUE_FLOAT) {
            Register reg = getFreeRegister(data->registers);
            data->registers |= reg;
            addInstFunctionCallUnary(data->inst_mem, data->registers, reg, a.reg, stringifyFloat);
            data->registers &= ~a.reg;
            a.type = VALUE_STRING;
            a.reg = reg;
            return a;
        } else if (a.type == VALUE_BOOLEAN) {
            addInstFunctionCallUnary(data->inst_mem, data->registers, a.reg, a.reg, stringifyBoolean);
            a.type = VALUE_STRING;
            return a;
        } else {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
            return ret;
        }
    }
}

static Value generateMCStr(AstUnary* ast, MCGenerationData* data) {
    return withFreeRegister((Ast*)ast, data, (GenerateMCFunction)generateMCStrAfterFreeReg, 1, 1);
}

static void printInt64(int64_t x) {
    fprintf(stdout, "%li", x);
}

static void printFloat(double x) {
    fprintf(stdout, "%lg", x);
}

static void printString(char* x) {
    if(x != NULL) {
        fprintf(stdout, "%s", x);
    }
}

static void printBoolean(int64_t x) {
    if (x) {
        fprintf(stdout, "True");
    } else {
        fprintf(stdout, "False");
    }
}

static void printLn() {
    fprintf(stdout, "\n");
}

static Value generateMCPrintAfterFreeReg(AstVariable* ast, MCGenerationData* data) {
    for (int i = 0; i < ast->count; i++) {
        Value a = generateMCForAst(ast->values[i], data);
        if (a.type == VALUE_ERROR) {
            return a;
        } else if (a.type == VALUE_NONE) {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_SYNTAX};
            return ret;
        } else {
            if (a.type == VALUE_INT) {
                addInstFunctionCallUnaryNoRet(data->inst_mem, data->registers, a.reg, printInt64);
            } else if (a.type == VALUE_FLOAT) {
                addInstFunctionCallUnaryNoRet(data->inst_mem, data->registers, a.reg, printFloat);
            } else if (a.type == VALUE_STRING) {
                addInstFunctionCallUnaryNoRet(data->inst_mem, data->registers, a.reg, printString);
            } else if (a.type == VALUE_BOOLEAN) {
                addInstFunctionCallUnaryNoRet(data->inst_mem, data->registers, a.reg, printBoolean);
            } else {
                Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
                return ret;
            }
            data->registers &= ~a.reg;
        }
    }
    if (!ast->open_end) {
        addInstFunctionCallSimple(data->inst_mem, data->registers, printLn);
    }
    Value ret = {.type = VALUE_NONE};
    return ret;
}

static Value generateMCPrint(AstVariable* ast, MCGenerationData* data) {
    return withFreeRegister((Ast*)ast, data, (GenerateMCFunction)generateMCPrintAfterFreeReg, 1, 1);
}

static int64_t inputInt() {
    int64_t ret;
    fscanf(stdin, " %li", &ret);
    return ret;
}

static double inputFloat() {
    double ret;
    fscanf(stdin, " %lg", &ret);
    return ret;
}

static char* inputString() {
    char tmp[2048];
    fgets(tmp, 2048, stdin);
    size_t len = strlen(tmp);
    if (tmp[len - 1] == '\n') {
        len--;
        tmp[len] = 0;
    }
    char* ret = (char*)allocAligned(&global_exec_alloc, len + 1);
    memcpy(ret, tmp, len + 1);
    return ret;
}

static Value generateMCInputArrayElementAfterFreeReg(AstIndex* ast, MCGenerationData* data) {
    AstIndex* index = (AstIndex*)ast;
    AstVar* var = (AstVar*)index->name;
    Variable* variable = getVariable(data->variable_table, var->name);
    if (variable != NULL) {
        if (variable->type != VARIABLE_STRING_ARRAY && variable->type != VARIABLE_INT_ARRAY && variable->type != VARIABLE_FLOAT_ARRAY) {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
            return ret;
        } else if (var->var_type != VAR_UNDEF) {
            if ((variable->type == VARIABLE_INT_ARRAY && var->var_type != VAR_INT) || (variable->type == VARIABLE_FLOAT_ARRAY && var->var_type != VAR_FLOAT) || (variable->type == VARIABLE_STRING_ARRAY && var->var_type != VAR_STR)) {
                Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
                return ret;
            }
        }
        if ((variable->type == VARIABLE_INT_ARRAY && ((VariableIntArray*)variable)->dim_count != index->count) || (variable->type == VARIABLE_FLOAT_ARRAY && ((VariableIntArray*)variable)->dim_count != index->count) || (variable->type == VARIABLE_STRING_ARRAY && ((VariableIntArray*)variable)->dim_count != index->count)) {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_ARRAY_DIM_COUNT_MISMATCH};
            return ret;
        }
        Register imm_reg = getFreeRegister(data->registers);
        data->registers |= imm_reg;
        Register index_reg = getFreeRegister(data->registers);
        data->registers |= index_reg;
        size_t indexing_size;
        if (variable->type == VARIABLE_INT_ARRAY) {
            addInstMovImmToReg(data->inst_mem, data->registers, index_reg, (intptr_t)((VariableIntArray*)variable)->value);
            indexing_size = sizeof(int64_t);
        } else if (variable->type == VARIABLE_FLOAT_ARRAY) {
            addInstMovImmToReg(data->inst_mem, data->registers, index_reg, (intptr_t)((VariableFloatArray*)variable)->value);
            indexing_size = sizeof(double);
        } else if (variable->type == VARIABLE_STRING_ARRAY) {
            addInstMovImmToReg(data->inst_mem, data->registers, index_reg, (intptr_t)((VariableStringArray*)variable)->str);
            indexing_size = sizeof(char*);
        }
        for (int i = 0; i < index->count; i++) {
            Value ind = generateMCForAst(index->size[i], data);
            if (ind.type == VALUE_ERROR) {
                return ind;
            } else if (ind.type == VALUE_NONE) {
                Value ret = {.type = VALUE_ERROR, .error = ERROR_SYNTAX};
                return ret;
            } else if (ind.type != VALUE_INT) {
                Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
                return ret;
            } else {
                addInstMovImmToReg(data->inst_mem, data->registers, imm_reg, indexing_size);
                addInstMul(data->inst_mem, data->registers, imm_reg, imm_reg, ind.reg);
                data->registers &= ~ind.reg;
                addInstAdd(data->inst_mem, data->registers, index_reg, index_reg, imm_reg);
            }
            if (variable->type == VARIABLE_INT_ARRAY) {
                indexing_size *= ((VariableIntArray*)variable)->size[i];
            } else if (variable->type == VARIABLE_FLOAT_ARRAY) {
                indexing_size *= ((VariableFloatArray*)variable)->size[i];
            } else if (variable->type == VARIABLE_STRING_ARRAY) {
                indexing_size *= ((VariableStringArray*)variable)->size[i];
            }
        }
        data->registers &= ~imm_reg;
        if (variable->type == VARIABLE_INT_ARRAY) {
            Register reg = getFreeRegister(data->registers);
            data->registers |= reg;
            addInstFunctionCallRetOnly(data->inst_mem, data->registers, reg, inputInt);
            addInstMovRegToMemReg(data->inst_mem, data->registers, index_reg, reg);
            data->registers &= ~reg;
        } else if (variable->type == VARIABLE_FLOAT_ARRAY) {
            Register freg = getFreeFRegister(data->registers);
            data->registers |= freg;
            addInstFunctionCallRetOnly(data->inst_mem, data->registers, freg, inputFloat);
            addInstMovFRegToMemReg(data->inst_mem, data->registers, index_reg, freg);
            data->registers &= ~freg;
        } else if (variable->type == VARIABLE_STRING_ARRAY) {
            Register reg = getFreeRegister(data->registers);
            data->registers |= reg;
            addInstFunctionCallRetOnly(data->inst_mem, data->registers, reg, inputString);
            addInstMovRegToMemReg(data->inst_mem, data->registers, index_reg, reg);
            data->registers &= ~reg;
        } else {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
            return ret;
        }
        data->registers &= ~index_reg;
        Value ret = {.type = VALUE_NONE};
        return ret;
    } else {
        Value ret = {.type = VALUE_ERROR, .error = ERROR_ARRAY_NOT_DEF};
        return ret;
    }
}

static Value generateMCInputAfterFreeReg(AstVariable* ast, MCGenerationData* data) {
    for (int i = 0; i < ast->count; i++) {
        if (ast->values[i]->type == AST_VAR) {
            AstVar* var = (AstVar*)ast->values[i];
            Variable* variable = getVariable(data->variable_table, var->name);
            if (variable == NULL) {
                if (var->var_type == VAR_UNDEF || var->var_type == VAR_FLOAT) {
                    VariableFloat* varib = (VariableFloat*)allocAligned(data->variable_mem, sizeof(VariableFloat));
                    varib->type = VARIABLE_FLOAT;
                    varib->for_jmp_loc = ~0;
                    varib->for_call_loc = ~0;
                    varib->value = 0.0;
                    addVariable(data->variable_table, var->name, (Variable*)varib, data->variable_mem);
                    variable = (Variable*)varib;
                } else if (var->var_type == VAR_INT) {
                    VariableInt* varib = (VariableInt*)allocAligned(data->variable_mem, sizeof(VariableInt));
                    varib->type = VARIABLE_INT;
                    varib->for_jmp_loc = ~0;
                    varib->for_call_loc = ~0;
                    varib->value = 0;
                    addVariable(data->variable_table, var->name, (Variable*)varib, data->variable_mem);
                    variable = (Variable*)varib;
                } else if (var->var_type == VAR_STR) {
                    VariableString* varib = (VariableString*)allocAligned(data->variable_mem, sizeof(VariableString));
                    varib->type = VARIABLE_STRING;
                    varib->str = NULL;
                    addVariable(data->variable_table, var->name, (Variable*)varib, data->variable_mem);
                    variable = (Variable*)varib;
                }
            } else if (var->var_type != VAR_UNDEF) {
                if ((variable->type == VARIABLE_INT && var->var_type != VAR_INT) || (variable->type == VARIABLE_FLOAT && var->var_type != VAR_FLOAT) || (variable->type == VARIABLE_STRING && var->var_type != VAR_STR)) {
                    Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
                    return ret;
                }
            } else if (variable->type != VARIABLE_FLOAT && variable->type != VARIABLE_INT && variable->type != VARIABLE_STRING) {
                Value ret = {.type = VALUE_ERROR, .error = ERROR_UNINDEXED_ARRAY};
                return ret;
            }
            if (variable->type == VARIABLE_INT) {
                Register reg = getFreeRegister(data->registers);
                data->registers |= reg;
                addInstFunctionCallRetOnly(data->inst_mem, data->registers, reg, inputInt);
                addInstMovRegToMem(data->inst_mem, data->registers, reg, &((VariableInt*)variable)->value);
                data->registers &= ~reg;
            } else if (variable->type == VARIABLE_FLOAT) {
                Register freg = getFreeFRegister(data->registers);
                data->registers |= freg;
                addInstFunctionCallRetOnly(data->inst_mem, data->registers, freg, inputFloat);
                addInstMovFRegToMem(data->inst_mem, data->registers, freg, &((VariableFloat*)variable)->value);
                data->registers &= ~freg;
            } else if (variable->type == VARIABLE_STRING) {
                Register reg = getFreeRegister(data->registers);
                data->registers |= reg;
                addInstFunctionCallRetOnly(data->inst_mem, data->registers, reg, inputString);
                addInstMovRegToMem(data->inst_mem, data->registers, reg, &((VariableString*)variable)->str);
                data->registers &= ~reg;
            } else {
                Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
                return ret;
            }
        } else if (ast->values[i]->type == AST_INDEX) {
            AstIndex* index = (AstIndex*)ast->values[i];
            Value ret = withFreeRegister((Ast*)index, data, (GenerateMCFunction)generateMCInputArrayElementAfterFreeReg, 3, 1);
            if (ret.type == VALUE_ERROR) {
                return ret;
            }
        } else if(ast->values[i]->type == AST_STRING) {
            Value a = generateMCForAst(ast->values[i], data);
            addInstFunctionCallUnaryNoRet(data->inst_mem, data->registers, a.reg, printString);
            data->registers &= ~a.reg;
        } else {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_SYNTAX};
            return ret;
        }
    }
    Value ret = {.type = VALUE_NONE};
    return ret;
}

static Value generateMCInput(AstVariable* ast, MCGenerationData* data) {
    return withFreeRegister((Ast*)ast, data, (GenerateMCFunction)generateMCInputAfterFreeReg, 1, 1);
}

static double randomFloat() { return rand() / (double)RAND_MAX; }

static Value generateMCRan(Ast* ast, MCGenerationData* data) {
    Register freg = getFreeFRegister(data->registers);
    data->registers |= freg;
    addInstFunctionCallRetOnly(data->inst_mem, data->registers, freg, randomFloat);
    Value ret = {.type = VALUE_FLOAT, .reg = freg};
    return ret;
}

static char* key() {
    char* ret = (char*)allocAligned(&global_exec_alloc, 2);
    ret[0] = getc(stdin);
    ret[1] = 0;
    return ret;
}

static Value generateMCKey(Ast* ast, MCGenerationData* data) {
    Register reg = getFreeRegister(data->registers);
    data->registers |= reg;
    addInstFunctionCallRetOnly(data->inst_mem, data->registers, reg, key);
    Value ret = {.type = VALUE_STRING, .reg = reg};
    return ret;
}

static void beep() {
    fprintf(stderr, "\a");
}

static void stop() {
    fprintf(stderr, "Press [ENTER] to continue...");
    getc(stdin);
}

static Value generateMCSimpleCall(Ast* ast, MCGenerationData* data) {
    switch (ast->type) {
    case AST_BEEP:
        addInstFunctionCallSimple(data->inst_mem, data->registers, beep);
        break;
    case AST_STOP:
        addInstFunctionCallSimple(data->inst_mem, data->registers, stop);
        break;
    default:
        break;
    }
    Value ret = {.type = VALUE_NONE};
    return ret;
}

static char* left(char* str, int64_t num) {
    size_t len = (str == NULL ? 0 : strlen(str));
    if (num < 0) {
        num = 0;
    } else if (len < (size_t)num) {
        num = len;
    }
    char* ret = (char*)allocAligned(&global_exec_alloc, num + 1);
    memcpy(ret, str, num);
    ret[num] = 0;
    return ret;
}

static char* right(char* str, int64_t num) {
    size_t len = (str == NULL ? 0 : strlen(str));
    if (num < 0) {
        num = 0;
    } else if (len < (size_t)num) {
        num = len;
    }
    char* ret = (char*)allocAligned(&global_exec_alloc, num + 1);
    memcpy(ret, str + len - num, num);
    ret[num] = 0;
    return ret;
}

static Value generateMCLeftOrRight(AstBinary* ast, MCGenerationData* data) {
    Value a = generateMCForAst(ast->first, data);
    if (a.type == VALUE_ERROR) {
        return a;
    } else if (a.type == VALUE_NONE) {
        Value ret = {.type = VALUE_ERROR, .error = ERROR_SYNTAX};
        return ret;
    } else if (a.type != VALUE_STRING) {
        Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
        return ret;
    } else {
        Value b = generateMCForAst(ast->second, data);
        if (b.type == VALUE_ERROR) {
            return b;
        } else if (b.type == VALUE_NONE) {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_SYNTAX};
            return ret;
        } else if (b.type != VALUE_INT) {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
            return ret;
        } else {
            switch (ast->type) {
            case AST_LEFT:
                addInstFunctionCallBinary(data->inst_mem, data->registers, a.reg, a.reg, b.reg, left);
                break;
            case AST_RIGHT:
                addInstFunctionCallBinary(data->inst_mem, data->registers, a.reg, a.reg, b.reg, right);
                break;
            default:
                break;
            }
            data->registers &= ~(b.reg);
            return a;
        }
    }
}

static void sleep_for(double sec) {
    usleep((int)(sec * 1000000.0));
}

static Value generateMCUnaryFloatNoRetAfterFreeReg(AstUnary* ast, MCGenerationData* data) {
    Value a = generateMCForAst(ast->value, data);
    if (a.type == VALUE_ERROR) {
        return a;
    } else if (a.type == VALUE_NONE) {
        Value ret = {.type = VALUE_ERROR, .error = ERROR_SYNTAX};
        return ret;
    } else {
        if (a.type == VALUE_INT) {
            Register freg = getFreeFRegister(data->registers);
            data->registers |= freg;
            addInstMovRegToFReg(data->inst_mem, data->registers, freg, a.reg);
            data->registers &= ~a.reg;
            a.type = VALUE_FLOAT;
            a.reg = freg;
        } else if (a.type != VALUE_FLOAT) {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
            return ret;
        }
        switch (ast->type) {
        case AST_SLEEP:
            addInstFunctionCallUnaryNoRet(data->inst_mem, data->registers, a.reg, sleep_for);
            break;
        default:
            break;
        }
        data->registers &= ~a.reg;
        Value ret = {.type = VALUE_NONE };
        return ret;
    }
}

static Value generateMCUnaryFloatNoRet(AstUnary* ast, MCGenerationData* data) {
    return withFreeRegister((Ast*)ast, data, (GenerateMCFunction)generateMCUnaryFloatNoRetAfterFreeReg, 1, 1);
}

static void end_simple() {
    exit(0);
}

static void end_value(int val) {
    exit(val);
}

static Value generateMCEndAfterFreeReg(AstUnary* ast, MCGenerationData* data) {
    Value a = generateMCForAst(ast->value, data);
    if (a.type == VALUE_ERROR) {
        return a;
    } else if (a.type == VALUE_NONE) {
        Value ret = {.type = VALUE_ERROR, .error = ERROR_SYNTAX};
        return ret;
    } else {
        if (a.type != VALUE_INT) {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
            return ret;
        } else {
            addInstFunctionCallUnaryNoRet(data->inst_mem, data->registers, a.reg, end_value);
            data->registers &= ~a.reg;
            Value ret = {.type = VALUE_NONE };
            return ret;
        }
    }
}

static Value generateMCEnd(AstUnary* ast, MCGenerationData* data) {
    if (ast->value == NULL) {
        addInstFunctionCallSimple(data->inst_mem, data->registers, end_simple);
        Value ret = {.type = VALUE_NONE };
        return ret;
    } else {
        return withFreeRegister((Ast*)ast, data, (GenerateMCFunction)generateMCEndAfterFreeReg, 1, 1);
    }
}

static void failedAssert(int64_t line) {
    fprintf(stderr, "error: Failed assertion at line %li\n", line);
    exit(EXIT_FAILURE);
}

static Value generateMCAssertAfterFreeReg(AstUnary* ast, MCGenerationData* data) {
    size_t skipjmp = 0;
    Value cond = generateMCForAst(ast->value, data);
    if (cond.type == VALUE_ERROR) {
        return cond;
    } else if (cond.type == VALUE_NONE) {
        Value ret = {.type = VALUE_ERROR, .error = ERROR_SYNTAX};
        return ret;
    } else if (cond.type != VALUE_BOOLEAN) {
        Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
        return ret;
    } else {
        Register zero = getFreeRegister(data->registers);
        data->registers |= zero;
        addInstMovImmToReg(data->inst_mem, data->registers, zero, 0);
        skipjmp = addInstCondJmpRel(data->inst_mem, data->registers, COND_NE, cond.reg, zero, 0);
        data->registers &= ~cond.reg;
        data->registers &= ~zero;
        Register line = getFreeRegister(data->registers);
        data->registers |= line;
        addInstMovImmToReg(data->inst_mem, data->registers, line, data->line);
        addInstFunctionCallUnaryNoRet(data->inst_mem, data->registers, line, failedAssert);
        data->registers &= ~line;
        updateRelativeJumpTarget(data->inst_mem, skipjmp, data->inst_mem->occupied);
        Value ret = { .type=VALUE_NONE };
        return ret;
    }
}

static Value generateMCAssert(AstUnary* ast, MCGenerationData* data) {
    return withFreeRegister((Ast*)ast, data, (GenerateMCFunction)generateMCAssertAfterFreeReg, 2, 2);
}

static int64_t stringLength(char* str) {
    return str == NULL ? 0 : strlen(str);
}

static Value generateMCLenAfterFreeReg(AstUnary* ast, MCGenerationData* data) {
    Value a = generateMCForAst(ast->value, data);
    if (a.type == VALUE_ERROR) {
        return a;
    } else if (a.type == VALUE_NONE) {
        Value ret = {.type = VALUE_ERROR, .error = ERROR_SYNTAX};
        return ret;
    } else {
        if (a.type != VALUE_STRING) {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
            return ret;
        } else {
            addInstFunctionCallUnary(data->inst_mem, data->registers, a.reg, a.reg, stringLength);
            a.type = VALUE_INT;
            return a;
        }
    }
}

static Value generateMCLen(AstUnary* ast, MCGenerationData* data) {
    return withFreeRegister((Ast*)ast, data, (GenerateMCFunction)generateMCLenAfterFreeReg, 1, 1);
}

Value generateMCForFunctions(Ast* ast, MCGenerationData* data) {
    Value value = {.type = VALUE_NONE};
    if (ast != NULL) {
        switch (ast->type) {
        case AST_POW:
            value = generateMCPowCall((AstBinary*)ast, data);
            break;
        case AST_TAB:
        case AST_SPC:
        case AST_CHR:
            value = generateMCUnaryIntToString((AstUnary*)ast, data);
            break;
        case AST_SIN:
        case AST_COS:
        case AST_TAN:
        case AST_ASN:
        case AST_ACS:
        case AST_ATN:
        case AST_LOG:
        case AST_LN:
        case AST_EXP:
        case AST_SQR:
        case AST_FRAC:
        case AST_DEG:
        case AST_RAD:
        case AST_RND:
            value = generateMCUnaryFloatCall((AstUnary*)ast, data);
            break;
        case AST_SGN:
        case AST_ABS:
            value = generateMCUnaryIntOrFloat((AstUnary*)ast, data);
            break;
        case AST_VAL:
            value = generateMCVal((AstUnary*)ast, data);
            break;
        case AST_ASC:
            value = generateMCAsc((AstUnary*)ast, data);
            break;
        case AST_STR:
            value = generateMCStr((AstUnary*)ast, data);
            break;
        case AST_PRINT:
            value = generateMCPrint((AstVariable*)ast, data);
            break;
        case AST_INPUT:
            value = generateMCInput((AstVariable*)ast, data);
            break;
        case AST_RAN:
            value = generateMCRan(ast, data);
            break;
        case AST_KEY:
            value = generateMCKey(ast, data);
            break;
        case AST_BEEP:
        case AST_STOP:
            value = generateMCSimpleCall(ast, data);
            break;
        case AST_LEFT:
        case AST_RIGHT:
            value = generateMCLeftOrRight((AstBinary*)ast, data);
            break;
        case AST_SLEEP:
            value = generateMCUnaryFloatNoRet((AstUnary*)ast, data);
            break;
        case AST_END:
            value = generateMCEnd((AstUnary*)ast, data);
            break;
        case AST_ASSERT:
            value = generateMCAssert((AstUnary*)ast, data);
            break;
        case AST_LEN:
            value = generateMCLen((AstUnary*)ast, data);
            break;
        default:
            value.type = VALUE_ERROR;
            value.error = ERROR_SYNTAX;
            break;
        }
    }
    return value;
}
