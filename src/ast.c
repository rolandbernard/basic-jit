
#include <stdlib.h>

#include "ast.h"

void freeAstUnary(AstUnary* ast) {
    freeAst(ast->value);
}

void freeAstBinary(AstBinary* ast) {
    freeAst(ast->first);
    freeAst(ast->second);
}

void freeAstString(AstString* ast) {
    free(ast->str);
}

void freeAstVariable(AstVariable* ast) {
    for(int i = 0; i < ast->count; i++) {
        freeAst(ast->values[i]);
    }
    free(ast->values);
}

void freeAstLet(AstLet* ast) {
    free(ast->name);
    freeAst(ast->value);
}

void freeAstIfThenElse(AstIfThenElse* ast) {
    freeAst(ast->condition);
    freeAst(ast->if_true);
    freeAst(ast->if_false);
}

void freeAstFor(AstFor* ast) {
    freeAst((Ast*)ast->variable);
    freeAst(ast->start);
    freeAst(ast->end);
    freeAst(ast->step);
}

void freeAstVar(AstVar* ast) {
    free(ast->name);
}

void freeAstSwitch(AstSwitch* ast) {
    freeAst(ast->value);
    for(int i = 0; i < ast->count; i++) {
        freeAst(ast->locations[i]);
    }
    free(ast->locations);
}

void freeAstIndex(AstIndex* ast) {
    freeAst((Ast*)ast->name);
    for(int i = 0; i < ast->count; i++) {
        freeAst(ast->size[i]);
    }
    free(ast->size);
}

void freeAst(Ast* ast) {
    if(ast != NULL) {
        switch (ast->type) {
        case AST_END:
        case AST_STOP:
        case AST_KEY:
        case AST_RETURN:
        case AST_RAN:
        case AST_BEEP:
        case AST_FLOAT:
        case AST_INTEGER:
        case AST_SET:
            // Nothing to free here
            break;
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
        case AST_GOTO:
        case AST_GOSUB:
        case AST_NEXT:
        case AST_RESTORE:
            freeAstUnary((AstUnary*)ast);
            break;
        case AST_ADD:
        case AST_SUB:
        case AST_MUL:
        case AST_DIV:
        case AST_MOD:
        case AST_POW:
        case AST_NEG:
        case AST_COND:
        case AST_MID:
            freeAstBinary((AstBinary*)ast);
            break;
        case AST_STRING:
        case AST_LABEL:
            freeAstString((AstString*)ast);
            break;
        case AST_INPUT:
        case AST_PRINT:
        case AST_DATA:
        case AST_READ:
        case AST_MULTIPLE:
            freeAstVariable((AstVariable*)ast);
            break;
        case AST_LET:
            freeAstLet((AstLet*)ast);
            break;
        case AST_IF_THEN_ELSE:
            freeAstIfThenElse((AstIfThenElse*)ast);
            break;
        case AST_FOR:
            freeAstFor((AstFor*)ast);
            break;
        case AST_VAR:
            freeAstVar((AstVar*)ast);
            break;
        case AST_ON_GOTO:
        case AST_ON_GOSUB:
            freeAstSwitch((AstSwitch*)ast);
            break;
        case AST_DIM:
        case AST_INDEX:
            freeAstIndex((AstIndex*)ast);
            break;
        }
        free(ast);
    }
}
