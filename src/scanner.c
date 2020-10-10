
#include <strings.h>

#include "scanner.h"

static const char* tokenTypeToString[] = {
    [TOKEN_END] = "END",
    [TOKEN_STOP] = "STOP",
    [TOKEN_KEY] = "KEY",
    [TOKEN_RETURN] = "RETURN",
    [TOKEN_RAN] = "RAN",
    [TOKEN_BEEP] = "BEEP",
    [TOKEN_TAB] = "TAB",
    [TOKEN_SPC] = "SPC",
    [TOKEN_SIN] = "SIN",
    [TOKEN_COS] = "COS",
    [TOKEN_TAN] = "TAN",
    [TOKEN_ASN] = "ASN",
    [TOKEN_ACS] = "ACS",
    [TOKEN_ATN] = "ATN",
    [TOKEN_LOG] = "LOG",
    [TOKEN_LN] = "LN",
    [TOKEN_EXP] = "EXP",
    [TOKEN_SQR] = "SQR",
    [TOKEN_ABS] = "ABS",
    [TOKEN_SGN] = "SGN",
    [TOKEN_INT] = "INT",
    [TOKEN_FRAC] = "FRAC",
    [TOKEN_RND] = "RND",
    [TOKEN_DEG] = "DEG",
    [TOKEN_RAD] = "RAD",
    [TOKEN_VAL] = "VAL",
    [TOKEN_STR] = "STR",
    [TOKEN_GOTO] = "GOTO",
    [TOKEN_GOSUB] = "GOSUB",
    [TOKEN_NEXT] = "NEXT",
    [TOKEN_RESTORE] = "RESTORE",
    [TOKEN_MID] = "MID",
    [TOKEN_LET] = "LET",
    [TOKEN_IF] = "IF",
    [TOKEN_THEN] = "THEN",
    [TOKEN_ELSE] = "ELSE",
    [TOKEN_FOR] = "FOR",
    [TOKEN_ON] = "ON",
    [TOKEN_MOD] = "MOD",
};

static bool isHexChar(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

static Token getToken(const char* input) {
    Token ret;
    ret.type = TOKEN_NONE;
    if (input[0] == 0) {
        ret.type = TOKEN_EOF;
        ret.len = 0;
    } else if ((input[0] >= 'a' && input[0] <= 'z') || (input[0] >= 'A' && input[0] <= 'Z') || input[0] == '_') {
        int len = 1;
        while ((input[len] >= 'a' && input[len] <= 'z') || (input[len] >= 'A' && input[len] <= 'Z') || (input[len] >= '0' && input[len] <= '9') || input[len] == '_') {
            len++;
        }
        for(int i = TOKEN_KEYWORDS_START + 1; i < TOKEN_KEYWORDS_END; i++) {
            if (strncasecmp(tokenTypeToString[i], input, len) == 0) {
                ret.type = i;
            }
        }
        if(ret.type == TOKEN_NONE) {
            ret.type = TOKEN_IDENTIFIER;
        }
        ret.len = len;
    } else if ((input[0] >= '0' && input[0] <= '9') || (input[0] == '.' && (input[1] >= '0' && input[1] <= '9'))) {
        if (input[1] == 'b') {
            int len = 2;
            while (input[len] == '0' || input[len] == '1' || input[len] == '_') {
                len++;
            }
            ret.type = TOKEN_INTEGER;
            ret.len = len;
        }
        if (input[1] == 'o') {
            int len = 2;
            while ((input[len] >= '0' && input[len] <= '7') || input[len] == '_') {
                len++;
            }
            ret.type = TOKEN_INTEGER;
            ret.len = len;
        }
        if (input[1] == 'h' || input[1] == 'x') {
            int len = 2;
            while (isHexChar(input[len]) || input[len] == '_') {
                len++;
            }
            ret.type = TOKEN_INTEGER;
            ret.len = len;
        }
        int len = 0;
        bool is_float = false;
        while ((input[len] >= '0' && input[len] <= '9') || input[len] == '_') {
            len++;
        }
        if (input[len] == '.') {
            len++;
            is_float = true;
            while ((input[len] >= '0' && input[len] <= '9') || input[len] == '_') {
                len++;
            }
        }
        if (input[len] == 'e' && (input[len + 1] == '-' || input[len + 1] == '+' || (input[len + 1] >= '0' && input[len + 1] <= '9') || input[len + 1] == '_')) {
            is_float = true;
            len++;
            if (input[len] == '+' || input[len] == '-') {
                len++;
            }
            while ((input[len] >= '0' && input[len] <= '9') || input[len] == '_') {
                len++;
            }
        }
        ret.type = is_float ? TOKEN_FLOAT : TOKEN_INTEGER;
        ret.len = len;
    } else if (input[0] == '\"') {
        int len = 1;
        while (input[len] != '\"' && input[len] != 0) {
            if (input[len] == '\\' && input[len + 1] == '\"') {
                len++;
            }
            len++;
        }
        if (input[len] == 0) {
            ret.type = TOKEN_STRING;
            ret.len = len;
        } else {
            len++;
            ret.type = TOKEN_STRING;
            ret.len = len;
        }
    } else {
        switch (input[0]) {
        case '$':
            ret.type = TOKEN_DOLLAR;
            ret.len = 1;
        case '=':
            ret.type = TOKEN_EQ;
            ret.len = 1;
        case '>':
            switch (input[1]) {
            case '=':
                ret.type = TOKEN_GE;
                ret.len = 2;
            default:
                ret.type = TOKEN_GT;
                ret.len = 1;
            }
        case '<':
            switch (input[1]) {
            case '=':
                ret.type = TOKEN_LE;
                ret.len = 2;
            default:
                ret.type = TOKEN_LT;
                ret.len = 1;
            }
        case '^':
            ret.type = TOKEN_CARET;
            ret.len = 1;
        case '+':
            ret.type = TOKEN_PLUS;
            ret.len = 1;
        case '-':
            ret.type = TOKEN_MINUS;
            ret.len = 1;
        case '*':
            ret.type = TOKEN_STAR;
            ret.len = 1;
        case '/':
            ret.type = TOKEN_SLASH;
            ret.len = 1;
        case '%':
            ret.type = TOKEN_PERCENT;
            ret.len = 1;
        case '!':
            switch (input[1]) {
            case '=':
                ret.type = TOKEN_NE;
                ret.len = 2;
            }
        case '.':
            ret.type = TOKEN_DOT;
            ret.len = 1;
        case ':':
            ret.type = TOKEN_COLON;
            ret.len = 1;
        case ',':
        case ';':
            ret.type = TOKEN_COMMA;
            ret.len = 1;
        case '(':
            ret.type = TOKEN_BRAC_OPEN;
            ret.len = 1;
        case ')':
            ret.type = TOKEN_BRAC_CLOSE;
            ret.len = 1;
        }
    }
    return ret;
}

bool acceptToken(Scanner* scanner, TokenType type) {
    if(!scanner->token_is_cached) {
        scanner->cached_token = getToken(scanner->input + scanner->offset);
        scanner->cached_token.start = scanner->offset;
        scanner->offset += scanner->cached_token.len;
        scanner->token_is_cached = true;
    }
    bool ret = (type == scanner->cached_token.type);
    if(ret) {
        scanner->token_is_cached = false;
    }
    return ret;
}

bool consumeToken(Scanner* scanner, TokenType type, Token* token) {
    if(!scanner->token_is_cached) {
        scanner->cached_token = getToken(scanner->input + scanner->offset);
        scanner->cached_token.start = scanner->offset;
        scanner->offset += scanner->cached_token.len;
        scanner->token_is_cached = true;
    }
    *token = scanner->cached_token;
    bool ret = (type == scanner->cached_token.type);
    if(ret) {
        scanner->token_is_cached = false;
    }
    return ret;
}

int getScannerOffset(Scanner* scanner) {
    return scanner->token_is_cached ? scanner->cached_token.start : scanner->offset;
}
