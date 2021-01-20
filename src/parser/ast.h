#ifndef _AST_H_
#define _AST_H_

#include <stdint.h>
#include <stdbool.h>

#define AST_BASE AstType type;

typedef enum {
    AST_NONE,
    AST_ERROR,
    
    // simple
    AST_STOP,
    AST_KEY,
    AST_RETURN,
    AST_RAN,
    AST_BEEP,
    AST_RUN,
    AST_NEW,
    AST_TRUE,
    AST_FALSE,
    
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
    AST_LIST,
    AST_SLEEP,
    AST_END,
    AST_ASSERT,
    AST_NOT,

    // binary
    AST_ADD,
    AST_SUB,
    AST_MUL,
    AST_DIV,
    AST_MOD,
    AST_POW,
    AST_NEG,
    AST_GT,
    AST_LT,
    AST_EQ,
    AST_LE,
    AST_GE,
    AST_NE,
    AST_LEFT,
    AST_RIGHT,
    AST_AND,
    AST_OR,
    AST_XOR,
    
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
    AST_INTEGER,
    AST_LET,
    AST_IF_THEN_ELSE,
    AST_FOR,
    AST_VAR,
    AST_LINENUM,
    
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
    int64_t number;
    Ast* line;
} AstLineNum;

typedef struct {
    AST_BASE
    int offset;
} AstError;

typedef struct {
    AST_BASE
    enum {
        VAR_UNDEF,
        VAR_FLOAT,
        VAR_INT,
        VAR_STR,
        VAR_BOOL,
    } var_type;
    char* name;
} AstVar;

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
    bool open_end;
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
    Ast* name;
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
    AstVar* variable;
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
    Ast* value;
    int count;
    Ast** locations;
} AstSwitch;

typedef struct {
    AST_BASE
    AstVar* name;
    int count;
    Ast** size;
} AstIndex;

#endif