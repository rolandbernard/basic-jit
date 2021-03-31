
#include <string.h>

#ifndef NONATIVEFN
#include <dlfcn.h>
#endif

#include "codegen/codegen.h"
#include "codegen/codegenfunc.h"
#include "exec/execalloc.h"

static uint64_t data_index = 0;

static int int64ToString(char* str, int64_t v) {
    int len = 0;
    if (v == 0) {
        str[0] = '0';
        len++;
    } else {
        while (v > 0) {
            str[len] = '0' + (v % 10);
            v /= 10;
            len++;
        }
        for (int i = 0; i < len / 2; i++) {
            char tmp = str[i];
            str[i] = str[len - i - 1];
            str[len - i - 1] = tmp;
        }
    }
    str[len] = 0;
    return len;
}

Value withFreeRegister(Ast* ast, MCGenerationData* data, GenerateMCFunction func, int num_regs, int num_fregs) {
    int free_regs = countFreeRegister(data->registers);
    int free_fregs = countFreeFRegister(data->registers);
    Register ret_reg = getFreeRegister(data->registers);
    Register ret_freg = getFreeFRegister(data->registers);
    uint64_t to_pop[num_regs + num_fregs];
    int to_pop_count = 0;
    while (free_regs < num_regs) {
        uint64_t reg = getUsedRegister(data->registers);
        addInstPush(data->inst_mem, data->registers, reg);
        data->registers &= ~reg;
        to_pop[to_pop_count] = reg;
        to_pop_count++;
        free_regs++;
    }
    while (free_fregs < num_fregs) {
        uint64_t freg = getUsedFRegister(data->registers);
        addInstPush(data->inst_mem, data->registers, freg);
        data->registers &= ~freg;
        to_pop[to_pop_count] = freg;
        to_pop_count++;
        free_fregs++;
    }
    Value ret = func(ast, data);
    if (ret.type != VALUE_NONE && ret.type != VALUE_ERROR) {
        if (ret.type == VALUE_FLOAT) {
            addInstMovFRegToFReg(data->inst_mem, data->registers, ret_freg, ret.reg);
            data->registers &= ~ret.reg;
            ret.reg = ret_freg;
            data->registers |= ret.reg;
        } else {
            addInstMovRegToReg(data->inst_mem, data->registers, ret_reg, ret.reg);
            data->registers &= ~ret.reg;
            ret.reg = ret_reg;
            data->registers |= ret.reg;
        }
    }
    while (to_pop_count > 0) {
        to_pop_count--;
        uint64_t reg = to_pop[to_pop_count];
        addInstPop(data->inst_mem, data->registers, reg);
        data->registers |= reg;
    }
    return ret;
}

static Value generateMCGo(AstUnary* ast, MCGenerationData* data) {
    size_t pos;
    if (ast->type == AST_GOSUB) {
        pos = addInstCallRel(data->inst_mem, data->registers, 0);
    } else {
        pos = addInstJmpRel(data->inst_mem, data->registers, 0);
    }
    if (ast->value->type == AST_INTEGER) {
        char name[25];
        AstInt* line = (AstInt*)ast->value;
        int len = int64ToString(name, line->value);
        char* sym = (char*)allocAligned(data->variable_mem, len + 1);
        memcpy(sym, name, len + 1);
        UnhandeledLabelEntry entry = {
            .name = sym,
            .line = data->line,
            .position = pos,
            .for_restore = false,
        };
        addLabelToList(data->label_list, entry);
    } else {
        AstVar* var = (AstVar*)ast->value;
        int len = strlen(var->name);
        char* sym = (char*)allocAligned(data->variable_mem, len + 1);
        memcpy(sym, var->name, len + 1);
        UnhandeledLabelEntry entry = {
            .name = sym,
            .line = data->line,
            .position = pos,
            .for_restore = false,
        };
        addLabelToList(data->label_list, entry);
    }
    Value ret = {.type = VALUE_NONE};
    return ret;
}

static Value generateMCNext(AstUnary* ast, MCGenerationData* data) {
    AstVar* var = (AstVar*)ast->value;
    Variable* variable = getVariable(data->variable_table, var->name);
    if (variable == NULL) {
        Value ret = {.type = VALUE_ERROR, .error = ERROR_VARIABLE_NOT_DEF};
        return ret;
    } else if (var->var_type != VAR_UNDEF) {
        if ((variable->type == VARIABLE_INT && var->var_type != VAR_INT) || (variable->type == VARIABLE_FLOAT && var->var_type != VAR_FLOAT) || (variable->type == VARIABLE_STRING || var->var_type == VAR_STR)) {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
            return ret;
        }
    }
    size_t jmp_for_cond = ~0;
    size_t jmp_for_pos = ~0;
    if (variable->type == VARIABLE_INT) {
        VariableInt* varib = (VariableInt*)variable;
        jmp_for_cond = varib->for_call_loc;
        jmp_for_pos = varib->for_jmp_loc;
    } else if (variable->type == VARIABLE_FLOAT) {
        VariableFloat* varib = (VariableFloat*)variable;
        jmp_for_cond = varib->for_call_loc;
        jmp_for_pos = varib->for_jmp_loc;
    }
    if (jmp_for_cond == ~0UL || jmp_for_pos == ~0UL) {
        Value ret = {.type = VALUE_ERROR, .error = ERROR_NO_MATCHING_FOR};
        return ret;
    }
    addInstPushAll(data->inst_mem, data->registers, data->registers);
    RegisterSet tmp_regs = data->registers;
    data->registers = 0;
    Register ret_reg = getFirstRegister();
    data->registers |= ret_reg;
    size_t pos = addInstCallRel(data->inst_mem, data->registers, 0);
    updateRelativeJumpTarget(data->inst_mem, pos, jmp_for_cond);
    Register cmp_reg = getFreeRegister(data->registers);
    data->registers |= cmp_reg;
    addInstMovImmToReg(data->inst_mem, data->registers, cmp_reg, 0);
    pos = addInstCondJmpRel(data->inst_mem, data->registers, COND_EQ, ret_reg, cmp_reg, 0);
    updateRelativeJumpTarget(data->inst_mem, pos, jmp_for_pos);
    data->registers = tmp_regs;
    addInstPopAll(data->inst_mem, data->registers, data->registers);
    Value ret = {.type = VALUE_NONE};
    return ret;
}

static Value generateMCRestoreAfterFreeReg(AstUnary* ast, MCGenerationData* data) {
    Register reg = getFreeRegister(data->registers);
    data->registers |= reg;
    size_t pos = addInstMovImmToReg(data->inst_mem, data->registers, reg, 0);
    addInstMovRegToMem(data->inst_mem, data->registers, reg, (void*)&data_index);
    data->registers &= ~reg;
    if (ast->value != NULL) {
        if (ast->value->type == AST_INTEGER) {
            char name[25];
            AstInt* line = (AstInt*)ast->value;
            int len = int64ToString(name, line->value);
            char* sym = (char*)allocAligned(data->variable_mem, len + 1);
            memcpy(sym, name, len + 1);
            UnhandeledLabelEntry entry = {
                .name = sym,
                .line = data->line,
                .position = pos,
                .for_restore = true,
            };
            addLabelToList(data->label_list, entry);
        } else {
            AstVar* var = (AstVar*)ast->value;
            int len = strlen(var->name);
            char* sym = (char*)allocAligned(data->variable_mem, len + 1);
            memcpy(sym, var->name, len + 1);
            UnhandeledLabelEntry entry = {
                .name = sym,
                .line = data->line,
                .position = pos,
                .for_restore = true,
            };
            addLabelToList(data->label_list, entry);
        }
    }
    Value ret = {.type = VALUE_NONE};
    return ret;
}

static Value generateMCRestore(AstUnary* ast, MCGenerationData* data) {
    return withFreeRegister((Ast*)ast, data, (GenerateMCFunction)generateMCRestoreAfterFreeReg, 1, 0);
}

static char* concatStrings(char* a, char* b) {
    int a_len = a == NULL ? 0 : strlen(a);
    int b_len = b == NULL ? 0 : strlen(b);
    char* ret = (char*)allocAligned(&global_exec_alloc, a_len + b_len + 1);
    if (a != NULL) {
        memcpy(ret, a, a_len);
    }
    if (b != NULL) {
        memcpy(ret + a_len, b, b_len);
    }
    ret[a_len + b_len] = 0;
    return ret;
}

static Value generateMCBinarayOperationAfterFreeReg(AstBinary* ast, MCGenerationData* data) {
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
                    switch (ast->type) {
                    case AST_ADD:
                        addInstFAdd(data->inst_mem, data->registers, b.reg, freg, b.reg);
                        break;
                    case AST_SUB:
                        addInstFSub(data->inst_mem, data->registers, b.reg, freg, b.reg);
                        break;
                    case AST_MUL:
                        addInstFMul(data->inst_mem, data->registers, b.reg, freg, b.reg);
                        break;
                    case AST_DIV:
                        addInstFDiv(data->inst_mem, data->registers, b.reg, freg, b.reg);
                        break;
                    default: {
                        Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
                        return ret;
                    }
                    }
                    data->registers &= ~freg;
                    data->registers &= ~a.reg;
                    return b;
                } else if (a.type == VALUE_FLOAT && b.type == VALUE_INT) {
                    Register freg = getFreeFRegister(data->registers);
                    data->registers |= freg;
                    addInstMovRegToFReg(data->inst_mem, data->registers, freg, b.reg);
                    switch (ast->type) {
                    case AST_ADD:
                        addInstFAdd(data->inst_mem, data->registers, a.reg, a.reg, freg);
                        break;
                    case AST_SUB:
                        addInstFSub(data->inst_mem, data->registers, a.reg, a.reg, freg);
                        break;
                    case AST_MUL:
                        addInstFMul(data->inst_mem, data->registers, a.reg, a.reg, freg);
                        break;
                    case AST_DIV:
                        addInstFDiv(data->inst_mem, data->registers, a.reg, a.reg, freg);
                        break;
                    default: {
                        Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
                        return ret;
                    }
                    }
                    data->registers &= ~freg;
                    data->registers &= ~(b.reg);
                    return a;
                } else {
                    Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
                    return ret;
                }
            } else {
                if (a.type == VALUE_INT) {
                    switch (ast->type) {
                    case AST_ADD:
                        addInstAdd(data->inst_mem, data->registers, a.reg, a.reg, b.reg);
                        break;
                    case AST_SUB:
                        addInstSub(data->inst_mem, data->registers, a.reg, a.reg, b.reg);
                        break;
                    case AST_MUL:
                        addInstMul(data->inst_mem, data->registers, a.reg, a.reg, b.reg);
                        break;
                    case AST_DIV:
                        addInstDiv(data->inst_mem, data->registers, a.reg, a.reg, b.reg);
                        break;
                    case AST_AND:
                        addInstAnd(data->inst_mem, data->registers, a.reg, a.reg, b.reg);
                        break;
                    case AST_XOR:
                        addInstXor(data->inst_mem, data->registers, a.reg, a.reg, b.reg);
                        break;
                    case AST_OR:
                        addInstOr(data->inst_mem, data->registers, a.reg, a.reg, b.reg);
                        break;
                    case AST_MOD:
                        addInstRem(data->inst_mem, data->registers, a.reg, a.reg, b.reg);
                        break;
                    default:
                        break;
                    }
                } else if (a.type == VALUE_FLOAT) {
                    switch (ast->type) {
                    case AST_ADD:
                        addInstFAdd(data->inst_mem, data->registers, a.reg, a.reg, b.reg);
                        break;
                    case AST_SUB:
                        addInstFSub(data->inst_mem, data->registers, a.reg, a.reg, b.reg);
                        break;
                    case AST_MUL:
                        addInstFMul(data->inst_mem, data->registers, a.reg, a.reg, b.reg);
                        break;
                    case AST_DIV:
                        addInstFDiv(data->inst_mem, data->registers, a.reg, a.reg, b.reg);
                        break;
                    default: {
                        Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
                        return ret;
                    }
                    }
                } else if (a.type == VALUE_STRING) {
                    switch (ast->type) {
                    case AST_ADD:
                        addInstFunctionCallBinary(data->inst_mem, data->registers, a.reg, a.reg, b.reg, concatStrings);
                        break;
                    default: {
                        Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
                        return ret;
                    }
                    }
                } else if (a.type == VALUE_BOOLEAN) {
                    switch (ast->type) {
                    case AST_AND:
                        addInstAnd(data->inst_mem, data->registers, a.reg, a.reg, b.reg);
                        break;
                    case AST_XOR:
                        addInstXor(data->inst_mem, data->registers, a.reg, a.reg, b.reg);
                        break;
                    case AST_OR:
                        addInstOr(data->inst_mem, data->registers, a.reg, a.reg, b.reg);
                        break;
                    default: {
                        Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
                        return ret;
                    }
                    }
                } else {
                    Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
                    return ret;
                }
                data->registers &= ~(b.reg);
                return a;
            }
        }
    }
}

static Value generateMCBinaryOperation(AstBinary* ast, MCGenerationData* data) {
    return withFreeRegister((Ast*)ast, data, (GenerateMCFunction)generateMCBinarayOperationAfterFreeReg, 2, 2);
}

static Value generateMCUnaryAfterFreeReg(AstUnary* ast, MCGenerationData* data) {
    Value a = generateMCForAst(ast->value, data);
    if (a.type == VALUE_ERROR) {
        return a;
    } else {
        if (a.type == VALUE_INT) {
            switch (ast->type) {
            case AST_NEG: {
                Register reg = getFreeRegister(data->registers);
                data->registers |= reg;
                addInstMovImmToReg(data->inst_mem, data->registers, reg, 0);
                addInstSub(data->inst_mem, data->registers, reg, reg, a.reg);
                data->registers &= ~a.reg;
                a.reg = reg;
            } break;
            case AST_NOT:
                addInstNot(data->inst_mem, data->registers, a.reg, a.reg);
                break;
            default: {
                Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
                return ret;
            }
            }
        } else if (a.type == VALUE_FLOAT) {
            switch (ast->type) {
            case AST_NEG: {
                Register freg = getFreeFRegister(data->registers);
                data->registers |= freg;
                addInstMovImmToFReg(data->inst_mem, data->registers, freg, 0);
                addInstFSub(data->inst_mem, data->registers, freg, freg, a.reg);
                data->registers &= ~a.reg;
                a.reg = freg;
            } break;
            default: {
                Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
                return ret;
            }
            }
        } else if (a.type == VALUE_BOOLEAN) {
            switch (ast->type) {
            case AST_NOT:
                addInstNot(data->inst_mem, data->registers, a.reg, a.reg);
                Register one = getFreeRegister(data->registers);
                data->registers |= one;
                addInstMovImmToReg(data->inst_mem, data->registers, one, 1);
                addInstAnd(data->inst_mem, data->registers, a.reg, a.reg, one);
                data->registers &= ~one;
                break;
            default: {
                Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
                return ret;
            }
            }
        } else {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
            return ret;
        }
        return a;
    }
}

static Value generateMCUnaryOperation(AstUnary* ast, MCGenerationData* data) {
    return withFreeRegister((Ast*)ast, data, (GenerateMCFunction)generateMCUnaryAfterFreeReg, 2, 2);
}

static Value generateMCString(AstString* ast, MCGenerationData* data) {
    int len = strlen(ast->str);
    char* value = (char*)allocAligned(data->variable_mem, len + 1);
    memcpy(value, ast->str, len + 1);
    Register reg = getFreeRegister(data->registers);
    data->registers |= reg;
    addInstMovImmToReg(data->inst_mem, data->registers, reg, (intptr_t)value);
    Value ret = {
        .type = VALUE_STRING,
        .reg = reg,
    };
    return ret;
}

static Value generateMCLabel(AstString* ast, MCGenerationData* data) {
    if (getVariable(data->label_table, ast->str) == NULL) {
        VariableLabel* var = (VariableLabel*)allocAligned(data->variable_mem, sizeof(VariableLabel));
        var->type = VARIABLE_LABEL;
        var->pos = data->inst_mem->occupied;
        var->data_pos = data->data_mem->count;
        addVariable(data->label_table, ast->str, (Variable*)var, data->variable_mem);
        Value ret = {.type = VALUE_NONE};
        return ret;
    } else {
        Value ret = {.type = VALUE_ERROR, .reg = ERROR_DUBLICATE_LABEL};
        return ret;
    }
}

static Value generateMCData(AstVariable* ast, MCGenerationData* data) {
    for (int i = 0; i < ast->count; i++) {
        DataElement data_element = {.integer = 0, .real = 0.0, .string = NULL};
        if (ast->values[i]->type == AST_INTEGER) {
            AstInt* value = (AstInt*)ast->values[i];
            data_element.integer = value->value;
            data_element.real = (double)value->value;
            data_element.string = NULL;
        } else if (ast->values[i]->type == AST_FLOAT) {
            AstFloat* value = (AstFloat*)ast->values[i];
            data_element.integer = (int64_t)value->value;
            data_element.real = value->value;
            data_element.string = NULL;
        } else if (ast->values[i]->type == AST_TRUE || ast->values[i]->type == AST_FALSE) {
            int64_t value = ast->values[i]->type == AST_TRUE ? 1 : 0;
            data_element.integer = value;
            data_element.real = value;
            data_element.string = NULL;
        } else {
            data_element.integer = 0;
            data_element.real = 0;
            AstString* value = (AstString*)ast->values[i];
            int len = strlen(value->str);
            char* v = (char*)allocAligned(data->variable_mem, len + 1);
            memcpy(v, value->str, len + 1);
            data_element.string = v;
        }
        addDataToList(data->data_mem, data_element);
    }
    Value ret = {.type = VALUE_NONE};
    return ret;
}

static Value generateMCIndexPositionAfterFreeReg(AstIndex* ast, MCGenerationData* data) {
    AstVar* var = (AstVar*)ast->name;
    Variable* variable = getVariable(data->variable_table, var->name);
    if (variable != NULL) {
        if (variable->type != VARIABLE_STRING_ARRAY && variable->type != VARIABLE_INT_ARRAY && variable->type != VARIABLE_FLOAT_ARRAY && variable->type != VARIABLE_BOOLEAN_ARRAY) {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
            return ret;
        } else if (var->var_type != VAR_UNDEF) {
            if (
                (variable->type == VARIABLE_INT_ARRAY && var->var_type != VAR_INT)
                || (variable->type == VARIABLE_FLOAT_ARRAY && var->var_type != VAR_FLOAT)
                || (variable->type == VARIABLE_STRING_ARRAY && var->var_type != VAR_STR)
                || (variable->type == VARIABLE_BOOLEAN_ARRAY && var->var_type != VAR_BOOL)
            ) {
                Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
                return ret;
            }
        }
        if (
            (variable->type == VARIABLE_INT_ARRAY && ((VariableIntArray*)variable)->dim_count != ast->count)
            || (variable->type == VARIABLE_FLOAT_ARRAY && ((VariableIntArray*)variable)->dim_count != ast->count)
            || (variable->type == VARIABLE_STRING_ARRAY && ((VariableStringArray*)variable)->dim_count != ast->count)
            || (variable->type == VARIABLE_BOOLEAN_ARRAY && ((VariableBooleanArray*)variable)->dim_count != ast->count)
        ) {
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
        } else if (variable->type == VARIABLE_BOOLEAN_ARRAY) {
            addInstMovImmToReg(data->inst_mem, data->registers, index_reg, (intptr_t)((VariableBooleanArray*)variable)->value);
            indexing_size = sizeof(int64_t);
        }
        for (int i = 0; i < ast->count; i++) {
            Value ind = generateMCForAst(ast->size[i], data);
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
            } else if (variable->type == VARIABLE_BOOLEAN_ARRAY) {
                indexing_size *= ((VariableBooleanArray*)variable)->size[i];
            }
        }
        data->registers &= ~imm_reg;
        Value ret = {.type = VALUE_INT, .reg=index_reg};
        return ret;
    } else {
        Value ret = {.type = VALUE_ERROR, .error = ERROR_ARRAY_NOT_DEF};
        return ret;
    }
}

static Value generateMCIndexPosition(AstIndex* ast, MCGenerationData* data) {
    return withFreeRegister((Ast*)ast, data, (GenerateMCFunction)generateMCIndexPositionAfterFreeReg, 3, 0);
}

static Value generateMCDataPositionAfterFreeReg(AstVar* ast, MCGenerationData* data) {
    Variable* variable = getVariable(data->variable_table, ast->name);
    if (variable == NULL) {
        Value ret = {.type = VALUE_ERROR, .error = ERROR_ARRAY_NOT_DEF};
        return ret;
    } else {
        Register imm_reg = getFreeRegister(data->registers);
        data->registers |= imm_reg;
        Register index_reg = getFreeRegister(data->registers);
        data->registers |= index_reg;
        Register data_reg = getFreeRegister(data->registers);
        data->registers |= data_reg;
        addInstMovMemToReg(data->inst_mem, data->registers, index_reg, (void*)&data_index);
        addInstMovMemToReg(data->inst_mem, data->registers, data_reg, (void*)&data->data_mem->data);
        addInstMovImmToReg(data->inst_mem, data->registers, imm_reg, sizeof(DataElement));
        addInstMul(data->inst_mem, data->registers, imm_reg, imm_reg, index_reg);
        addInstAdd(data->inst_mem, data->registers, data_reg, data_reg, imm_reg);
        addInstMovImmToReg(data->inst_mem, data->registers, imm_reg, 1);
        addInstAdd(data->inst_mem, data->registers, index_reg, index_reg, imm_reg);
        addInstMovRegToMem(data->inst_mem, data->registers, index_reg, (void*)&data_index);
        data->registers &= ~index_reg;
        if (variable->type == VARIABLE_INT || variable->type == VARIABLE_INT_ARRAY) {
            addInstMovImmToReg(data->inst_mem, data->registers, imm_reg, offsetof(DataElement, integer));
        } else if (variable->type == VARIABLE_FLOAT || variable->type == VARIABLE_FLOAT_ARRAY) {
            addInstMovImmToReg(data->inst_mem, data->registers, imm_reg, offsetof(DataElement, real));
        } else if (variable->type == VARIABLE_STRING || variable->type == VARIABLE_STRING_ARRAY) {
            addInstMovImmToReg(data->inst_mem, data->registers, imm_reg, offsetof(DataElement, string));
        } else if (variable->type == VARIABLE_BOOLEAN || variable->type == VARIABLE_BOOLEAN_ARRAY) {
            addInstMovImmToReg(data->inst_mem, data->registers, imm_reg, offsetof(DataElement, integer));
        }
        addInstAdd(data->inst_mem, data->registers, data_reg, data_reg, imm_reg);
        data->registers &= ~imm_reg;
        Value ret = {.type = VALUE_INT, .reg = data_reg};
        return ret;
    }
}

static Value generateMCDataPosition(AstVar* ast, MCGenerationData* data) {
    return withFreeRegister((Ast*)ast, data, (GenerateMCFunction)generateMCDataPositionAfterFreeReg, 2, 0);
}

static Value generateMCReadOfArrayAccessAfterFreeReg(AstIndex* ast, MCGenerationData* data) {
    AstVar* var = (AstVar*)ast->name;
    Value index = generateMCIndexPosition(ast, data);
    if (index.type == VALUE_ERROR) {
        return index;
    } else if (index.type != VALUE_INT) {
        Value ret = {.type=VALUE_ERROR, .error=ERROR_TYPE};
        return ret;
    } else {
        Value data_pos = generateMCDataPosition(var, data);
        if (data_pos.type == VALUE_ERROR) {
            return data_pos;
        } else if (data_pos.type != VALUE_INT) {
            Value ret = {.type=VALUE_ERROR, .error=ERROR_TYPE};
            return ret;
        } else {
            addInstMovMemRegToReg(data->inst_mem, data->registers, data_pos.reg, data_pos.reg);
            addInstMovRegToMemReg(data->inst_mem, data->registers, index.reg, data_pos.reg);
            data->registers &= ~index.reg;
            data->registers &= ~data_pos.reg;
            Value ret = {.type = VALUE_NONE};
            return ret;
        }
    }
}

static Value generateMCVariableEntry(AstVar* var, MCGenerationData* data, Value value) {
    Variable* variable = getVariable(data->variable_table, var->name);
    if (variable == NULL) {
        if ((var->var_type == VAR_UNDEF && (value.type == VALUE_FLOAT || value.type == VALUE_NONE)) || var->var_type == VAR_FLOAT) {
            VariableFloat* varib = (VariableFloat*)allocAligned(data->variable_mem, sizeof(VariableFloat));
            varib->type = VARIABLE_FLOAT;
            varib->for_jmp_loc = ~0;
            varib->for_call_loc = ~0;
            varib->value = 0.0;
            addVariable(data->variable_table, var->name, (Variable*)varib, data->variable_mem);
            variable = (Variable*)varib;
        } else if ((var->var_type == VAR_UNDEF && value.type == VALUE_INT) || var->var_type == VAR_INT) {
            VariableInt* varib = (VariableInt*)allocAligned(data->variable_mem, sizeof(VariableInt));
            varib->type = VARIABLE_INT;
            varib->for_jmp_loc = ~0;
            varib->for_call_loc = ~0;
            varib->value = 0;
            addVariable(data->variable_table, var->name, (Variable*)varib, data->variable_mem);
            variable = (Variable*)varib;
        } else if ((var->var_type == VAR_UNDEF && value.type == VALUE_STRING) || var->var_type == VAR_STR) {
            VariableString* varib = (VariableString*)allocAligned(data->variable_mem, sizeof(VariableString));
            varib->type = VARIABLE_STRING;
            varib->str = NULL;
            addVariable(data->variable_table, var->name, (Variable*)varib, data->variable_mem);
            variable = (Variable*)varib;
        } else if ((var->var_type == VAR_UNDEF && value.type == VALUE_BOOLEAN) || var->var_type == VAR_BOOL) {
            VariableBoolean* varib = (VariableBoolean*)allocAligned(data->variable_mem, sizeof(VariableBoolean));
            varib->type = VARIABLE_BOOLEAN;
            varib->value = 0;
            addVariable(data->variable_table, var->name, (Variable*)varib, data->variable_mem);
            variable = (Variable*)varib;
        } else {
            Value ret = {.type = VALUE_ERROR,.error=ERROR_VARIABLE_NOT_DEF};
            return ret;
        }
    } else if (var->var_type != VAR_UNDEF) {
        if ((variable->type == VARIABLE_INT && var->var_type != VAR_INT) || (variable->type == VARIABLE_FLOAT && var->var_type != VAR_FLOAT) || (variable->type == VARIABLE_STRING && var->var_type != VAR_STR) || (variable->type == VARIABLE_BOOLEAN && var->var_type != VAR_BOOL)) {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
            return ret;
        }
    } else if (variable->type != VARIABLE_FLOAT && variable->type != VARIABLE_INT && variable->type != VARIABLE_STRING && variable->type != VARIABLE_BOOLEAN) {
        Value ret = {.type = VALUE_ERROR, .error = ERROR_UNINDEXED_ARRAY};
        return ret;
    }
    Value ret = {.type = VALUE_NONE};
    return ret;
}

static Value generateMCReadAfterFreeReg(AstVariable* ast, MCGenerationData* data) {
    for (int i = 0; i < ast->count; i++) {
        if (ast->values[i]->type == AST_VAR) {
            AstVar* var = (AstVar*)ast->values[i];
            Value none = {.type=VALUE_NONE};
            Value var_gen = generateMCVariableEntry(var, data, none);
            if (var_gen.type == VALUE_ERROR) {
                return var_gen;
            }
            Variable* variable = getVariable(data->variable_table, var->name);
            Value data_pos = generateMCDataPosition(var, data);
            if (data_pos.type == VALUE_ERROR) {
                return data_pos;
            } else if (data_pos.type != VALUE_INT) {
                Value ret = {.type=VALUE_ERROR, .error=ERROR_TYPE};
                return ret;
            } else {
                addInstMovMemRegToReg(data->inst_mem, data->registers, data_pos.reg, data_pos.reg);
                if (variable->type == VARIABLE_INT) {
                    addInstMovRegToMem(data->inst_mem, data->registers, data_pos.reg, &((VariableInt*)variable)->value);
                } else if (variable->type == VARIABLE_FLOAT) {
                    addInstMovRegToMem(data->inst_mem, data->registers, data_pos.reg, &((VariableFloat*)variable)->value);
                } else if (variable->type == VARIABLE_STRING) {
                    addInstMovRegToMem(data->inst_mem, data->registers, data_pos.reg, &((VariableString*)variable)->str);
                } else if (variable->type == VARIABLE_BOOLEAN) {
                    addInstMovRegToMem(data->inst_mem, data->registers, data_pos.reg, &((VariableBoolean*)variable)->value);
                }
                data->registers &= ~data_pos.reg;
            }
        } else if (ast->values[i]->type == AST_INDEX) {
            AstIndex* index = (AstIndex*)ast->values[i];
            Value ret = withFreeRegister((Ast*)index, data, (GenerateMCFunction)generateMCReadOfArrayAccessAfterFreeReg, 4, 0);
            if (ret.type == VALUE_ERROR) {
                return ret;
            }
        } else {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_SYNTAX};
            return ret;
        }
    }
    Value ret = {.type = VALUE_NONE};
    return ret;
}

static Value generateMCRead(AstVariable* ast, MCGenerationData* data) {
    return withFreeRegister((Ast*)ast, data, (GenerateMCFunction)generateMCReadAfterFreeReg, 3, 0);
}

static Value generateMCMultiple(AstVariable* ast, MCGenerationData* data) {
    for (int i = 0; i < ast->count; i++) {
        data->registers = 0;
        Value ret = generateMCForAst(ast->values[i], data);
        if (ret.type == VALUE_ERROR) {
            return ret;
        }
    }
    Value ret = {.type = VALUE_NONE};
    return ret;
}

static Value generateMCLetOfArrayAccessAfterFreeReg(AstLet* ast, MCGenerationData* data) {
    AstIndex* index = (AstIndex*)ast->name;
    AstVar* var = (AstVar*)index->name;
    Variable* variable = getVariable(data->variable_table, var->name);
    Value index_pos = generateMCIndexPosition(index, data);
    if (index_pos.type == VALUE_ERROR) {
        return index_pos;
    } else if (index_pos.type != VALUE_INT) {
        Value ret = {.type=VALUE_ERROR, .error=ERROR_TYPE};
        return ret;
    } else {
        Value a = generateMCForAst(ast->value, data);
        if (a.type == VALUE_ERROR) {
            return a;
        } else if (a.type == VALUE_NONE) {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_SYNTAX};
            return ret;
        } else if (
            (variable->type == VARIABLE_INT_ARRAY && a.type == VALUE_INT)
            || (variable->type == VARIABLE_STRING_ARRAY && a.type == VALUE_STRING)
            || (variable->type == VARIABLE_BOOLEAN_ARRAY && a.type == VALUE_BOOLEAN)
        ) {
            addInstMovRegToMemReg(data->inst_mem, data->registers, index_pos.reg, a.reg);
        } else if (variable->type == VARIABLE_FLOAT_ARRAY && a.type == VALUE_FLOAT) {
            addInstMovFRegToMemReg(data->inst_mem, data->registers, index_pos.reg, a.reg);
        } else if (variable->type == VARIABLE_FLOAT_ARRAY && a.type == VALUE_INT) {
            Register freg = getFreeFRegister(data->registers);
            data->registers |= freg;
            addInstMovRegToFReg(data->inst_mem, data->registers, freg, a.reg);
            addInstMovFRegToMemReg(data->inst_mem, data->registers, index_pos.reg, freg);
            data->registers &= ~freg;
        } else {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
            return ret;
        }
        data->registers &= ~a.reg;
        data->registers &= ~index_pos.reg;
        Value ret = {.type = VALUE_NONE};
        return ret;
    }
}

static Value generateMCLetAfterFreeReg(AstLet* ast, MCGenerationData* data) {
    if (ast->name->type == AST_VAR) {
        Value a = generateMCForAst(ast->value, data);
        if (a.type == VALUE_ERROR) {
            return a;
        } else if (a.type == VALUE_NONE) {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_SYNTAX};
            return ret;
        } else {
            AstVar* var = (AstVar*)ast->name;
            Value var_gen = generateMCVariableEntry(var, data, a);
            if (var_gen.type == VALUE_ERROR) {
                return var_gen;
            }
            Variable* variable = getVariable(data->variable_table, var->name);
            if (variable->type == VARIABLE_INT && a.type == VALUE_INT) {
                addInstMovRegToMem(data->inst_mem, data->registers, a.reg, &((VariableInt*)variable)->value);
            } else if (variable->type == VARIABLE_FLOAT && a.type == VALUE_FLOAT) {
                addInstMovFRegToMem(data->inst_mem, data->registers, a.reg, &((VariableFloat*)variable)->value);
            } else if (variable->type == VARIABLE_STRING && a.type == VALUE_STRING) {
                addInstMovRegToMem(data->inst_mem, data->registers, a.reg, &((VariableString*)variable)->str);
            } else if (variable->type == VARIABLE_BOOLEAN && a.type == VALUE_BOOLEAN) {
                addInstMovRegToMem(data->inst_mem, data->registers, a.reg, &((VariableBoolean*)variable)->value);
            } else if (variable->type == VARIABLE_FLOAT && a.type == VALUE_INT) {
                Register freg = getFreeFRegister(data->registers);
                data->registers |= freg;
                addInstMovRegToFReg(data->inst_mem, data->registers, freg, a.reg);
                addInstMovFRegToMem(data->inst_mem, data->registers, freg, &((VariableFloat*)variable)->value);
                data->registers &= ~freg;
            } else {
                Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
                return ret;
            }
            data->registers &= ~a.reg;
            Value ret = {.type = VALUE_NONE};
            return ret;
        }
    } else if (ast->name->type == AST_INDEX) {
        Value ret = withFreeRegister((Ast*)ast, data, (GenerateMCFunction)generateMCLetOfArrayAccessAfterFreeReg, 3, 1);
        return ret;
    } else {
        Value ret = {.type = VALUE_ERROR, .error = ERROR_SYNTAX};
        return ret;
    }
}

static Value generateMCLet(AstLet* ast, MCGenerationData* data) {
    return withFreeRegister((Ast*)ast, data, (GenerateMCFunction)generateMCLetAfterFreeReg, 1, 1);
}

static int64_t compareStrings(char* a, char* b) {
    return strcmp(a == NULL ? "" : a, b == NULL ? "" : b);
}

static Value generateMCConditionAfterFreeReg(AstBinary* ast, MCGenerationData* data) {
    size_t ifendjmp = 0;
    size_t elsejmp = 0;
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
            if (b.type != a.type) {
                if (b.type == VALUE_INT && a.type == VALUE_FLOAT) {
                    Register freg = getFreeFRegister(data->registers);
                    data->registers |= freg;
                    addInstMovRegToFReg(data->inst_mem, data->registers, freg, b.reg);
                    data->registers &= ~b.reg;
                    b.type = VALUE_FLOAT;
                    b.reg = freg;
                } else if (b.type == VALUE_FLOAT && a.type == VALUE_INT) {
                    Register freg = getFreeFRegister(data->registers);
                    data->registers |= freg;
                    addInstMovRegToFReg(data->inst_mem, data->registers, freg, a.reg);
                    data->registers &= ~a.reg;
                    a.type = VALUE_FLOAT;
                    a.reg = freg;
                } else {
                    Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
                    return ret;
                }
            }
            if (a.type == VALUE_INT || a.type == VALUE_FLOAT || a.type == VALUE_STRING || a.type == VALUE_BOOLEAN) {
                if (a.type == VALUE_STRING) {
                    addInstFunctionCallBinary(data->inst_mem, data->registers, a.reg, a.reg, b.reg, compareStrings);
                    a.type = VALUE_INT;
                    addInstMovImmToReg(data->inst_mem, data->registers, b.reg, 0);
                    b.type = VALUE_INT;
                }
                switch (ast->type) {
                case AST_EQ:
                    elsejmp = addInstCondJmpRel(data->inst_mem, data->registers, COND_NE, a.reg, b.reg, 0);
                    break;
                case AST_NE:
                    elsejmp = addInstCondJmpRel(data->inst_mem, data->registers, COND_EQ, a.reg, b.reg, 0);
                    break;
                case AST_LT:
                    elsejmp = addInstCondJmpRel(data->inst_mem, data->registers, COND_GE, a.reg, b.reg, 0);
                    break;
                case AST_GT:
                    elsejmp = addInstCondJmpRel(data->inst_mem, data->registers, COND_LE, a.reg, b.reg, 0);
                    break;
                case AST_LE:
                    elsejmp = addInstCondJmpRel(data->inst_mem, data->registers, COND_GT, a.reg, b.reg, 0);
                    break;
                case AST_GE:
                    elsejmp = addInstCondJmpRel(data->inst_mem, data->registers, COND_LT, a.reg, b.reg, 0);
                    break;
                default:
                    break;
                }
            } else {
                Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
                return ret;
            }
            data->registers &= ~a.reg;
            data->registers &= ~b.reg;
            Register reg = getFreeRegister(data->registers);
            data->registers |= reg;
            addInstMovImmToReg(data->inst_mem, data->registers, reg, 1);
            ifendjmp = addInstJmpRel(data->inst_mem, data->registers, 0);
            updateRelativeJumpTarget(data->inst_mem, elsejmp, data->inst_mem->occupied);
            addInstMovImmToReg(data->inst_mem, data->registers, reg, 0);
            updateRelativeJumpTarget(data->inst_mem, ifendjmp, data->inst_mem->occupied);
            Value ret = {
                .type = VALUE_BOOLEAN,
                .reg = reg,
            };
            return ret;
        }
    }
}

static Value generateMCCondition(AstBinary* ast, MCGenerationData* data) {
    return withFreeRegister((Ast*)ast, data, (GenerateMCFunction)generateMCConditionAfterFreeReg, 2, 2);
}

static Value generateMCIfThenElseAfterFreeReg(AstIfThenElse* ast, MCGenerationData* data) {
    size_t ifendjmp = 0;
    size_t elsejmp = 0;
    Value cond = generateMCForAst(ast->condition, data);
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
        elsejmp = addInstCondJmpRel(data->inst_mem, data->registers, COND_EQ, cond.reg, zero, 0);
        data->registers &= ~cond.reg;
        data->registers &= ~zero;
        Value if_block = generateMCForAst(ast->if_true, data);
        if (if_block.type == VALUE_ERROR) {
            return if_block;
        } else {
            if (ast->if_false == NULL) {
                updateRelativeJumpTarget(data->inst_mem, elsejmp, data->inst_mem->occupied);
                Value ret = {.type = VALUE_NONE};
                return ret;
            } else {
                ifendjmp = addInstJmpRel(data->inst_mem, data->registers, 0);
                updateRelativeJumpTarget(data->inst_mem, elsejmp, data->inst_mem->occupied);
                Value else_block = generateMCForAst(ast->if_false, data);
                if (else_block.type == VALUE_ERROR) {
                    return else_block;
                } else {
                    updateRelativeJumpTarget(data->inst_mem, ifendjmp, data->inst_mem->occupied);
                    Value ret = {.type = VALUE_NONE};
                    return ret;
                }
            }
        }
    }
}

static Value generateMCIfThenElse(AstIfThenElse* ast, MCGenerationData* data) {
    return withFreeRegister((Ast*)ast, data, (GenerateMCFunction)generateMCIfThenElseAfterFreeReg, 2, 2);
}

static Value generateMCForAfterFreeReg(AstFor* ast, MCGenerationData* data) {
    Value initial = generateMCForAst(ast->start, data);
    if (initial.type == VALUE_ERROR) {
        return initial;
    } else if (initial.type == VALUE_NONE) {
        Value ret = {.type = VALUE_ERROR, .error = ERROR_SYNTAX};
        return ret;
    } else {
        AstVar* var = (AstVar*)ast->variable;
        Value var_gen = generateMCVariableEntry(var, data, initial);
        if (var_gen.type == VALUE_ERROR) {
            return var_gen;
        }
        Variable* variable = getVariable(data->variable_table, var->name);
        if (variable->type == VARIABLE_INT && initial.type == VALUE_INT) {
            addInstMovRegToMem(data->inst_mem, data->registers, initial.reg, &((VariableInt*)variable)->value);
        } else if (variable->type == VARIABLE_FLOAT && initial.type == VALUE_FLOAT) {
            addInstMovFRegToMem(data->inst_mem, data->registers, initial.reg, &((VariableFloat*)variable)->value);
        } else if (variable->type == VARIABLE_FLOAT && initial.type == VALUE_INT) {
            Register freg = getFreeFRegister(data->registers);
            data->registers |= freg;
            addInstMovRegToFReg(data->inst_mem, data->registers, freg, initial.reg);
            addInstMovFRegToMem(data->inst_mem, data->registers, freg, &((VariableFloat*)variable)->value);
            data->registers &= ~freg;
        } else {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
            return ret;
        }
        data->registers &= ~initial.reg;
        size_t jmp_after_cond = addInstJmpRel(data->inst_mem, data->registers, 0);
        // This is the start of a subroutine
        RegisterSet tmp_regs = data->registers;
        data->registers = 0;
        if (variable->type == VARIABLE_INT) {
            VariableInt* varib = (VariableInt*)variable;
            varib->for_call_loc = data->inst_mem->occupied;
        } else if (variable->type == VARIABLE_FLOAT) {
            VariableFloat* varib = (VariableFloat*)variable;
            varib->for_call_loc = data->inst_mem->occupied;
        }
        Register ret_reg = getFirstRegister();
        data->registers |= ret_reg;
        addInstMovImmToReg(data->inst_mem, data->registers, ret_reg, 0);
        Register vreg = 0;
        if (variable->type == VARIABLE_INT) {
            vreg = getFreeRegister(data->registers);
            data->registers |= vreg;
            addInstMovMemToReg(data->inst_mem, data->registers, vreg, &((VariableInt*)variable)->value);
        } else if (variable->type == VARIABLE_FLOAT) {
            vreg = getFreeFRegister(data->registers);
            data->registers |= vreg;
            addInstMovMemToFReg(data->inst_mem, data->registers, vreg, &((VariableFloat*)variable)->value);
        }
        if (ast->step) {
            Value step = generateMCForAst(ast->step, data);
            if (step.type == VALUE_ERROR) {
                return step;
            } else if (step.type == VALUE_NONE) {
                Value ret = {.type = VALUE_ERROR, .error = ERROR_SYNTAX};
                return ret;
            } else if (variable->type == VARIABLE_INT && step.type == VALUE_INT) {
                addInstAdd(data->inst_mem, data->registers, vreg, vreg, step.reg);
                addInstMovRegToMem(data->inst_mem, data->registers, vreg, &((VariableInt*)variable)->value);
            } else if (variable->type == VARIABLE_FLOAT && step.type == VALUE_FLOAT) {
                addInstFAdd(data->inst_mem, data->registers, vreg, vreg, step.reg);
                addInstMovFRegToMem(data->inst_mem, data->registers, vreg, &((VariableFloat*)variable)->value);
            } else if (variable->type == VARIABLE_FLOAT && step.type == VALUE_INT) {
                Register freg = getFreeFRegister(data->registers);
                data->registers |= freg;
                addInstMovRegToFReg(data->inst_mem, data->registers, freg, step.reg);
                addInstFAdd(data->inst_mem, data->registers, vreg, vreg, freg);
                data->registers &= ~freg;
                addInstMovFRegToMem(data->inst_mem, data->registers, vreg, &((VariableFloat*)variable)->value);
            } else {
                Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
                return ret;
            }
            data->registers &= ~step.reg;
        } else {
            if (variable->type == VARIABLE_INT) {
                Register reg = getFreeRegister(data->registers);
                data->registers |= reg;
                addInstMovImmToReg(data->inst_mem, data->registers, reg, 1);
                addInstAdd(data->inst_mem, data->registers, vreg, vreg, reg);
                data->registers &= ~reg;
                addInstMovRegToMem(data->inst_mem, data->registers, vreg, &((VariableInt*)variable)->value);
            } else if (variable->type == VARIABLE_FLOAT) {
                Register freg = getFreeFRegister(data->registers);
                data->registers |= freg;
                addInstMovImmToFReg(data->inst_mem, data->registers, freg, 1.0);
                addInstFAdd(data->inst_mem, data->registers, vreg, vreg, freg);
                data->registers &= ~freg;
                addInstMovFRegToMem(data->inst_mem, data->registers, vreg, &((VariableFloat*)variable)->value);
            }
        }
        size_t jmp_to_ret;
        Value end = generateMCForAst(ast->end, data);
        if (end.type == VALUE_ERROR) {
            return end;
        } else if (end.type == VALUE_NONE) {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_SYNTAX};
            return ret;
        } else if ((variable->type == VARIABLE_INT && end.type == VALUE_INT) || (variable->type == VARIABLE_FLOAT && end.type == VALUE_FLOAT)) {
            jmp_to_ret = addInstCondJmpRel(data->inst_mem, data->registers, COND_LE, vreg, end.reg, 0);
        } else if (variable->type == VARIABLE_FLOAT && end.type == VALUE_INT) {
            Register freg = getFreeFRegister(data->registers);
            data->registers |= freg;
            addInstMovRegToFReg(data->inst_mem, data->registers, freg, end.reg);
            jmp_to_ret = addInstCondJmpRel(data->inst_mem, data->registers, COND_LE, vreg, freg, 0);
            data->registers &= ~freg;
        } else {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
            return ret;
        }
        data->registers &= ~end.reg;
        addInstMovImmToReg(data->inst_mem, data->registers, ret_reg, 1);
        updateRelativeJumpTarget(data->inst_mem, jmp_to_ret, data->inst_mem->occupied);
        addInstReturn(data->inst_mem, data->registers);
        // This is the end of the subroutine
        data->registers = tmp_regs;
        updateRelativeJumpTarget(data->inst_mem, jmp_after_cond, data->inst_mem->occupied);
        if (variable->type == VARIABLE_INT) {
            VariableInt* varib = (VariableInt*)variable;
            varib->for_jmp_loc = data->inst_mem->occupied;
        } else if (variable->type == VARIABLE_FLOAT) {
            VariableFloat* varib = (VariableFloat*)variable;
            varib->for_jmp_loc = data->inst_mem->occupied;
        }
        Value ret = {.type = VALUE_NONE};
        return ret;
    }
}

static Value generateMCFor(AstFor* ast, MCGenerationData* data) {
    return withFreeRegister((Ast*)ast, data, (GenerateMCFunction)generateMCForAfterFreeReg, 2, 2);
}

static Value generateMCVarAfterFreeReg(AstVar* ast, MCGenerationData* data) {
    Value none = {.type=VALUE_NONE};
    Value var_gen = generateMCVariableEntry(ast, data, none);
    if (var_gen.type == VALUE_ERROR) {
        return var_gen;
    }
    Variable* variable = getVariable(data->variable_table, ast->name);
    Register imm_reg;
    Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
    if (variable->type == VARIABLE_INT) {
        imm_reg = getFreeRegister(data->registers);
        data->registers |= imm_reg;
        addInstMovMemToReg(data->inst_mem, data->registers, imm_reg, &((VariableInt*)variable)->value);
        ret.type = VALUE_INT;
        ret.reg = imm_reg;
    } else if (variable->type == VARIABLE_FLOAT) {
        imm_reg = getFreeFRegister(data->registers);
        data->registers |= imm_reg;
        addInstMovMemToFReg(data->inst_mem, data->registers, imm_reg, &((VariableFloat*)variable)->value);
        ret.type = VALUE_FLOAT;
        ret.reg = imm_reg;
    } else if (variable->type == VARIABLE_STRING) {
        imm_reg = getFreeRegister(data->registers);
        data->registers |= imm_reg;
        addInstMovMemToReg(data->inst_mem, data->registers, imm_reg, &((VariableString*)variable)->str);
        ret.type = VALUE_STRING;
        ret.reg = imm_reg;
    } else if (variable->type == VARIABLE_BOOLEAN) {
        imm_reg = getFreeRegister(data->registers);
        data->registers |= imm_reg;
        addInstMovMemToReg(data->inst_mem, data->registers, imm_reg, &((VariableBoolean*)variable)->value);
        ret.type = VALUE_BOOLEAN;
        ret.reg = imm_reg;
    }
    return ret;
}

static Value generateMCVar(AstVar* ast, MCGenerationData* data) {
    return withFreeRegister((Ast*)ast, data, (GenerateMCFunction)generateMCVarAfterFreeReg, 1, 1);
}

static Value generateMCOnGoAfterFreeReg(AstSwitch* ast, MCGenerationData* data) {
    Value index = generateMCForAst(ast->value, data);
    if (index.type == VALUE_ERROR) {
        return index;
    } else if (index.type == VALUE_NONE) {
        Value ret = {.type = VALUE_ERROR, .error = ERROR_SYNTAX};
        return ret;
    } else if (index.type != VALUE_INT) {
        Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
        return ret;
    } else {
        Register imm_reg = getFreeRegister(data->registers);
        data->registers |= imm_reg;
        for (int i = 0; i < ast->count; i++) {
            addInstMovImmToReg(data->inst_mem, data->registers, imm_reg, i);
            size_t pos;
            if (ast->type == AST_ON_GOSUB) {
                size_t skip_call = addInstCondJmpRel(data->inst_mem, data->registers, COND_NE, index.reg, imm_reg, 0);
                addInstPush(data->inst_mem, data->registers, index.reg);
                pos = addInstCallRel(data->inst_mem, data->registers, 0);
                addInstPop(data->inst_mem, data->registers, index.reg);
                updateRelativeJumpTarget(data->inst_mem, skip_call, data->inst_mem->occupied);
            } else {
                pos = addInstCondJmpRel(data->inst_mem, data->registers, COND_EQ, index.reg, imm_reg, 0);
            }
            if (ast->locations[i]->type == AST_INTEGER) {
                char name[25];
                AstInt* line = (AstInt*)ast->locations[i];
                int len = int64ToString(name, line->value);
                char* sym = (char*)allocAligned(data->variable_mem, len + 1);
                memcpy(sym, name, len + 1);
                UnhandeledLabelEntry entry = {
                    .name = sym,
                    .line = data->line,
                    .position = pos,
                    .for_restore = false,
                };
                addLabelToList(data->label_list, entry);
            } else {
                AstVar* var = (AstVar*)ast->locations[i];
                int len = strlen(var->name);
                char* sym = (char*)allocAligned(data->variable_mem, len + 1);
                memcpy(sym, var->name, len + 1);
                UnhandeledLabelEntry entry = {
                    .name = sym,
                    .line = data->line,
                    .position = pos,
                    .for_restore = false,
                };
                addLabelToList(data->label_list, entry);
            }
        }
        data->registers &= ~imm_reg;
    }
    Value ret = {.type = VALUE_NONE};
    return ret;
}

static Value generateMCOnGo(AstSwitch* ast, MCGenerationData* data) {
    return withFreeRegister((Ast*)ast, data, (GenerateMCFunction)generateMCOnGoAfterFreeReg, 2, 0);
}

static Value generateMCDim(AstIndex* ast, MCGenerationData* data) {
    AstVar* var = ast->name;
    size_t size = 1;
    for (int i = 0; i < ast->count; i++) {
        AstInt* ds = (AstInt*)ast->size[i];
        size *= ds->value;
    }
    if (var->var_type == VAR_UNDEF || var->var_type == VAR_FLOAT) {
        VariableFloatArray* varib = (VariableFloatArray*)allocAligned(data->variable_mem, sizeof(VariableFloatArray));
        varib->type = VARIABLE_FLOAT_ARRAY;
        varib->dim_count = ast->count;
        varib->size = (int64_t*)allocAligned(data->variable_mem, sizeof(int64_t) * ast->count);
        for (int i = 0; i < ast->count; i++) {
            AstInt* ds = (AstInt*)ast->size[i];
            varib->size[i] = ds->value;
        }
        varib->value = (double*)allocAligned(data->variable_mem, sizeof(double) * size);
        for (size_t i = 0; i < size; i++) {
            varib->value[i] = 0;
        }
        addVariable(data->variable_table, var->name, (Variable*)varib, data->variable_mem);
    } else if (var->var_type == VAR_INT) {
        VariableIntArray* varib = (VariableIntArray*)allocAligned(data->variable_mem, sizeof(VariableIntArray));
        varib->type = VARIABLE_INT_ARRAY;
        varib->dim_count = ast->count;
        varib->size = (int64_t*)allocAligned(data->variable_mem, sizeof(int64_t) * ast->count);
        for (int i = 0; i < ast->count; i++) {
            AstInt* ds = (AstInt*)ast->size[i];
            varib->size[i] = ds->value;
        }
        varib->value = (int64_t*)allocAligned(data->variable_mem, sizeof(int64_t) * size);
        for (size_t i = 0; i < size; i++) {
            varib->value[i] = 0;
        }
        addVariable(data->variable_table, var->name, (Variable*)varib, data->variable_mem);
    } else if (var->var_type == VAR_STR) {
        VariableStringArray* varib = (VariableStringArray*)allocAligned(data->variable_mem, sizeof(VariableStringArray));
        varib->type = VARIABLE_STRING_ARRAY;
        varib->dim_count = ast->count;
        varib->size = (int64_t*)allocAligned(data->variable_mem, sizeof(int64_t) * ast->count);
        for (int i = 0; i < ast->count; i++) {
            AstInt* ds = (AstInt*)ast->size[i];
            varib->size[i] = ds->value;
        }
        varib->str = (char**)allocAligned(data->variable_mem, sizeof(char*) * size);
        for (size_t i = 0; i < size; i++) {
            varib->str[i] = NULL;
        }
        addVariable(data->variable_table, var->name, (Variable*)varib, data->variable_mem);
    } else if (var->var_type == VAR_BOOL) {
        VariableBooleanArray* varib = (VariableBooleanArray*)allocAligned(data->variable_mem, sizeof(VariableBooleanArray));
        varib->type = VARIABLE_BOOLEAN_ARRAY;
        varib->dim_count = ast->count;
        varib->size = (int64_t*)allocAligned(data->variable_mem, sizeof(int64_t) * ast->count);
        for (int i = 0; i < ast->count; i++) {
            AstInt* ds = (AstInt*)ast->size[i];
            varib->size[i] = ds->value;
        }
        varib->value = (int64_t*)allocAligned(data->variable_mem, sizeof(int64_t) * size);
        for (size_t i = 0; i < size; i++) {
            varib->value[i] = 0;
        }
        addVariable(data->variable_table, var->name, (Variable*)varib, data->variable_mem);
    }
    Value ret = {.type = VALUE_NONE};
    return ret;
}

static Value generateMCIndexAfterFreeReg(AstIndex* ast, MCGenerationData* data) {
    AstVar* var = (AstVar*)ast->name;
    Variable* variable = getVariable(data->variable_table, var->name);
    Value index = generateMCIndexPosition(ast, data);
    if (index.type == VALUE_ERROR) {
        return index;
    } else if (index.type != VALUE_INT) {
        Value ret = {.type=VALUE_ERROR, .error=ERROR_TYPE};
        return ret;
    } else {
        Register ret_reg;
        Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
        if (variable->type == VARIABLE_INT_ARRAY) {
            ret_reg = getFreeRegister(data->registers);
            data->registers |= ret_reg;
            addInstMovMemRegToReg(data->inst_mem, data->registers, ret_reg, index.reg);
            ret.type = VALUE_INT;
            ret.reg = ret_reg;
        } else if (variable->type == VARIABLE_FLOAT_ARRAY) {
            ret_reg = getFreeFRegister(data->registers);
            data->registers |= ret_reg;
            addInstMovMemRegToFReg(data->inst_mem, data->registers, ret_reg, index.reg);
            ret.type = VALUE_FLOAT;
            ret.reg = ret_reg;
        } else if (variable->type == VARIABLE_STRING_ARRAY) {
            ret_reg = getFreeRegister(data->registers);
            data->registers |= ret_reg;
            addInstMovMemRegToReg(data->inst_mem, data->registers, ret_reg, index.reg);
            ret.type = VALUE_STRING;
            ret.reg = ret_reg;
        } else if (variable->type == VARIABLE_BOOLEAN_ARRAY) {
            ret_reg = getFreeRegister(data->registers);
            data->registers |= ret_reg;
            addInstMovMemRegToReg(data->inst_mem, data->registers, ret_reg, index.reg);
            ret.type = VALUE_BOOLEAN;
            ret.reg = ret_reg;
        }
        data->registers &= ~index.reg;
        return ret;
    }
}

static Value generateMCIndex(AstIndex* ast, MCGenerationData* data) {
    return withFreeRegister((Ast*)ast, data, (GenerateMCFunction)generateMCIndexAfterFreeReg, 2, 1);
}

static Value generateMCReturn(Ast* ast, MCGenerationData* data) {
    addInstReturn(data->inst_mem, data->registers);
    Value ret = {.type = VALUE_NONE};
    return ret;
}

static Value generateMCInteger(AstInt* ast, MCGenerationData* data) {
    Register reg = getFreeRegister(data->registers);
    data->registers |= reg;
    addInstMovImmToReg(data->inst_mem, data->registers, reg, ast->value);
    Value ret = {
        .type = VALUE_INT,
        .reg = reg,
    };
    return ret;
}

static Value generateMCFloat(AstFloat* ast, MCGenerationData* data) {
    Register freg = getFreeFRegister(data->registers);
    data->registers |= freg;
    addInstMovImmToFReg(data->inst_mem, data->registers, freg, ast->value);
    Value ret = {
        .type = VALUE_FLOAT,
        .reg = freg,
    };
    return ret;
}

static Value generateMCToIntConvAfterFreeReg(AstUnary* ast, MCGenerationData* data) {
    Value a = generateMCForAst(ast->value, data);
    if (a.type == VALUE_ERROR) {
        return a;
    } else {
        if (a.type == VALUE_INT) {
            return a;
        } else if (a.type == VALUE_FLOAT) {
            Register reg = getFreeRegister(data->registers);
            data->registers |= reg;
            addInstMovFRegToReg(data->inst_mem, data->registers, reg, a.reg);
            data->registers &= ~a.reg;
            a.type = VALUE_INT;
            a.reg = reg;
            return a;
        } else if (a.type == VALUE_BOOLEAN) {
            a.type = VALUE_INT;
            return a;
        } else {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
            return ret;
        }
    }
}

static Value generateMCToIntConv(AstUnary* ast, MCGenerationData* data) {
    return withFreeRegister((Ast*)ast, data, (GenerateMCFunction)generateMCToIntConvAfterFreeReg, 1, 1);
}

static Value generateMCLineNum(AstLineNum* ast, MCGenerationData* data) {
    char name[25];
    int len = int64ToString(name, ast->number);
    char* sym = (char*)allocAligned(data->variable_mem, len + 1);
    memcpy(sym, name, len + 1);
    if (getVariable(data->label_table, sym) == NULL) {
        VariableLabel* var = (VariableLabel*)allocAligned(data->variable_mem, sizeof(VariableLabel));
        var->type = VARIABLE_LABEL;
        var->pos = data->inst_mem->occupied;
        var->data_pos = data->data_mem->count;
        addVariable(data->label_table, sym, (Variable*)var, data->variable_mem);
        if (ast->line != NULL) {
            return generateMCForAst(ast->line, data);
        } else {
            Value ret = {.type = VALUE_NONE};
            return ret;
        }
    } else {
        Value ret = {.type = VALUE_ERROR, .reg = ERROR_DUBLICATE_LABEL};
        return ret;
    }
}

static Value generateMCBoolean(Ast* ast, MCGenerationData* data) {
    Register reg = getFreeRegister(data->registers);
    data->registers |= reg;
    if (ast->type == AST_TRUE) {
        addInstMovImmToReg(data->inst_mem, data->registers, reg, 1);
    } else if (ast->type == AST_FALSE) {
        addInstMovImmToReg(data->inst_mem, data->registers, reg, 0);
    }
    Value ret = {
        .type = VALUE_BOOLEAN,
        .reg = reg,
    };
    return ret;
}

static Value generateMCDef(AstDef* ast, MCGenerationData* data) {
    size_t jump = addInstJmpRel(data->inst_mem, data->registers, 0);
    size_t call_target = data->inst_mem->occupied;
    RegisterSet old_regs = data->registers;
    data->registers = 0;
    Variable* global_params[ast->variable_count + 1];
    Variable* local_params[ast->variable_count + 1];
    for (int i = 0; i < ast->variable_count; i++) {
        global_params[i] = getVariable(data->variable_table, ast->variables[i]->name);
        if (global_params[i] != NULL) {
            removeVariable(data->variable_table, ast->variables[i]->name);
        }
        Value none = {.type=VALUE_NONE};
        Value var_gen = generateMCVariableEntry(ast->variables[i], data, none);
        if (var_gen.type == VALUE_ERROR) {
            return var_gen;
        }
        local_params[i] = getVariable(data->variable_table, ast->variables[i]->name);
    }
    VariableFunc* function = (VariableFunc*)allocAligned(data->variable_mem, sizeof(VariableFunc));
    function->type = VARIABLE_FUNC;
    function->pos = call_target;
    function->param_count = ast->variable_count;
    function->params = (Variable**)allocAligned(data->variable_mem, sizeof(Variable*) * ast->variable_count);
    for (int i = 0; i < ast->variable_count; i++) {
        function->params[i] = local_params[i];
    }
    if (ast->name->var_type == VAR_UNDEF || ast->name->var_type == VAR_FLOAT) {
        function->return_type = VALUE_FLOAT;
    } else if (ast->name->var_type == VAR_INT) {
        function->return_type = VALUE_INT;
    } else if (ast->name->var_type == VAR_STR) {
        function->return_type = VALUE_STRING;
    } else if (ast->name->var_type == VAR_BOOL) {
        function->return_type = VALUE_BOOLEAN;
    }
    addVariable(data->func_table, ast->name->name, (Variable*)function, data->variable_mem);
    Value retur = generateMCForAst(ast->function, data);
    function->return_type = retur.type;
    if (retur.type == VALUE_ERROR) {
        return retur;
    }
    if (retur.type == VALUE_FLOAT) {
        Register return_reg = getFirstFRegister();
        addInstMovFRegToFReg(data->inst_mem, data->registers, return_reg, retur.reg);
    } else if (retur.type == VALUE_INT || retur.type == VALUE_STRING || retur.type == VALUE_BOOLEAN) {
        Register return_reg = getFirstRegister();
        addInstMovRegToReg(data->inst_mem, data->registers, return_reg, retur.reg);
    } else {
        Value ret = { .type = VALUE_ERROR, .error = ERROR_TYPE };
        return ret;
    }
    for (int i = 0; i < ast->variable_count; i++) {
        removeVariable(data->variable_table, ast->variables[i]->name);
        if (global_params[i] != NULL) {
            addVariable(data->variable_table, ast->variables[i]->name, global_params[i], data->variable_mem);
        }
    }
    addInstReturn(data->inst_mem, data->registers);
    data->registers = old_regs;
    updateRelativeJumpTarget(data->inst_mem, jump, data->inst_mem->occupied);
    Value ret = { .type = VALUE_NONE, };
    return ret;
}

static Value generateMCExt(AstExt* ast, MCGenerationData* data) {
#ifndef NONATIVEFN
    VariableNativeFunc* function = (VariableNativeFunc*)allocAligned(data->variable_mem, sizeof(VariableNativeFunc));
    function->type = VARIABLE_NATIVE_FUNC;
    function->param_count = ast->variable_count;
    function->params = (ValueType*)allocAligned(data->variable_mem, sizeof(ValueType) * ast->variable_count);
    for (int i = 0; i < ast->variable_count; i++) {
        if (ast->variables[i]->var_type == VAR_UNDEF || ast->variables[i]->var_type == VAR_FLOAT) {
            function->params[i] = VALUE_FLOAT;
        } else if (ast->variables[i]->var_type == VAR_INT) {
            function->params[i] = VALUE_INT;
        } else if (ast->variables[i]->var_type == VAR_STR) {
            function->params[i] = VALUE_STRING;
        } else if (ast->variables[i]->var_type == VAR_BOOL) {
            function->params[i] = VALUE_BOOLEAN;
        }
    }
    if (ast->name->var_type == VAR_UNDEF || ast->name->var_type == VAR_FLOAT) {
        function->return_type = VALUE_FLOAT;
    } else if (ast->name->var_type == VAR_INT) {
        function->return_type = VALUE_INT;
    } else if (ast->name->var_type == VAR_STR) {
        function->return_type = VALUE_STRING;
    } else if (ast->name->var_type == VAR_BOOL) {
        function->return_type = VALUE_BOOLEAN;
    }
    char name[strlen(ast->name->name) + 1];
    strcpy(name, ast->name->name);
    static void* handle = NULL;
    if (handle == NULL) {
        handle = dlopen(NULL, RTLD_LAZY);
    }
    if (handle == NULL) {
        Value ret = { .type = VALUE_ERROR, .error = ERROR_FUNC_NOT_DEF };
        return ret;
    } else {
        function->function = dlsym(handle, name);
        if (function->function != NULL) {
            addVariable(data->func_table, ast->name->name, (Variable*)function, data->variable_mem);
            Value ret = { .type = VALUE_NONE, };
            return ret;
        } else {
            Value ret = { .type = VALUE_ERROR, .error = ERROR_FUNC_NOT_DEF };
            return ret;
        }
    }
#else
    Value ret = { .type = VALUE_ERROR, .error = ERROR_NOT_SUPPORTED };
    return ret;
#endif
}

static Value generateMCFn(AstFn* ast, MCGenerationData* data) {
    Variable* variable = (Variable*)getVariable(data->func_table, ast->name->name);
    if (variable == NULL) {
        Value ret = { .type = VALUE_ERROR, .error = ERROR_FUNC_NOT_DEF };
        return ret;
    }
    if (variable->type == VARIABLE_FUNC) {
        VariableFunc* function = (VariableFunc*)variable;
        Register ret_reg;
        if (function->return_type == VALUE_FLOAT) {
            ret_reg = getFreeFRegister(data->registers);
        } else {
            ret_reg = getFreeRegister(data->registers);
        }
        addInstPushAll(data->inst_mem, data->registers, data->registers);
        RegisterSet old_regs = data->registers;
        data->registers = 0;
        data->registers |= ret_reg;
        if (ast->value_count < function->param_count) {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_TO_FEW_PARAMS};
            return ret;
        } else if (ast->value_count > function->param_count) {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_TO_MANY_PARAMS};
            return ret;
        }
        void* param_ptrs[function->param_count + 1];
        for (int i = 0; i < ast->value_count; i++) {
            if (function->params[i]->type == VARIABLE_INT) {
                param_ptrs[i] = (void*)&((VariableInt*)function->params[i])->value;
            } else if (function->params[i]->type == VARIABLE_FLOAT) {
                param_ptrs[i] = &((VariableFloat*)function->params[i])->value;
            } else if (function->params[i]->type == VARIABLE_STRING) {
                param_ptrs[i] = &((VariableString*)function->params[i])->str;
            } else if (function->params[i]->type == VARIABLE_BOOLEAN) {
                param_ptrs[i] = &((VariableBoolean*)function->params[i])->value;
            } else {
                Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
                return ret;
            }
            Register old_val = getFreeRegister(data->registers);
            data->registers |= old_val;
            addInstMovMemToReg(data->inst_mem, data->registers, old_val, param_ptrs[i]);
            addInstPush(data->inst_mem, data->registers, old_val);
            data->registers &= ~old_val;
        }
        for (int i = 0; i < ast->value_count; i++) {
            Value a = generateMCForAst(ast->values[i], data);
            if (a.type == VALUE_ERROR) {
                return a;
            } else {
                if (
                    (a.type == VALUE_INT && function->params[i]->type == VARIABLE_INT)
                    || (a.type == VALUE_STRING && function->params[i]->type == VARIABLE_STRING)
                    || (a.type == VALUE_BOOLEAN && function->params[i]->type == VARIABLE_BOOLEAN)
                    || (a.type == VALUE_FLOAT && function->params[i]->type == VARIABLE_FLOAT)
                ) {
                    addInstPush(data->inst_mem, data->registers, a.reg);
                } else if (a.type == VALUE_INT && function->params[i]->type == VARIABLE_FLOAT) {
                    Register tmp_reg = getFreeFRegister(data->registers);
                    data->registers |= tmp_reg;
                    addInstMovRegToFReg(data->inst_mem, data->registers, tmp_reg, a.reg);
                    addInstPush(data->inst_mem, data->registers, tmp_reg);
                    data->registers &= ~tmp_reg;
                } else {
                    Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
                    return ret;
                }
                data->registers &= ~a.reg;
            }
        }
        for (int i = ast->value_count - 1; i >= 0; i--) {
            Register new_val = getFreeRegister(data->registers);
            data->registers |= new_val;
            addInstPop(data->inst_mem, data->registers, new_val);
            addInstMovRegToMem(data->inst_mem, data->registers, new_val, param_ptrs[i]);
            data->registers &= ~new_val;
        }
        addInstCallRel(data->inst_mem, data->registers, function->pos);
        if (function->return_type == VALUE_FLOAT) {
            addInstMovFRegToFReg(data->inst_mem, data->registers, ret_reg, getFirstFRegister());
        } else {
            addInstMovRegToReg(data->inst_mem, data->registers, ret_reg, getFirstRegister());
        }
        for (int i = ast->value_count - 1; i >= 0; i--) {
            Register old_val = getFreeRegister(data->registers);
            data->registers |= old_val;
            addInstPop(data->inst_mem, data->registers, old_val);
            addInstMovRegToMem(data->inst_mem, data->registers, old_val, param_ptrs[i]);
            data->registers &= ~old_val;
        }
        data->registers = old_regs;
        addInstPopAll(data->inst_mem, data->registers, data->registers);
        data->registers |= ret_reg;
        Value ret = { .type = function->return_type, .reg = ret_reg };
        return ret;
    } else if (variable->type == VARIABLE_NATIVE_FUNC) {
        // TODO: Find a way to do this without the register limit
        VariableNativeFunc* function = (VariableNativeFunc*)variable;
        Register arg_registers[function->param_count];
        if (ast->value_count < function->param_count) {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_TO_FEW_PARAMS};
            return ret;
        } else if (ast->value_count > function->param_count) {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_TO_MANY_PARAMS};
            return ret;
        }
        Value ret = { .type = function->return_type };
        if (function->return_type == VALUE_FLOAT) {
            ret.reg = getFreeFRegister(data->registers);
        } else {
            ret.reg = getFreeRegister(data->registers);
        }
        addInstPushAll(data->inst_mem, data->registers, data->registers);
        RegisterSet old_regs = data->registers;
        data->registers = ret.reg;
        for (int i = 0; i < function->param_count; i++) {
            Value argument = generateMCForAst(ast->values[i], data);
            if (argument.type == VALUE_ERROR) {
                return argument;
            } else if (argument.type == VALUE_INT && function->params[i] == VALUE_FLOAT) {
                Register tmp_reg = getFreeFRegister(data->registers);
                data->registers |= tmp_reg;
                addInstMovRegToFReg(data->inst_mem, data->registers, tmp_reg, argument.reg);
                data->registers &= ~argument.reg;
                argument.reg = tmp_reg;
            } else if (argument.type != function->params[i]) {
                Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
                return ret;
            }
            arg_registers[i] = argument.reg;
        }
        addInstFunctionCall(data->inst_mem, 0, ret.reg, function->param_count, arg_registers, function->function);
        addInstPopAll(data->inst_mem, data->registers, old_regs);
        data->registers |= old_regs;
        return ret;
    } else {
        Value ret = { .type = VALUE_ERROR, .error = ERROR_FUNC_NOT_DEF };
        return ret;
    }
}

static Value generateMCExprIfElseAfterFreeReg(AstIfThenElse* ast, MCGenerationData* data) {
    size_t ifendjmp = 0;
    size_t elsejmp = 0;
    Value cond = generateMCForAst(ast->condition, data);
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
        elsejmp = addInstCondJmpRel(data->inst_mem, data->registers, COND_EQ, cond.reg, zero, 0);
        data->registers &= ~cond.reg;
        data->registers &= ~zero;
        Value if_block = generateMCForAst(ast->if_true, data);
        if (if_block.type == VALUE_ERROR) {
            return if_block;
        } else {
            ifendjmp = addInstJmpRel(data->inst_mem, data->registers, 0);
            updateRelativeJumpTarget(data->inst_mem, elsejmp, data->inst_mem->occupied);
            Value else_block = generateMCForAst(ast->if_false, data);
            if (else_block.type == VALUE_ERROR) {
                return else_block;
            } else if (else_block.type != if_block.type) {
                Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
                return ret;
            } else {
                if (else_block.type == VALUE_FLOAT) {
                    addInstMovFRegToFReg(data->inst_mem, data->registers, if_block.reg, else_block.reg);
                } else {
                    addInstMovRegToReg(data->inst_mem, data->registers, if_block.reg, else_block.reg);
                }
                data->registers &= ~else_block.reg;
                updateRelativeJumpTarget(data->inst_mem, ifendjmp, data->inst_mem->occupied);
                return if_block;
            }
        }
    }
}

static Value generateMCExprIfElse(AstIfThenElse* ast, MCGenerationData* data) {
    return withFreeRegister((Ast*)ast, data, (GenerateMCFunction)generateMCExprIfElseAfterFreeReg, 2, 2);
}

Value generateMCForAst(Ast* ast, MCGenerationData* data) {
    Value value = {.type = VALUE_NONE};
    switch (ast->type) {
    case AST_GOTO:
    case AST_GOSUB:
        value = generateMCGo((AstUnary*)ast, data);
        break;
    case AST_NEXT:
        value = generateMCNext((AstUnary*)ast, data);
        break;
    case AST_RESTORE:
        value = generateMCRestore((AstUnary*)ast, data);
        break;
    case AST_ADD:
    case AST_SUB:
    case AST_MUL:
    case AST_DIV:
    case AST_MOD:
    case AST_AND:
    case AST_OR:
    case AST_XOR:
        value = generateMCBinaryOperation((AstBinary*)ast, data);
        break;
    case AST_NEG:
    case AST_NOT:
        value = generateMCUnaryOperation((AstUnary*)ast, data);
        break;
    case AST_STRING:
        value = generateMCString((AstString*)ast, data);
        break;
    case AST_LABEL:
        value = generateMCLabel((AstString*)ast, data);
        break;
    case AST_DATA:
        value = generateMCData((AstVariable*)ast, data);
        break;
    case AST_READ:
        value = generateMCRead((AstVariable*)ast, data);
        break;
    case AST_MULTIPLE:
        value = generateMCMultiple((AstVariable*)ast, data);
        break;
    case AST_LET:
        value = generateMCLet((AstLet*)ast, data);
        break;
    case AST_IF_THEN_ELSE:
        value = generateMCIfThenElse((AstIfThenElse*)ast, data);
        break;
    case AST_EXPR_IF_ELSE:
        value = generateMCExprIfElse((AstIfThenElse*)ast, data);
        break;
    case AST_FOR:
        value = generateMCFor((AstFor*)ast, data);
        break;
    case AST_VAR:
        value = generateMCVar((AstVar*)ast, data);
        break;
    case AST_ON_GOTO:
    case AST_ON_GOSUB:
        value = generateMCOnGo((AstSwitch*)ast, data);
        break;
    case AST_DIM:
        value = generateMCDim((AstIndex*)ast, data);
        break;
    case AST_INDEX:
        value = generateMCIndex((AstIndex*)ast, data);
        break;
    case AST_RETURN:
        value = generateMCReturn((Ast*)ast, data);
        break;
    case AST_FLOAT:
        value = generateMCFloat((AstFloat*)ast, data);
        break;
    case AST_INTEGER:
        value = generateMCInteger((AstInt*)ast, data);
        break;
    case AST_INT:
        value = generateMCToIntConv((AstUnary*)ast, data);
        break;
    case AST_LINENUM:
        value = generateMCLineNum((AstLineNum*)ast, data);
        break;
    case AST_EQ:
    case AST_NE:
    case AST_LT:
    case AST_LE:
    case AST_GT:
    case AST_GE:
        value = generateMCCondition((AstBinary*)ast, data);
        break;
    case AST_TRUE:
    case AST_FALSE:
        value = generateMCBoolean(ast, data);
        break;
    case AST_DEF:
        value = generateMCDef((AstDef*)ast, data);
        break;
    case AST_EXT:
        value = generateMCExt((AstExt*)ast, data);
        break;
    case AST_FN:
        value = generateMCFn((AstFn*)ast, data);
        break;
    default:
        value = generateMCForFunctions(ast, data);
        break;
    }
    return value;
}

Error generateMC(Ast* ast, MCGenerationData* data) {
    if (ast != NULL) {
        data->registers = 0; // There can't be any inter-line register dependency
        Value value = generateMCForAst(ast, data);
        if (value.type == VALUE_ERROR) {
            return value.error;
        } else {
            return ERROR_NONE;
        }
    } else {
        return ERROR_SYNTAX;
    }
}
