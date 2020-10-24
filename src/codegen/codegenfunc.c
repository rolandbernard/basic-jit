
#include <math.h>

#include "codegenfunc.h"

#define PI 3.14159265358979323

int64_t powInt64(int64_t b, int64_t e) {
    if(e < 0) {
        if(b == 1) {
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

Value generateMCPowCallAfterFreeReg(AstBinary* ast, MCGenerationData* data) {
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
                    data->registers &= ~a.reg;
                    a.type = VALUE_FLOAT;
                    a.reg = freg;
                } else if(a.type == VALUE_FLOAT && b.type == VALUE_INT) {
                    Register freg = getFreeFRegister(data->registers);
                    data->registers |= freg;
                    addInstMovRegToFReg(data->inst_mem, data->registers, freg, b.reg);
                    data->registers &= ~b.reg;
                    b.type = VALUE_FLOAT;
                    b.reg = freg;
                } else {
                    Value ret = { .type = VALUE_ERROR, .error = ERROR_TYPE };
                    return ret;
                }
            }
            if(a.type == VALUE_INT) {
                addInstFunctionCallBinary(data->inst_mem, data->registers, a.reg, a.reg, b.reg, powInt64);
            } else if(a.type == VALUE_FLOAT) {
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

Value generateMCPowCall(AstBinary* ast, MCGenerationData* data) {
    return withFreeRegister((Ast*)ast, data, (GenerateMCFunction)generateMCPowCallAfterFreeReg, 2, 2);
}

double frac(double x) {
    double integ;
    return modf(x, &integ);
}

double toDeg(double x) {
    return x * 180.0 / PI;
}

double toRad(double x) {
    return x * PI / 180.0;
}

double round(double x) {
    return floor(x + 0.5);
}

Value generateMCUnaryFloatCallAfterFreeReg(AstBinary* ast, MCGenerationData* data) {
    Value a = generateMCForAst(ast->first, data);
    if(a.type == VALUE_ERROR) {
        return a;
    } else if(a.type == VALUE_NONE) {
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
        } else {
            Value ret = {.type = VALUE_ERROR, .error = ERROR_TYPE};
            return ret;
        }
        switch(ast->type) {
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

Value generateMCUnaryFloatCall(AstBinary* ast, MCGenerationData* data) {
    return withFreeRegister((Ast*)ast, data, (GenerateMCFunction)generateMCUnaryFloatCallAfterFreeReg, 1, 1);
}

Value generateMCPrint(AstVariable* ast, MCGenerationData* data) {

}

Value generateMCInput(AstVariable* ast, MCGenerationData* data) {

}

Value generateMCSimpleCall(Ast* ast, MCGenerationData* data) {

}

Value generateMCForFunctions(Ast* ast, MCGenerationData* data) {
    Value value = { .type = VALUE_NONE };
    if(ast != NULL) {
        switch (ast->type) {
        case AST_POW:
            value = generateMCPowCall((AstBinary*)ast, data);
            break;
        case AST_TAB:
        case AST_SPC:
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
            value = generateMCUnaryFloatCall((AstBinary*)ast, data);
            break;
        case AST_SGN:
        case AST_ABS:
            break;
        case AST_VAL:
            break;
        case AST_STR:
            break;
        case AST_INPUT:
            break;
        case AST_PRINT:
            break;
        case AST_RAN:
            break;
        case AST_BEEP:
        case AST_END:
        case AST_STOP:
            break;
        case AST_KEY:
            break;
        default:
            value.type = VALUE_ERROR;
            value.error = ERROR_SYNTAX;
            break;
        }
    }
    return value;
}
