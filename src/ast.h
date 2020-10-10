#ifndef _AST_H_
#define _AST_H_

#include <stdint.h>

#define AST_BASE AstType type;

typedef enum {
    // simple
    AST_END,
    AST_STOP,
    AST_KEY,
    AST_RETURN,
    AST_RAN,
    AST_BEEP,
    
    // unary 
    AST_TAB,
    AST_SPC,
    AST_SIN,
    AST_COS,
    AST_TAN,
    AST_ASN,
    AST_ACS,
    AST_ATN,
    AST_LOG,
    AST_LN,
    AST_EXP,
    AST_SQR,
    AST_ABS,
    AST_SGN,
    AST_INT,
    AST_FRAC,
    AST_RND,
    AST_DEG,
    AST_RAD,
    AST_VAL,
    AST_STR,
    AST_GOTO,
    AST_GOSUB,
    AST_NEXT,
    AST_RESTORE,

    // binary
    AST_ADD,
    AST_SUB,
    AST_MUL,
    AST_DIV,
    AST_MOD,
    AST_POW,
    AST_NEG,
    AST_COND,
    AST_MID,
    
    // string
    AST_STRING,
    AST_LABEL,

    // variable
    AST_INPUT,
    AST_PRINT,
    AST_DATA,
    AST_READ,
    AST_MULTIPLE,

    // special
    AST_FLOAT,
    AST_INT,
    AST_LET,
    AST_IF_THEN_ELSE,
    AST_FOR,
    AST_SET,
    AST_VAR,
    
    // switch
    AST_ON_GOTO,
    AST_ON_GOSUB,

    // index
    AST_DIM,
    AST_INDEX,
} AstType;

typedef struct {
    AST_BASE
} Ast;

typedef struct {
    AST_BASE
    Ast* value;
} AstUnary;

typedef struct {
    AST_BASE
    Ast* first;
    Ast* second;
} AstBinary;

typedef struct {
    AST_BASE
    char* str;
} AstString;

typedef struct {
    AST_BASE
    int count;
    Ast** values;
} AstVariable;

typedef struct {
    AST_BASE
    double value;
} AstFloat;

typedef struct {
    AST_BASE
    int64_t value;
} AstInt;

typedef struct {
    AST_BASE
    char* name;
    Ast* value;
} AstLet;

typedef struct {
    AST_BASE
    Ast* condition;
    Ast* if_true;
    Ast* if_false;
} AstIfThenElse;

typedef struct {
    AST_BASE
    char* variable;
    Ast* start;
    Ast* end;
    Ast* step;
} AstFor;

typedef struct {
    AST_BASE
    bool significant;
    int length;
} AstSet;

typedef struct {
    AST_BASE
    enum {
        VAR_UNDEF,
        VAR_FLOAT,
        VAR_INT,
        VAR_STR
    } var_type;
    char* name;
} AstVar;

typedef struct {
    AST_BASE
    Ast* value;
    int count;
    Ast** locations;
} AstSwitch;

typedef struct {
    AST_BASE
    char* name;
    int count;
    int* size;
} AstIndex;

#endif