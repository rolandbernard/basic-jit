
#include <string.h>

#include "codegen/codegen.h"
#include "codegen/codegenfunc.h"

static uint64_t data_index = 0;

static int int64ToString(char* str, int64_t v) {
    int len = 0;
    while (v > 0) {
        str[len] = '0' + (v % 10);
        v /= 10;
        len++;
    }
    for(int i = 0; i < len / 2; i++) {
        char tmp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = tmp;
    }
    str[len] = 0;
    return len;
}

static Value generateMCGo(AstUnary* ast, MCGenerationData* data) {
    size_t pos;
    if(ast->type == AST_GOSUB) {
        pos = addInstCallRel(data->inst_mem, data->registers, ~0);
    } else {
        pos = addInstJmpRel(data->inst_mem, data->registers, ~0);
    }
    if(ast->value->type == AST_INTEGER) {
        char name[25]; 
        AstInt* line = (AstInt*)ast->value;
        int len = int64ToString(name, line->value);
        char* sym = (char*)alloc_aligned(data->variable_mem, len + 1);
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
        char* sym = (char*)alloc_aligned(data->variable_mem, len + 1);
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

static Value generateMCNext(AstUnary* ast, MCGenerationData* data) {

}

static Value generateMCRestore(AstUnary* ast, MCGenerationData* data) {
    
}

typedef Value (*GenerateMCFunction)(Ast*, MCGenerationData*);

static Value withFreeRegister(Ast* ast, MCGenerationData* data, GenerateMCFunction func, int num_regs, int num_fregs) {
    int free_regs = countFreeRegister(data->registers);
    int free_fregs = countFreeFRegister(data->registers);
    uint64_t to_pop[num_regs + num_fregs];
    int to_pop_count = 0;
    while(free_regs < num_regs) {
        uint64_t reg = getUsedRegister(data->registers);
        addInstPush(data->inst_mem, data->registers, reg);
        data->registers &= ~reg;
        to_pop[to_pop_count] = reg;
        to_pop_count++;
    }
    while(free_fregs < num_fregs) {
        uint64_t freg = getUsedFRegister(data->registers);
        addInstPush(data->inst_mem, data->registers, freg);
        data->registers &= ~freg;
        to_pop[to_pop_count] = freg;
        to_pop_count++;
    }
    func(ast, data);
    while(to_pop_count > 0) {
        to_pop_count--;
        uint64_t reg = to_pop[to_pop_count];
        addInstPop(data->inst_mem, data->registers, reg);
        data->registers |= reg;
    }
}

static Value generateMCBinarayOperationAfterFreeReg(AstBinary* ast, MCGenerationData* data) {
    Value a = generateMCForAst(ast->first, data);
    if(a.type == VALUE_ERROR) {
        return a;
    } else if(a.type == VALUE_NONE) {
        Value ret = {.type = VALUE_ERROR, .error = ERROR_SYNTAX};
        return ret;
    } else {
        Value b = generateMCForAst(ast->second, data);
        if(b.type == VALUE_ERROR) {
            return b;
        } else if(b.type == VALUE_NONE) {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_SYNTAX};
            return ret;
        } else {
            if(a.type != b.type) {
                if(a.type == VALUE_INT && b.type == VALUE_FLOAT) {
                    Register freg = getFreeFRegister(data->registers);
                    data->registers |= freg;
                    addInstMovRegToFReg(data->inst_mem, data->registers, freg, a.reg);
                    switch (ast->type) {
                    case AST_ADD:
                        addInstFAdd(data->inst_mem, data->registers, b.reg, b.reg, freg);
                        break;
                    case AST_SUB:
                        addInstFSub(data->inst_mem, data->registers, b.reg, b.reg, freg);
                        break;
                    case AST_MUL:
                        addInstFMul(data->inst_mem, data->registers, b.reg, b.reg, freg);
                        break;
                    case AST_DIV:
                        addInstFDiv(data->inst_mem, data->registers, b.reg, b.reg, freg);
                        break;
                    case AST_MOD: {
                        Value ret = { .type = VALUE_ERROR, .error = ERROR_TYPE };
                        return ret;
                    }
                    default:
                        break;
                    }
                    data->registers &= ~freg;
                    data->registers &= ~a.reg;
                    return b;
                } else if(a.type == VALUE_FLOAT && b.type == VALUE_INT) {
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
                    case AST_MOD: {
                        Value ret = { .type = VALUE_ERROR, .error = ERROR_TYPE };
                        return ret;
                    }
                    default:
                        break;
                    }
                    data->registers &= ~freg;
                    data->registers &= ~(b.reg);
                    return a;
                } else {
                    Value ret = { .type = VALUE_ERROR, .error = ERROR_TYPE };
                    return ret;
                }
            } else {
                if(a.type == VALUE_INT) {
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
                    case AST_MOD:
                        addInstRem(data->inst_mem, data->registers, a.reg, a.reg, b.reg);
                        break;
                    default:
                        break;
                    }
                } else if(a.type == VALUE_FLOAT) {
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
                    case AST_MOD: {
                        Value ret = { .type = VALUE_ERROR, .error = ERROR_TYPE };
                        return ret;
                    }
                    default:
                        break;
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

static Value generateMCBinarayOperation(AstBinary* ast, MCGenerationData* data) {
    withFreeRegister((Ast*)ast, data, (GenerateMCFunction)generateMCBinarayOperationAfterFreeReg, 2, 2);
}

static Value generateMCNegAfterFreeReg(AstUnary* ast, MCGenerationData* data) {
    Value a = generateMCForAst(ast->value, data);
    if(a.type == VALUE_ERROR) {
        return a;
    } else {
        if (a.type == VALUE_INT) {
            Register reg = getFreeRegister(data->registers);
            data->registers |= reg;
            addInstMovImmToReg(data->inst_mem, data->registers, reg, 0, false);
            addInstSub(data->inst_mem, data->registers, reg, reg, a.reg);
            data->registers &= ~a.reg;
            a.reg = reg;
        } else if (a.type == VALUE_FLOAT) {
            Register freg = getFreeFRegister(data->registers);
            data->registers |= freg;
            addInstMovImmToFReg(data->inst_mem, data->registers, freg, 0);
            addInstSub(data->inst_mem, data->registers, freg, freg, a.reg);
            data->registers &= ~a.reg;
            a.reg = freg;
        } else {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
            return ret;
        }
        return a;
    }
}

static Value generateMCNeg(AstUnary* ast, MCGenerationData* data) {
    withFreeRegister((Ast*)ast, data, (GenerateMCFunction)generateMCNegAfterFreeReg, 2, 2);
}

static Value generateMCString(AstString* ast, MCGenerationData* data) {
    int len = strlen(ast->str);
    char* value = (char*)alloc_aligned(data->variable_mem, len + 1);
    memcpy(value, ast->str, len + 1);
    Register reg = getFreeRegister(data->registers);
    data->registers |= reg;
    Value ret = {
        .type = VALUE_STRING,
        .reg = reg,
    };
    return ret;
}

static Value generateMCLabel(AstString* ast, MCGenerationData* data) {
    VariableLabel* var = (VariableLabel*)alloc_aligned(data->variable_mem, sizeof(VariableLabel));
    var->type = VARIABLE_LABEL;
    var->pos = data->inst_mem->occupied;
    var->data_pos = data->data_mem->count;
    addVariable(data->label_table, ast->str, (Variable*)var, data->variable_mem);
}

static Value generateMCData(AstVariable* ast, MCGenerationData* data) {
    for(int i = 0; i < ast->count; i++) {
        DataElement data_element = { .integer = 0, .real = 0.0, .string = NULL };
        if(ast->values[i]->type == AST_INTEGER) {
            AstInt* value = (AstInt*)ast->values[i];
            data_element.integer = value->value;
            data_element.real = value->value;
        } else if(ast->values[i]->type == AST_FLOAT) {
            AstFloat* value = (AstFloat*)ast->values[i];
            data_element.integer = value->value;
            data_element.real = value->value;
        } else {
            AstString* value = (AstString*)ast->values[i];
            int len = strlen(value->str);
            char* v = (char*)alloc_aligned(data->variable_mem, len + 1);
            memcpy(v, value->str, len + 1);
            data_element.string = v;
        }
        addDataToList(data->data_mem, data_element);
    }
}

static Value generateMCReadOfArrayAccessAfterFreeReg(AstIndex* ast, MCGenerationData* data) {
    AstVar* var = (AstVar*)ast->name;
    Variable* variable = getVariable(data->variable_table, var->name);
    if(variable != NULL) {
        if(variable->type != VARIABLE_STRING_ARRAY && variable->type != VARIABLE_INT_ARRAY && variable->type != VARIABLE_FLOAT_ARRAY) {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
            return ret;
        } else if (var->var_type != VAR_UNDEF) {
            if ((variable->type == VARIABLE_INT_ARRAY && var->var_type != VAR_INT) || 
                (variable->type == VARIABLE_FLOAT_ARRAY && var->var_type != VAR_FLOAT) ||
                (variable->type == VARIABLE_STRING_ARRAY && var->var_type != VAR_STR)) {
                Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
                return ret;
            }
        } else {
             if ((variable->type == VARIABLE_INT_ARRAY && ((VariableIntArray*)variable)->dim_count != ast->count) || 
                (variable->type == VARIABLE_FLOAT_ARRAY && ((VariableIntArray*)variable)->dim_count != ast->count) ||
                (variable->type == VARIABLE_STRING_ARRAY && ((VariableIntArray*)variable)->dim_count != ast->count)) {
                Value ret = {.type = VALUE_ERROR, .error = ERROR_ARRAY_DIM_COUNT};
                return ret;
            }
        }
        Register imm_reg = getFreeRegister(data->registers);
        data->registers |= imm_reg;
        Register index_reg = getFreeRegister(data->registers);
        data->registers |= index_reg;
        Register data_reg = getFreeRegister(data->registers);
        data->registers |= data_reg;
        addInstMovMemToReg(data->inst_mem, data->registers, index_reg, (void*)&data_index);
        addInstMovMemToReg(data->inst_mem, data->registers, data_reg, (void*)data->data_mem->data);
        addInstMovImmToReg(data->inst_mem, data->registers, imm_reg, sizeof(DataElement), false);
        addInstMul(data->inst_mem, data->registers, imm_reg, imm_reg, index_reg);
        addInstAdd(data->inst_mem, data->registers, data_reg, data_reg, imm_reg);
        addInstMovImmToReg(data->inst_mem, data->registers, imm_reg, 1, false);
        addInstAdd(data->inst_mem, data->registers, index_reg, index_reg, imm_reg);
        addInstMovRegToMem(data->inst_mem, data->registers, index_reg, (void*)&data_index);
        if(variable->type == VARIABLE_INT_ARRAY) {
            addInstMovImmToReg(data->inst_mem, data->registers, imm_reg, offsetof(DataElement, integer), false);
        } else if(variable->type == VARIABLE_FLOAT_ARRAY) {
            addInstMovImmToReg(data->inst_mem, data->registers, imm_reg, offsetof(DataElement, real), false);
        } else if(variable->type == VARIABLE_STRING_ARRAY) {
            addInstMovImmToReg(data->inst_mem, data->registers, imm_reg, offsetof(DataElement, string), false);
        }
        addInstAdd(data->inst_mem, data->registers, data_reg, data_reg, imm_reg);
        addInstMovMemRegToReg(data->inst_mem, data->registers, data_reg, data_reg);
        size_t indexing_size;
        if(variable->type == VARIABLE_INT_ARRAY) {
            addInstMovImmToReg(data->inst_mem, data->registers, index_reg, (intptr_t)((VariableIntArray*)variable)->value, false);
            indexing_size = sizeof(int64_t);
        } else if(variable->type == VARIABLE_FLOAT_ARRAY) {
            addInstMovImmToReg(data->inst_mem, data->registers, index_reg, (intptr_t)((VariableFloatArray*)variable)->value, false);
            indexing_size = sizeof(double);
        } else if(variable->type == VARIABLE_STRING_ARRAY) {
            addInstMovImmToReg(data->inst_mem, data->registers, index_reg, (intptr_t)((VariableStringArray*)variable)->str, false);
            indexing_size = sizeof(char*);
        }
        for(int i = 0; i < ast->count; i++) {
            Value index = generateMCForAst(ast->size[i], data);
            if(index.type == VALUE_ERROR) {
                return index;
            } else if(index.type == VALUE_NONE) {
                Value ret = {.type = VALUE_ERROR, .error = ERROR_SYNTAX};
                return ret;
            } else if(index.type != VALUE_INT) {
                Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
                return ret;
            } else {
                addInstMovImmToReg(data->inst_mem, data->registers, imm_reg, indexing_size, false);
                addInstMul(data->inst_mem, data->registers, imm_reg, imm_reg, index.reg);
                data->registers &= ~index.reg;
                addInstAdd(data->inst_mem, data->registers, index_reg, index_reg, imm_reg);
            }
            if(variable->type == VARIABLE_INT_ARRAY) {
                indexing_size *= ((VariableIntArray*)variable)->size[i];
            } else if(variable->type == VARIABLE_FLOAT_ARRAY) {
                indexing_size *= ((VariableFloatArray*)variable)->size[i];
            } else if(variable->type == VARIABLE_STRING_ARRAY) {
                indexing_size *= ((VariableStringArray*)variable)->size[i];
            }
        }
        data->registers &= ~imm_reg;
        if(variable->type == VARIABLE_INT_ARRAY) {
            addInstMovRegToMemReg(data->inst_mem, data->registers, index_reg, data_reg);
        } else if(variable->type == VARIABLE_FLOAT_ARRAY) {
            addInstMovRegToMemReg(data->inst_mem, data->registers, index_reg, data_reg);
        } else if(variable->type == VARIABLE_STRING_ARRAY) {
            addInstMovRegToMemReg(data->inst_mem, data->registers, index_reg, data_reg);
        }
        data->registers &= ~index_reg;
        data->registers &= ~data_reg;
    } else {
        Value ret = {.type = VALUE_ERROR, .error = ERROR_ARRAY_NOT_DEF};
        return ret;
    }
}

static Value generateMCReadAfterFreeReg(AstVariable* ast, MCGenerationData* data) {
    for(int i = 0; i < ast->count; i++) {
        if(ast->values[i]->type == AST_VAR) {
            AstVar* var = (AstVar*)ast->values[i];
            Variable* variable = getVariable(data->variable_table, var->name);
            if(variable == NULL) {
                if(var->var_type == VAR_UNDEF && var->var_type == VAR_FLOAT) {
                    VariableFloat* varib = (VariableFloat*)alloc_aligned(data->variable_mem, sizeof(VariableFloat));
                    varib->type = VARIABLE_FLOAT;
                    varib->for_jmp_loc = data->inst_mem->occupied;
                    varib->value = 0.0;
                    addVariable(data->variable_table, var->name, (Variable*)varib, data->variable_mem);
                    variable = (Variable*)varib;
                } else if(var->var_type == VAR_INT) {
                    VariableInt* varib = (VariableInt*)alloc_aligned(data->variable_mem, sizeof(VariableInt));
                    varib->type = VARIABLE_INT;
                    varib->for_jmp_loc = data->inst_mem->occupied;
                    varib->value = 0;
                    addVariable(data->variable_table, var->name, (Variable*)varib, data->variable_mem);
                    variable = (Variable*)varib;
                } else if(var->var_type == VAR_STR) {
                    VariableString* varib = (VariableString*)alloc_aligned(data->variable_mem, sizeof(VariableString));
                    varib->type = VARIABLE_STRING;
                    varib->str = NULL;
                    addVariable(data->variable_table, var->name, (Variable*)varib, data->variable_mem);
                    variable = (Variable*)varib;
                }
            } else if(var->var_type != VAR_UNDEF) {
                if((variable->type == VARIABLE_INT && var->var_type != VAR_INT) ||
                   (variable->type == VARIABLE_FLOAT && var->var_type != VAR_FLOAT) ||
                   (variable->type == VARIABLE_STRING && var->var_type != VAR_STR)) {
                    Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
                    return ret;
                }
            }
            Register imm_reg = getFreeRegister(data->registers);
            data->registers |= imm_reg;
            Register index_reg = getFreeRegister(data->registers);
            data->registers |= index_reg;
            Register data_reg = getFreeRegister(data->registers);
            data->registers |= data_reg;
            addInstMovMemToReg(data->inst_mem, data->registers, index_reg, (void*)&data_index);
            addInstMovMemToReg(data->inst_mem, data->registers, data_reg, (void*)data->data_mem->data);
            addInstMovImmToReg(data->inst_mem, data->registers, imm_reg, sizeof(DataElement), false);
            addInstMul(data->inst_mem, data->registers, imm_reg, imm_reg, index_reg);
            addInstAdd(data->inst_mem, data->registers, data_reg, data_reg, imm_reg);
            addInstMovImmToReg(data->inst_mem, data->registers, imm_reg, 1, false);
            addInstAdd(data->inst_mem, data->registers, index_reg, index_reg, imm_reg);
            addInstMovRegToMem(data->inst_mem, data->registers, index_reg, (void*)&data_index);
            data->registers &= ~index_reg;
            if(variable->type == VARIABLE_INT) {
                addInstMovImmToReg(data->inst_mem, data->registers, imm_reg, offsetof(DataElement, integer), false);
            } else if(variable->type == VARIABLE_FLOAT) {
                addInstMovImmToReg(data->inst_mem, data->registers, imm_reg, offsetof(DataElement, real), false);
            } else if(variable->type == VARIABLE_STRING) {
                addInstMovImmToReg(data->inst_mem, data->registers, imm_reg, offsetof(DataElement, string), false);
            }
            addInstAdd(data->inst_mem, data->registers, data_reg, data_reg, imm_reg);
            data->registers &= ~imm_reg;
            addInstMovMemRegToReg(data->inst_mem, data->registers, data_reg, data_reg);
            if(variable->type == VARIABLE_INT) {
                addInstMovRegToMem(data->inst_mem, data->registers, data_reg, &((VariableInt*)variable)->value);
            } else if(variable->type == VARIABLE_FLOAT) {
                addInstMovRegToMem(data->inst_mem, data->registers, data_reg, &((VariableFloat*)variable)->value);
            } else if(variable->type == VARIABLE_STRING) {
                addInstMovRegToMem(data->inst_mem, data->registers, data_reg, &((VariableString*)variable)->str);
            }
            data->registers &= ~data_reg;
        } else if(ast->values[i]->type == AST_INDEX) { 
            AstIndex* index = (AstIndex*)ast->values[i]->type;
            Value ret = withFreeRegister((Ast*)index, data, (GenerateMCFunction)generateMCReadOfArrayAccessAfterFreeReg, 4, 0);
            if(ret.type == VALUE_ERROR) {
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
    for(int i = 0; i < ast->count; i++) {
        data->registers = 0;
        Value ret = generateMCForAst(ast->values[i], data);
        if(ret.type == VALUE_ERROR) {
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
    if(variable != NULL) {
        if(variable->type != VARIABLE_STRING_ARRAY && variable->type != VARIABLE_INT_ARRAY && variable->type != VARIABLE_FLOAT_ARRAY) {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
            return ret;
        } else if (var->var_type != VAR_UNDEF) {
            if ((variable->type == VARIABLE_INT_ARRAY && var->var_type != VAR_INT) || 
                (variable->type == VARIABLE_FLOAT_ARRAY && var->var_type != VAR_FLOAT) ||
                (variable->type == VARIABLE_STRING_ARRAY && var->var_type != VAR_STR)) {
                Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
                return ret;
            }
        } else {
             if ((variable->type == VARIABLE_INT_ARRAY && ((VariableIntArray*)variable)->dim_count != index->count) || 
                (variable->type == VARIABLE_FLOAT_ARRAY && ((VariableIntArray*)variable)->dim_count != index->count) ||
                (variable->type == VARIABLE_STRING_ARRAY && ((VariableIntArray*)variable)->dim_count != index->count)) {
                Value ret = {.type = VALUE_ERROR, .error = ERROR_ARRAY_DIM_COUNT};
                return ret;
            }
        }
        Register imm_reg = getFreeRegister(data->registers);
        data->registers |= imm_reg;
        Register index_reg = getFreeRegister(data->registers);
        data->registers |= index_reg;
        size_t indexing_size;
        if(variable->type == VARIABLE_INT_ARRAY) {
            addInstMovImmToReg(data->inst_mem, data->registers, index_reg, (intptr_t)((VariableIntArray*)variable)->value, false);
            indexing_size = sizeof(int64_t);
        } else if(variable->type == VARIABLE_FLOAT_ARRAY) {
            addInstMovImmToReg(data->inst_mem, data->registers, index_reg, (intptr_t)((VariableFloatArray*)variable)->value, false);
            indexing_size = sizeof(double);
        } else if(variable->type == VARIABLE_STRING_ARRAY) {
            addInstMovImmToReg(data->inst_mem, data->registers, index_reg, (intptr_t)((VariableStringArray*)variable)->str, false);
            indexing_size = sizeof(char*);
        }
        for(int i = 0; i < index->count; i++) {
            Value ind = generateMCForAst(index->size[i], data);
            if(ind.type == VALUE_ERROR) {
                return ind;
            } else if(ind.type == VALUE_NONE) {
                Value ret = {.type = VALUE_ERROR, .error = ERROR_SYNTAX};
                return ret;
            } else if(ind.type != VALUE_INT) {
                Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
                return ret;
            } else {
                addInstMovImmToReg(data->inst_mem, data->registers, imm_reg, indexing_size, false);
                addInstMul(data->inst_mem, data->registers, imm_reg, imm_reg, ind.reg);
                data->registers &= ~ind.reg;
                addInstAdd(data->inst_mem, data->registers, index_reg, index_reg, imm_reg);
            }
            if(variable->type == VARIABLE_INT_ARRAY) {
                indexing_size *= ((VariableIntArray*)variable)->size[i];
            } else if(variable->type == VARIABLE_FLOAT_ARRAY) {
                indexing_size *= ((VariableFloatArray*)variable)->size[i];
            } else if(variable->type == VARIABLE_STRING_ARRAY) {
                indexing_size *= ((VariableStringArray*)variable)->size[i];
            }
        }
        data->registers &= ~imm_reg;
        Value a = generateMCForAst(ast->value, data);
        if(a.type == VALUE_ERROR) {
            return a;
        } else if(a.type == VALUE_NONE) {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_SYNTAX};
            return ret;
        } else if((variable->type == VARIABLE_INT && a.type == VALUE_INT) || (variable->type == VARIABLE_STRING && a.type == VALUE_STRING)) {
            addInstMovRegToMemReg(data->inst_mem, data->registers, a.reg, index_reg);
        } else if(variable->type == VARIABLE_FLOAT && a.type == VALUE_FLOAT) {
            addInstMovFRegToMemReg(data->inst_mem, data->registers, a.reg, index_reg);
        } else if(variable->type == VARIABLE_FLOAT && a.type == VALUE_INT) {
            Register freg = getFreeFRegister(data->registers);
            data->registers |= freg;
            addInstMovRegToFReg(data->inst_mem, data->registers, freg, a.reg);
            addInstMovFRegToMemReg(data->inst_mem, data->registers, freg, index_reg);
            data->registers &= ~freg;
        } else {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
            return ret;
        }
        data->registers &= ~a.reg;
        data->registers &= ~index_reg;
    } else {
        Value ret = {.type = VALUE_ERROR, .error = ERROR_ARRAY_NOT_DEF};
        return ret;
    }
}

static Value generateMCLetAfterFreeReg(AstLet* ast, MCGenerationData* data) {
    if(ast->name->type == AST_VAR) {
        AstVar* var = (AstVar*)ast->name;
        Variable* variable = getVariable(data->variable_table, var->name);
        if(variable == NULL) {
            if (var->var_type == VAR_UNDEF && var->var_type == VAR_FLOAT) {
                VariableFloat* varib = (VariableFloat*)alloc_aligned(data->variable_mem, sizeof(VariableFloat));
                varib->type = VARIABLE_FLOAT;
                varib->for_jmp_loc = data->inst_mem->occupied;
                varib->value = 0.0;
                addVariable(data->variable_table, var->name, (Variable*)varib, data->variable_mem);
                variable = (Variable*)varib;
            } else if (var->var_type == VAR_INT) {
                VariableInt* varib = (VariableInt*)alloc_aligned(data->variable_mem, sizeof(VariableInt));
                varib->type = VARIABLE_INT;
                varib->for_jmp_loc = data->inst_mem->occupied;
                varib->value = 0;
                addVariable(data->variable_table, var->name, (Variable*)varib, data->variable_mem);
                variable = (Variable*)varib;
            } else if (var->var_type == VAR_STR) {
                VariableString* varib = (VariableString*)alloc_aligned(data->variable_mem, sizeof(VariableString));
                varib->type = VARIABLE_STRING;
                varib->str = NULL;
                addVariable(data->variable_table, var->name, (Variable*)varib, data->variable_mem);
                variable = (Variable*)varib;
            }
        } else if(var->var_type != VAR_UNDEF) {
            if((variable->type == VARIABLE_INT && var->var_type != VAR_INT) ||
               (variable->type == VARIABLE_FLOAT && var->var_type != VAR_FLOAT) ||
               (variable->type == VARIABLE_STRING && var->var_type != VAR_STR)) {
                Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
                return ret;
            }
        }
        Value a = generateMCForAst(ast->value, data);
        if(a.type == VALUE_ERROR) {
            return a;
        } else if(a.type == VALUE_NONE) {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_SYNTAX};
            return ret;
        } else if(variable->type == VARIABLE_INT && a.type == VALUE_INT) {
            addInstMovRegToMem(data->inst_mem, data->registers, a.reg, &((VariableInt*)variable)->value);
        } else if(variable->type == VARIABLE_FLOAT && a.type == VALUE_FLOAT) {
            addInstMovFRegToMem(data->inst_mem, data->registers, a.reg, &((VariableFloat*)variable)->value);
        } else if(variable->type == VARIABLE_STRING && a.type == VALUE_STRING) {
            addInstMovRegToMem(data->inst_mem, data->registers, a.reg, &((VariableString*)variable)->str);
        } else if(variable->type == VARIABLE_FLOAT && a.type == VALUE_INT) {
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
    } else if(ast->name->type == AST_INDEX) { 
        AstIndex* index = (AstIndex*)ast->name->type;
        Value ret = withFreeRegister((Ast*)index, data, (GenerateMCFunction)generateMCLetOfArrayAccessAfterFreeReg, 3, 1);
        if(ret.type == VALUE_ERROR) {
            return ret;
        }
    } else {
        Value ret = {.type = VALUE_ERROR, .error = ERROR_SYNTAX};
        return ret;
    }
}

static Value generateMCLet(AstLet* ast, MCGenerationData* data) {
    return withFreeRegister((Ast*)ast, data, (GenerateMCFunction)generateMCLetAfterFreeReg, 1, 1);
}

static Value generateMCIfThenElse(AstIfThenElse* ast, MCGenerationData* data) {
    
}

static Value generateMCFor(AstFor* ast, MCGenerationData* data) {
    
}

static Value generateMCVar(AstVar* ast, MCGenerationData* data) {
    
}

static Value generateMCOnGoto(AstSwitch* ast, MCGenerationData* data) {
    
}

static Value generateMCOnGoSub(AstSwitch* ast, MCGenerationData* data) {
    
}

static Value generateMCDim(AstIndex* ast, MCGenerationData* data) {
    
}

static Value generateMCIndex(AstIndex* ast, MCGenerationData* data) {
    
}

static Value generateMCReturn(Ast* ast, MCGenerationData* data) {
    
}

static Value generateMCInteger(AstInt* ast, MCGenerationData* data) {
    
}

static Value generateMCFloat(AstFloat* ast, MCGenerationData* data) {
    
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
        value = generateMCBinarayOperation((AstBinary*)ast, data);
        break;
    case AST_NEG:
        value = generateMCNeg((AstUnary*)ast, data);
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
    case AST_FOR:
        value = generateMCFor((AstFor*)ast, data);
        break;
    case AST_VAR:
        value = generateMCVar((AstVar*)ast, data);
        break;
    case AST_ON_GOTO:
        value = generateMCOnGoto((AstSwitch*)ast, data);
        break;
    case AST_ON_GOSUB:
        value = generateMCOnGoSub((AstSwitch*)ast, data);
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
    default:
        value = generateMCForFunctions(ast, data);
        break;
    }
    return value;
}

Error generateMC(Ast* ast, MCGenerationData* data) {
    if(ast != NULL) {
        data->registers = 0;
        Value value = generateMCForAst(ast, data);
        if(value.type == VALUE_ERROR) {
            return value.error;
        } else {
            return ERROR_NONE;
        }
    }
    return ERROR_NONE;
}
