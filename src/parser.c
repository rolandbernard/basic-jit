
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <math.h>

#include "parser.h"
#include "ast.h"
#include "scanner.h"
#include "stackalloc.h"

#define MAX_LIST_LENGTH (1 << 20)
static Ast* tmp_data[MAX_LIST_LENGTH];

static int hexCharToInt(char c) {
    if(c >= '0' && c <= '9') {
        return c - '0';
    } else if(c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    } else if(c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    } else {
        return 0;
    }
}

static int parseEscapeCode(const char* data, int* length) {
    int ret;
    switch (data[0]) {
    case 'a':
        ret = '\a';
        *length = 1;
        break;
    case 'b':
        ret = '\b';
        *length = 1;
        break;
    case 't':
        ret = '\t';
        *length = 1;
        break;
    case 'n':
        ret = '\n';
        *length = 1;
        break;
    case 'v':
        ret = '\v';
        *length = 1;
        break;
    case 'f':
        ret = '\f';
        *length = 1;
        break;
    case 'r':
        ret = '\r';
        *length = 1;
        break;
    case 'e':
        ret = '\e';
        *length = 1;
        break;
    case 'x':
        if (isHexChar(data[1]) && isHexChar(data[2])) {
            ret = (hexCharToInt(data[1]) << 4) | hexCharToInt(data[2]);
            *length = 2;
        } else {
            ret = -1;
        }
        break;
    case 'u':
        if (isHexChar(data[1]) && isHexChar(data[2]) && isHexChar(data[3]) && isHexChar(data[4])) {
            ret = (hexCharToInt(data[1]) << 12) | (hexCharToInt(data[2]) << 8) | (hexCharToInt(data[3]) << 4) | hexCharToInt(data[4]);
            *length = 4;
        } else {
            ret = -1;
        }
        break;
    case 'U':
        if (isHexChar(data[1]) && isHexChar(data[2]) && isHexChar(data[3]) && isHexChar(data[4]) && isHexChar(data[5]) && isHexChar(data[6]) && isHexChar(data[7]) && isHexChar(data[8])) {
            ret = (hexCharToInt(data[1]) << 28) | (hexCharToInt(data[2]) << 24) | (hexCharToInt(data[3]) << 20) | (hexCharToInt(data[4]) << 16);
            ret = (hexCharToInt(data[5]) << 12) | (hexCharToInt(data[6]) << 8) | (hexCharToInt(data[7]) << 4) | hexCharToInt(data[8]);
            *length = 8;
        } else {
            ret = -1;
        }
        break;
    default:
        ret = data[0];
        *length = 1;
        break;
    }
    return ret;
}

static int printUTF8(int c, char* out) {
    if(c <= 0x7f) {
        out[0] = (char)c;
        return 1;
    } else {
        int i = 2;
        while (c > (1 << ((i - 1) * 6 + (6 - i)))) {
            i++;
        }
        out[0] = 0;
        for(int j = 0; j < i; j++) {
            out[0] |= (1 << (7 - j));
        }
        out[0] |= (c >> ((i - 1) * 6)); 
        for(int j = 1; j < i; j++) {
            out[j] = 0x80 | ((c >> ((i - j - 1) * 6)) & 0x3f);
        }
        return i;
    }
}

static AstError* createError(int offset) {
    AstError* ret = (AstError*)alloc_on_stack(sizeof(AstError));
    ret->type = AST_ERROR;
    ret->offset = offset;
    return ret;
}

static Ast* parseExpression(Scanner* scanner);

static long stringToInt(const char* str, int len) {
    long ret = 0;
    int base = 10;
    if (str[0] == '0') {
        if (str[1] == 'x' || str[1] == 'h') {
            base = 16;
            str += 2;
            len -= 2;
        } else if (str[1] == 'o') {
            base = 8;
            str += 2;
            len -= 2;
        } else if (str[1] == 'b') {
            base = 1;
            str += 2;
            len -= 2;
        }
    }
    while (len > 0) {
        if(*str != '_') {
            ret *= base;
            ret += hexCharToInt(*str);
        }
        str++; 
        len--;
    }
    return ret;
}

static double stringToFloat(const char* str, int len) {
    double ret = 0;
    while (len > 0 && *str != '.' && *str != 'e') {
        if(*str != '_') {
            ret *= 10;
            ret += hexCharToInt(*str);
        }
        str++; 
        len--;
    }
    if(*str == '.') {
        str++;
        double mult = 1;
        while (len > 0 && *str != 'e') {
            if (*str != '_') {
                mult /= 10;
                ret += mult * hexCharToInt(*str);
            }
            str++;
            len--;
        }
    }
    if(*str == 'e') {
        str++;
        int exp = 0;
        int sign = 1;
        if(*str == '-') {
            sign = -1;
            str++;
        } else if(*str == '+') {
            str++;
        }
        while (len > 0) {
            if (*str != '_') {
                exp *= 10;
                exp += hexCharToInt(*str);
            }
            str++;
            len--;
        }
        ret *= pow(10, exp * sign);
    }
    return ret;
}

static char* copyEscapedString(const char* str, int len) {
    char* ret = (char*)alloc_on_stack(len + 1);
    int new = 0;
    int old = 0;
    while (old < len) {
        if (str[old] == '\\') {
            int length;
            int codepoint = parseEscapeCode(str + old + 1, &length);
            if (codepoint == -1) {
                return NULL;
            } else {
                new += printUTF8(codepoint, ret + new);
                old += length;
            }
            old++;
        } else {
            ret[new] = str[old];
            new++;
            old++;
        }
    }
    ret[new] = 0;
    return ret;
}

static Ast* parseBaseExpression(Scanner* scanner) {
    int error_offset = getScannerOffset(scanner);
    Token consumed;
    consumeToken(scanner, TOKEN_NONE, &consumed);
    if(acceptToken(scanner, TOKEN_RAN)) {
        Ast* ret = (Ast*)alloc_on_stack(sizeof(Ast*));
        ret->type = AST_RAN;
        return ret;
    } else if(acceptToken(scanner, TOKEN_INTEGER)) {
        AstInt* ret = (AstInt*)alloc_on_stack(sizeof(AstInt));
        ret->type = AST_INTEGER;
        ret->value = stringToInt(scanner->input + consumed.start, consumed.len);
        return (Ast*)ret;
    } else if(acceptToken(scanner, TOKEN_FLOAT)) {
        AstFloat* ret = (AstFloat*)alloc_on_stack(sizeof(AstFloat));
        ret->type = AST_FLOAT;
        ret->value = stringToFloat(scanner->input + consumed.start, consumed.len);
        return (Ast*)ret;
    } else if(acceptToken(scanner, TOKEN_STRING)) {
        char* str = copyEscapedString(scanner->input + consumed.start + 1, consumed.len - 2);
        if(str != NULL) {
            AstString* ret = (AstString*)alloc_on_stack(sizeof(AstString));
            ret->type = AST_STRING;
            ret->str = str;
            return (Ast*)ret;
        } else {
            return (Ast*)createError(error_offset);
        }
    } else if(acceptToken(scanner, TOKEN_KEY)) {
        Ast* ret = (Ast*)alloc_on_stack(sizeof(Ast*));
        ret->type = AST_KEY;
        return ret;
    } else if(acceptToken(scanner, TOKEN_IDENTIFIER)) {
        char* name = (char*)malloc(consumed.len);
        AstVar* ret = (AstVar*)alloc_on_stack(sizeof(AstVar));
        ret->type = AST_VAR;
        ret->name = name;
        ret->var_type = VAR_UNDEF;
        if(acceptToken(scanner, TOKEN_DOT)) {
            ret->var_type = VAR_FLOAT;
        } else if(acceptToken(scanner, TOKEN_STRING)) {
            ret->var_type = VAR_STR;
        } else if(acceptToken(scanner, TOKEN_PERCENT)) {
            ret->var_type = VAR_INT;
        }
        return (Ast*)ret;
    } else if(acceptToken(scanner, TOKEN_BRAC_OPEN)) {
        Ast* ret = parseExpression(scanner);
        if(!acceptToken(scanner, TOKEN_BRAC_CLOSE)) {
            return (Ast*)createError(getScannerOffset(scanner));
        } else {
            return ret;
        }
    } else {
        return NULL;
    }
}

static Ast* parseUnaryExpression(Scanner* scanner) {
    AstType type = AST_NONE;
    {
        if (acceptToken(scanner, TOKEN_TAB)) {
            type = AST_TAB;
        } else if (acceptToken(scanner, TOKEN_SPC)) {
            type = AST_SPC;
        } else if (acceptToken(scanner, TOKEN_SIN)) {
            type = AST_SIN;
        } else if (acceptToken(scanner, TOKEN_COS)) {
            type = AST_COS;
        } else if (acceptToken(scanner, TOKEN_TAN)) {
            type = AST_TAN;
        } else if (acceptToken(scanner, TOKEN_ASN)) {
            type = AST_ASN;
        } else if (acceptToken(scanner, TOKEN_ACS)) {
            type = AST_ACS;
        } else if (acceptToken(scanner, TOKEN_ATN)) {
            type = AST_ATN;
        } else if (acceptToken(scanner, TOKEN_LOG)) {
            type = AST_LOG;
        } else if (acceptToken(scanner, TOKEN_LN)) {
            type = AST_LN;
        } else if (acceptToken(scanner, TOKEN_EXP)) {
            type = AST_EXP;
        } else if (acceptToken(scanner, TOKEN_SQR)) {
            type = AST_SQR;
        } else if (acceptToken(scanner, TOKEN_ABS)) {
            type = AST_ABS;
        } else if (acceptToken(scanner, TOKEN_SGN)) {
            type = AST_SGN;
        } else if (acceptToken(scanner, TOKEN_INT)) {
            type = AST_INT;
        } else if (acceptToken(scanner, TOKEN_FRAC)) {
            type = AST_FRAC;
        } else if (acceptToken(scanner, TOKEN_RND)) {
            type = AST_RND;
        } else if (acceptToken(scanner, TOKEN_DEG)) {
            type = AST_DEG;
        } else if (acceptToken(scanner, TOKEN_RAD)) {
            type = AST_RAD;
        } else if (acceptToken(scanner, TOKEN_VAL)) {
            type = AST_VAL;
        } else if (acceptToken(scanner, TOKEN_STR)) {
            type = AST_STR;
        }
    }
    if(type != AST_NONE) {
        int error_offset = getScannerOffset(scanner);
        Ast* value = parseUnaryExpression(scanner);
        if (value == NULL) {
            return (Ast*)createError(error_offset);
        } else if (value->type == AST_ERROR) {
            return value;
        }
        AstUnary* ret = (AstUnary*)alloc_on_stack(sizeof(AstUnary));
        ret->type = type;
        ret->value = value;
        return (Ast*)ret;
    } else {
        return parseBaseExpression(scanner);
    }
}

static Ast* parseMultiplicativeExpression(Scanner* scanner) {
    int error_offset = getScannerOffset(scanner);
    Ast* ret = parseUnaryExpression(scanner);
    if (ret == NULL) {
        return (Ast*)createError(error_offset);
    } else if (ret->type == AST_ERROR) {
        return ret;
    }
    AstType type;
    do {
        type = AST_NONE;
        if (acceptToken(scanner, TOKEN_MOD)) {
            type = AST_MOD;
        } else if (acceptToken(scanner, TOKEN_STAR)) {
            type = AST_MUL;
        } else if (acceptToken(scanner, TOKEN_SLASH)) {
            type = AST_DIV;
        }
        if (type != AST_NONE) {
            Ast* second = parseUnaryExpression(scanner);
            if (ret == NULL) {
                return (Ast*)createError(error_offset);
            } else if (ret->type == AST_ERROR) {
                return ret;
            }
            AstBinary* parent = (AstBinary*)alloc_on_stack(sizeof(AstBinary));
            parent->type = type;
            parent->first = ret;
            parent->second = second;
            ret = (Ast*)parent;
        }
    } while(type != AST_NONE);
    return (Ast*)ret;
}

static Ast* parseAdditiveExpression(Scanner* scanner) {
    int error_offset = getScannerOffset(scanner);
    Ast* ret = parseMultiplicativeExpression(scanner);
    if (ret == NULL) {
        return (Ast*)createError(error_offset);
    } else if (ret->type == AST_ERROR) {
        return ret;
    }
    AstType type;
    do {
        type = AST_NONE;
        if (acceptToken(scanner, TOKEN_PLUS)) {
            type = AST_ADD;
        } else if (acceptToken(scanner, TOKEN_MINUS)) {
            type = AST_SUB;
        }
        if (type != AST_NONE) {
            Ast* second = parseMultiplicativeExpression(scanner);
            if (ret == NULL) {
                return (Ast*)createError(error_offset);
            } else if (ret->type == AST_ERROR) {
                return ret;
            }
            AstBinary* parent = (AstBinary*)alloc_on_stack(sizeof(AstBinary));
            parent->type = type;
            parent->first = ret;
            parent->second = second;
            ret = (Ast*)parent;
        }
    } while(type != AST_NONE);
    return (Ast*)ret;
}

static inline Ast* parseExpression(Scanner* scanner) {
    return parseAdditiveExpression(scanner);
}

static Ast* parseUnaryStatement(Scanner* scanner) {
    AstType type = AST_NONE;
    if(acceptToken(scanner, TOKEN_GOTO)) {
        type = AST_GOTO;    
    } else if(acceptToken(scanner, TOKEN_GOSUB)) {
        type = AST_GOSUB;
    } else if(acceptToken(scanner, TOKEN_NEXT)) {
        type = AST_NEXT;
    } else if(acceptToken(scanner, TOKEN_RESTORE)) {
        type = AST_RESTORE;
    }
    if(type != AST_NONE) {
        int error_offset = getScannerOffset(scanner);
        Ast* value = parseExpression(scanner);
        if (value == NULL) {
            return (Ast*)createError(error_offset);
        } else if (value->type == AST_ERROR) {
            return value;
        } else if(value->type != AST_VAR && value->type != AST_INTEGER) {
            return (Ast*)createError(error_offset);
        } else if(value->type != AST_VAR && type == AST_NEXT) {
            return (Ast*)createError(error_offset);
        }
        AstUnary* ret = (AstUnary*)alloc_on_stack(sizeof(AstUnary));
        ret->type = type;
        ret->value = value;
        return (Ast*)ret;
    } else {
        return NULL;
    }
}

static Ast* parseSimpleStatement(Scanner* scanner) {
    AstType type = AST_NONE;
    if(acceptToken(scanner, TOKEN_END)) {
        type = AST_END;    
    } else if(acceptToken(scanner, TOKEN_STOP)) {
        type = AST_STOP;
    } else if(acceptToken(scanner, TOKEN_RETURN)) {
        type = AST_RETURN;
    } else if(acceptToken(scanner, TOKEN_BEEP)) {
        type = AST_BEEP;
    }
    if(type != AST_NONE) {
        Ast* ret = (Ast*)alloc_on_stack(sizeof(Ast));
        ret->type = type;
        return ret;
    } else {
        return NULL;
    }
}

static Ast* parseSingleOperation(Scanner* scanner) {
    Ast* ret = NULL;
    if((ret = parseSimpleStatement(scanner)) != NULL ||
       (ret = parseUnaryStatement(scanner)) != NULL ||
       (ret = parseExpression(scanner)) != NULL);
    return ret;
}

static Ast* parseMultiple(Scanner* scanner) {
    int count = 0;
    do {
        if(count < MAX_LIST_LENGTH) {
            tmp_data[count] = parseSingleOperation(scanner);
            if (tmp_data[count] != NULL) {
                if (tmp_data[count]->type == AST_ERROR) {
                    return tmp_data[count];
                }
                count++;
            } else {
                break;
            }
        } else {
            return (Ast*)createError(scanner->cached_token.start);
        }
    } while(acceptToken(scanner, TOKEN_COLON));
    if(count > 1) {
        AstVariable* ret = (AstVariable*)alloc_on_stack(sizeof(AstVariable));
        ret->type = AST_MULTIPLE;
        ret->count = count;
        ret->values = (Ast**)alloc_on_stack(sizeof(Ast*) * count);
        for (int i = 0; i < count; i++) {
            ret->values[i] = tmp_data[i];
        }
        return (Ast*)ret;
    } else if(count != 0) {
        return tmp_data[0];
    } else {
        return NULL;
    }
}

static Ast* parseLineRoot(Scanner* scanner) {
    int error_offset = getScannerOffset(scanner);
    Ast* ret = parseMultiple(scanner); 
    if (ret == NULL) {
        return (Ast*)createError(error_offset);
    } else if (ret->type == AST_ERROR) {
        return ret;
    }
    if(!acceptToken(scanner, TOKEN_EOF)) {
        return (Ast*)createError(getScannerOffset(scanner));
    } else {
        return ret;
    }
}

Ast* parseLine(const char* line) {
    Scanner scanner = {
        .input = line,
        .offset = 0,
        .token_is_cached = false,
    };
    return parseLineRoot(&scanner);
}