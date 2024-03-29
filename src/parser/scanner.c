
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "parser/scanner.h"

#define max(X, Y) ((X) > (Y) ? (X) : (Y))

static const char* tokenTypeToString[] = {
    [TOKEN_REM] = "REM",
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
    [TOKEN_LET] = "LET",
    [TOKEN_IF] = "IF",
    [TOKEN_THEN] = "THEN",
    [TOKEN_ELSE] = "ELSE",
    [TOKEN_FOR] = "FOR",
    [TOKEN_ON] = "ON",
    [TOKEN_MOD] = "MOD",
    [TOKEN_INPUT] = "INPUT",
    [TOKEN_PRINT] = "PRINT",
    [TOKEN_DATA] = "DATA",
    [TOKEN_READ] = "READ",
    [TOKEN_TO] = "TO",
    [TOKEN_STEP] = "STEP",
    [TOKEN_DIM] = "DIM",
    [TOKEN_RUN] = "RUN",
    [TOKEN_LIST] = "LIST",
    [TOKEN_NEW] = "NEW",
    [TOKEN_LEFT] = "LEFT",
    [TOKEN_RIGHT] = "RIGHT",
    [TOKEN_SLEEP] = "SLEEP",
    [TOKEN_ASSERT] = "ASSERT",
    [TOKEN_AND] = "AND",
    [TOKEN_OR] = "OR",
    [TOKEN_XOR] = "XOR",
    [TOKEN_NOT] = "NOT",
    [TOKEN_TRUE] = "TRUE",
    [TOKEN_FALSE] = "FALSE",
    [TOKEN_CHR] = "CHR",
    [TOKEN_ASC] = "ASC",
    [TOKEN_SAVE] = "SAVE",
    [TOKEN_LOAD] = "LOAD",
    [TOKEN_DEF] = "DEF",
    [TOKEN_FN] = "FN",
    [TOKEN_LEN] = "LEN",
    [TOKEN_EDIT] = "EDIT",
    [TOKEN_EXT] = "EXT",
    [TOKEN_DLIB] = "DLIB",
};

typedef struct {
    const char* string;
    TokenType token;
} StringToTokenEntry;

#define NUM_KEYWORDS (TOKEN_KEYWORDS_END - TOKEN_KEYWORDS_START - 1)

StringToTokenEntry tokenStringToType[NUM_KEYWORDS];

static int stringToTokenEntryCompare(const void* a, const void* b) {
    return strcasecmp(((const StringToTokenEntry*)a)->string, ((const StringToTokenEntry*)b)->string);
}

static void initTokenSearchTable() {
    static bool initialized = false;
    if (!initialized) {
        for(int i = TOKEN_KEYWORDS_START + 1; i < TOKEN_KEYWORDS_END; i++) {
            tokenStringToType[i - TOKEN_KEYWORDS_START - 1].string = tokenTypeToString[i];
            tokenStringToType[i - TOKEN_KEYWORDS_START - 1].token = (TokenType)i;
        }
        qsort(tokenStringToType, NUM_KEYWORDS, sizeof(StringToTokenEntry), stringToTokenEntryCompare);
        initialized = true;
    }
}

static TokenType searchForToken(const char* string, int len) {
    char tmp[len + 1];
    memcpy(tmp, string, len);
    tmp[len] = 0;
    StringToTokenEntry search = { .string = tmp };
    StringToTokenEntry* result = bsearch(&search, tokenStringToType, NUM_KEYWORDS, sizeof(StringToTokenEntry), stringToTokenEntryCompare);
    if (result == NULL) {
        return TOKEN_NONE;
    } else {
        return result->token;
    }
}

bool isHexChar(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

static void skipWhitespace(Scanner* scanner) {
    while (isspace(scanner->input[scanner->offset])) {
        scanner->offset++;
    }
}

static Token getToken(const char* input) {
    Token ret;
    ret.type = TOKEN_NONE;
    ret.len = 1;
    if (input[0] == 0) {
        ret.type = TOKEN_EOF;
        ret.len = 0;
    } else if ((input[0] >= 'a' && input[0] <= 'z') || (input[0] >= 'A' && input[0] <= 'Z') || input[0] == '_') {
        size_t len = 1;
        while ((input[len] >= 'a' && input[len] <= 'z') || (input[len] >= 'A' && input[len] <= 'Z') || (input[len] >= '0' && input[len] <= '9') || input[len] == '_') {
            len++;
        }
        initTokenSearchTable();
        ret.type = searchForToken(input, len);
        if(ret.type == TOKEN_NONE) {
            ret.type = TOKEN_IDENTIFIER;
        }
        ret.len = len;
    } else if (
        (input[0] >= '0' && input[0] <= '9')
        || (input[0] == '.' && input[1] >= '0' && input[1] <= '9')
    ) {
        int len = 0;
        if (input[len + 1] == 'b') {
            len += 2;
            while (input[len] == '0' || input[len] == '1' || input[len] == '_') {
                len++;
            }
            ret.type = TOKEN_INTEGER;
            ret.len = len;
        } else if (input[len + 1] == 'o') {
            len += 2;
            while ((input[len] >= '0' && input[len] <= '7') || input[len] == '_') {
                len++;
            }
            ret.type = TOKEN_INTEGER;
            ret.len = len;
        } else if (input[len + 1] == 'h' || input[len + 1] == 'x') {
            len += 2;
            while (isHexChar(input[len]) || input[len] == '_') {
                len++;
            }
            ret.type = TOKEN_INTEGER;
            ret.len = len;
        } else {
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
        }
    } else if (input[0] == '\"') {
        int len = 1;
        while (input[len] != '\"' && input[len] != 0) {
            if (input[len] == '\\') {
                len++;
            }
            if (input[len] != 0) {
                len++;
            }
        }
        len++;
        ret.type = TOKEN_STRING;
        ret.len = len;
    } else {
        switch (input[0]) {
        case '?':
            ret.type = TOKEN_QUESTION_MARK;
            ret.len = 1;
            break;
        case '$':
            ret.type = TOKEN_DOLLAR;
            ret.len = 1;
            break;
        case '=':
            ret.type = TOKEN_EQ;
            ret.len = 1;
            break;
        case '>':
            switch (input[1]) {
            case '=':
                ret.type = TOKEN_GE;
                ret.len = 2;
                break;
            default:
                ret.type = TOKEN_GT;
                ret.len = 1;
                break;
            }
            break;
        case '<':
            switch (input[1]) {
            case '=':
                ret.type = TOKEN_LE;
                ret.len = 2;
                break;
            default:
                ret.type = TOKEN_LT;
                ret.len = 1;
                break;
            }
            break;
        case '^':
            ret.type = TOKEN_CARET;
            ret.len = 1;
            break;
        case '+':
            ret.type = TOKEN_PLUS;
            ret.len = 1;
            break;
        case '-':
            ret.type = TOKEN_MINUS;
            ret.len = 1;
            break;
        case '*':
            ret.type = TOKEN_STAR;
            ret.len = 1;
            break;
        case '/':
            ret.type = TOKEN_SLASH;
            ret.len = 1;
            break;
        case '%':
            ret.type = TOKEN_PERCENT;
            ret.len = 1;
            break;
        case '!':
            switch (input[1]) {
            case '=':
                ret.type = TOKEN_NE;
                ret.len = 2;
                break;
            }
            break;
        case '.':
            ret.type = TOKEN_DOT;
            ret.len = 1;
            break;
        case ':':
            ret.type = TOKEN_COLON;
            ret.len = 1;
            break;
        case ',':
        case ';':
            ret.type = TOKEN_COMMA;
            ret.len = 1;
            break;
        case '(':
            ret.type = TOKEN_BRAC_OPEN;
            ret.len = 1;
            break;
        case ')':
            ret.type = TOKEN_BRAC_CLOSE;
            ret.len = 1;
            break;
        }
    }
    return ret;
}

static inline void cacheToken(Scanner* scanner) {
    if(!scanner->token_is_cached) {
        skipWhitespace(scanner);
        scanner->cached_token = getToken(scanner->input + scanner->offset);
        if(scanner->cached_token.type == TOKEN_REM) {
            scanner->cached_token.type = TOKEN_EOF;
            scanner->cached_token.start = scanner->offset;
            scanner->offset = strlen(scanner->input);
            scanner->cached_token.len = 0;
        } else {
            scanner->cached_token.start = scanner->offset;
            scanner->offset += scanner->cached_token.len;
        }
        scanner->token_is_cached = true;
        skipWhitespace(scanner);
    }
}

bool testToken(Scanner* scanner, TokenType type) {
    cacheToken(scanner);
    return (type == scanner->cached_token.type);
}

bool acceptToken(Scanner* scanner, TokenType type) {
    cacheToken(scanner);
    bool ret = (type == scanner->cached_token.type);
    if(ret) {
        scanner->token_is_cached = false;
    }
    return ret;
}

bool consumeToken(Scanner* scanner, TokenType type, Token* token) {
    cacheToken(scanner);
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

