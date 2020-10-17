
#include "codegenfunc.h"




Value generateMCForFunctions(Ast* ast, MCGenerationData* data) {
    Value value = { .type = VALUE_NONE };
    if(ast != NULL) {
        switch (ast->type) {
        case AST_POW:
        case AST_TAB:
        case AST_SPC:
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
        case AST_ABS:
        case AST_SGN:
        case AST_INT:
        case AST_FRAC:
        case AST_RND:
        case AST_DEG:
        case AST_RAD:
        case AST_VAL:
        case AST_STR:
        case AST_INPUT:
        case AST_PRINT:
        case AST_RAN:
        case AST_BEEP:
        case AST_KEY:
        case AST_END:
        case AST_STOP:
        default:
            value.type = VALUE_ERROR;
            value.error = ERROR_SYNTAX;
            break;
        }
    }
    return value;
}
