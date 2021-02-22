#ifndef _SCANNER_H_
#define _SCANNER_H_

#include <stdbool.h>

typedef enum {
    TOKEN_NONE,

    TOKEN_KEYWORDS_START,
    TOKEN_REM,
    TOKEN_END,
    TOKEN_STOP,
    TOKEN_KEY,
    TOKEN_RETURN,
    TOKEN_RAN,
    TOKEN_BEEP,
    TOKEN_TAB,
    TOKEN_SPC,
    TOKEN_SIN,
    TOKEN_COS,
    TOKEN_TAN,
    TOKEN_ASN,
    TOKEN_ACS,
    TOKEN_ATN,
    TOKEN_LOG,
    TOKEN_LN,
    TOKEN_EXP,
    TOKEN_SQR,
    TOKEN_ABS,
    TOKEN_SGN,
    TOKEN_INT,
    TOKEN_FRAC,
    TOKEN_RND,
    TOKEN_DEG,
    TOKEN_RAD,
    TOKEN_VAL,
    TOKEN_STR,
    TOKEN_GOTO,
    TOKEN_GOSUB,
    TOKEN_NEXT,
    TOKEN_RESTORE,
    TOKEN_LET,
    TOKEN_IF,
    TOKEN_THEN,
    TOKEN_ELSE,
    TOKEN_FOR,
    TOKEN_ON,
    TOKEN_MOD,
    TOKEN_INPUT,
    TOKEN_PRINT,
    TOKEN_DATA,
    TOKEN_READ,
    TOKEN_TO,
    TOKEN_STEP,
    TOKEN_DIM,
    TOKEN_RUN,
    TOKEN_LIST,
    TOKEN_NEW,
    TOKEN_LEFT,
    TOKEN_RIGHT,
    TOKEN_SLEEP,
    TOKEN_ASSERT,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_XOR,
    TOKEN_NOT,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_CHR,
    TOKEN_ASC,
    TOKEN_SAVE,
    TOKEN_LOAD,
    TOKEN_DEF,
    TOKEN_FN,
    TOKEN_LEN,
    TOKEN_KEYWORDS_END,
    
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_PERCENT,
    TOKEN_CARET,
    TOKEN_EQ,
    TOKEN_GT,
    TOKEN_LT,
    TOKEN_GE,
    TOKEN_LE,
    TOKEN_NE,
    TOKEN_COLON,
    TOKEN_COMMA,
    TOKEN_DOLLAR,
    TOKEN_DOT,
    TOKEN_BRAC_OPEN,
    TOKEN_BRAC_CLOSE,
    TOKEN_QUESTION_MARK,
    
    TOKEN_STRING,
    TOKEN_FLOAT,
    TOKEN_INTEGER,
    TOKEN_IDENTIFIER,
    TOKEN_EOF,
} TokenType;

typedef struct {
    TokenType type;
    int start;
    int len;
} Token;

typedef struct {
    const char* input;
    int offset;
    bool token_is_cached;
    Token cached_token;
} Scanner;

bool testToken(Scanner* scanner, TokenType type);

bool acceptToken(Scanner* scanner, TokenType type);

bool consumeToken(Scanner* scanner, TokenType type, Token* token);

int getScannerOffset(Scanner* scanner);

bool isHexChar(char c);

#endif