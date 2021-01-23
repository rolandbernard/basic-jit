
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <math.h>

#include "parser/parser.h"
#include "parser/ast.h"
#include "parser/scanner.h"
#include "common/utf8.h"

#define INITIAL_LIST_LENGTH 32

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
            *length = 3;
        } else {
            ret = -1;
        }
        break;
    case 'u':
        if (isHexChar(data[1]) && isHexChar(data[2]) && isHexChar(data[3]) && isHexChar(data[4])) {
            ret = (hexCharToInt(data[1]) << 12) | (hexCharToInt(data[2]) << 8) | (hexCharToInt(data[3]) << 4) | hexCharToInt(data[4]);
            *length = 5;
        } else {
            ret = -1;
        }
        break;
    case 'U':
        if (isHexChar(data[1]) && isHexChar(data[2]) && isHexChar(data[3]) && isHexChar(data[4]) && isHexChar(data[5]) && isHexChar(data[6]) && isHexChar(data[7]) && isHexChar(data[8])) {
            ret = (hexCharToInt(data[1]) << 28) | (hexCharToInt(data[2]) << 24) | (hexCharToInt(data[3]) << 20) | (hexCharToInt(data[4]) << 16);
            ret |= (hexCharToInt(data[5]) << 12) | (hexCharToInt(data[6]) << 8) | (hexCharToInt(data[7]) << 4) | hexCharToInt(data[8]);
            *length = 9;
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

static AstError* createError(int offset, StackAllocator* mem) {
    AstError* ret = (AstError*)allocAligned(mem, sizeof(AstError));
    ret->type = AST_ERROR;
    ret->offset = offset;
    return ret;
}

static Ast* parseMultiple(Scanner* scanner, StackAllocator* mem);

static Ast* parseExpression(Scanner* scanner, StackAllocator* mem);

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
            base = 2;
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
        len--;
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
        len--;
        int exp = 0;
        int sign = 1;
        if(*str == '-') {
            sign = -1;
            str++;
            len--;
        } else if(*str == '+') {
            str++;
            len--;
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

static char* copyEscapedString(const char* str, int len, StackAllocator* mem) {
    char* ret = (char*)allocAligned(mem, len + 1);
    int new = 0;
    int old = 0;
    while (old < len) {
        if (str[old] == '\\') {
            int length;
            int codepoint = parseEscapeCode(str + old + 1, &length);
            if (codepoint == -1) {
                return NULL;
            } else {
                new += writeUTF8(codepoint, ret + new);
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

static char* copyIdentifier(const char* str, int len, StackAllocator* mem) {
    char* ret = (char*)allocAligned(mem, len + 1);
    memcpy(ret, str, len);
    ret[len] = 0;
    return ret;
}

static Ast* parseBaseExpression(Scanner* scanner, StackAllocator* mem) {
    int error_offset = getScannerOffset(scanner);
    Token consumed;
    consumeToken(scanner, TOKEN_NONE, &consumed);
    if(acceptToken(scanner, TOKEN_RAN)) {
        Ast* ret = (Ast*)allocAligned(mem, sizeof(Ast*));
        ret->type = AST_RAN;
        return ret;
    } else if(acceptToken(scanner, TOKEN_INTEGER)) {
        AstInt* ret = (AstInt*)allocAligned(mem, sizeof(AstInt));
        ret->type = AST_INTEGER;
        ret->value = stringToInt(scanner->input + consumed.start, consumed.len);
        return (Ast*)ret;
    } else if(acceptToken(scanner, TOKEN_FLOAT)) {
        AstFloat* ret = (AstFloat*)allocAligned(mem, sizeof(AstFloat));
        ret->type = AST_FLOAT;
        ret->value = stringToFloat(scanner->input + consumed.start, consumed.len);
        return (Ast*)ret;
    } else if(acceptToken(scanner, TOKEN_STRING)) {
        char* str = copyEscapedString(scanner->input + consumed.start + 1, consumed.len - 2, mem);
        if(str != NULL) {
            AstString* ret = (AstString*)allocAligned(mem, sizeof(AstString));
            ret->type = AST_STRING;
            ret->str = str;
            return (Ast*)ret;
        } else {
            return (Ast*)createError(error_offset, mem);
        }
    } else if(acceptToken(scanner, TOKEN_KEY)) {
        Ast* ret = (Ast*)allocAligned(mem, sizeof(Ast*));
        ret->type = AST_KEY;
        return ret;
    } else if(acceptToken(scanner, TOKEN_IDENTIFIER)) {
        char* name = copyIdentifier(scanner->input + consumed.start, consumed.len, mem);
        AstVar* ret = (AstVar*)allocAligned(mem, sizeof(AstVar));
        ret->type = AST_VAR;
        ret->name = name;
        ret->var_type = VAR_UNDEF;
        if(acceptToken(scanner, TOKEN_DOT)) {
            ret->var_type = VAR_FLOAT;
        } else if(acceptToken(scanner, TOKEN_DOLLAR)) {
            ret->var_type = VAR_STR;
        } else if(acceptToken(scanner, TOKEN_PERCENT)) {
            ret->var_type = VAR_INT;
        } else if(acceptToken(scanner, TOKEN_QUESTION_MARK)) {
            ret->var_type = VAR_BOOL;
        }
        if(acceptToken(scanner, TOKEN_BRAC_OPEN)) {
            int count = 0;
            int capacity = INITIAL_LIST_LENGTH;
            Ast** tmp_data = (Ast**)malloc(sizeof(Ast*) * capacity);
            do {
                if (count == capacity) {
                    capacity *= 2;
                    tmp_data = (Ast**)realloc(tmp_data, sizeof(Ast*) * capacity);
                }
                tmp_data[count] = parseExpression(scanner, mem);
                if (tmp_data[count] != NULL) {
                    if (tmp_data[count]->type == AST_ERROR) {
                        free(tmp_data);
                        return tmp_data[count];
                    }
                    count++;
                }
            } while (acceptToken(scanner, TOKEN_COMMA));
            if(acceptToken(scanner, TOKEN_BRAC_CLOSE)) {
                AstIndex* index = (AstIndex*)allocAligned(mem, sizeof(AstIndex));
                index->type = AST_INDEX;
                index->name = ret;
                index->count = count;
                index->size = (Ast**)allocAligned(mem, sizeof(Ast*) * count);
                for(int i = 0; i < count; i++) {
                    index->size[i] = tmp_data[i];
                }
                free(tmp_data);
                return (Ast*)index;
            } else {
                free(tmp_data);
                return (Ast*)createError(getScannerOffset(scanner), mem);
            }
        } else {
            return (Ast*)ret;
        }
    } else if(acceptToken(scanner, TOKEN_BRAC_OPEN)) {
        Ast* ret = parseExpression(scanner, mem);
        if(!acceptToken(scanner, TOKEN_BRAC_CLOSE)) {
            return (Ast*)createError(getScannerOffset(scanner), mem);
        } else {
            return ret;
        }
    } else if(acceptToken(scanner, TOKEN_TRUE)) {
        Ast* ret = (Ast*)allocAligned(mem, sizeof(Ast*));
        ret->type = AST_TRUE;
        return ret;
    } else if(acceptToken(scanner, TOKEN_FALSE)) {
        Ast* ret = (Ast*)allocAligned(mem, sizeof(Ast*));
        ret->type = AST_FALSE;
        return ret;
    } else {
        return NULL;
    }
}

static Ast* parseFunctionLikeBinaryExpression(Scanner* scanner, StackAllocator* mem) {
    AstType type = AST_NONE;
    if (acceptToken(scanner, TOKEN_LEFT)) {
        acceptToken(scanner, TOKEN_DOLLAR);
        type = AST_LEFT;
    } else if (acceptToken(scanner, TOKEN_RIGHT)) {
        acceptToken(scanner, TOKEN_DOLLAR);
        type = AST_RIGHT;
    }
    if(type != AST_NONE) {
        if(acceptToken(scanner, TOKEN_BRAC_OPEN)) {
            Ast* a = parseExpression(scanner, mem);
            if (a == NULL) {
                return (Ast*)createError(getScannerOffset(scanner), mem);
            } else if (a->type == AST_ERROR) {
                return a;
            }
            if(acceptToken(scanner, TOKEN_COMMA)) {
                Ast* b = parseExpression(scanner, mem);
                if (b == NULL) {
                    return (Ast*)createError(getScannerOffset(scanner), mem);
                } else if (b->type == AST_ERROR) {
                    return b;
                }
                if(acceptToken(scanner, TOKEN_BRAC_CLOSE)) {
                    AstBinary* ast = (AstBinary*)allocAligned(mem, sizeof(AstBinary));
                    ast->type = type;
                    ast->first = a;
                    ast->second = b;
                    return (Ast*)ast;
                }
            }
        }
        return (Ast*)createError(getScannerOffset(scanner), mem);
    } else {
        return parseBaseExpression(scanner, mem);
    }
}
    
static Ast* parseUnaryExpression(Scanner* scanner, StackAllocator* mem) {
    AstType type = AST_NONE;
    {
        if (acceptToken(scanner, TOKEN_TAB)) {
            acceptToken(scanner, TOKEN_DOLLAR);
            type = AST_TAB;
        } else if (acceptToken(scanner, TOKEN_SPC)) {
            acceptToken(scanner, TOKEN_DOLLAR);
            type = AST_SPC;
        } else if (acceptToken(scanner, TOKEN_SIN)) {
            type = AST_SIN;
        } else if (acceptToken(scanner, TOKEN_COS)){
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
        } else if (acceptToken(scanner, TOKEN_MINUS)) {
            type = AST_NEG;
        } else if (acceptToken(scanner, TOKEN_NOT)) {
            type = AST_NOT;
        } else if (acceptToken(scanner, TOKEN_STR)) {
            acceptToken(scanner, TOKEN_DOLLAR);
            type = AST_STR;
        } else if (acceptToken(scanner, TOKEN_CHR)) {
            acceptToken(scanner, TOKEN_DOLLAR);
            type = AST_CHR;
        } else if (acceptToken(scanner, TOKEN_ASC)) {
            type = AST_ASC;
        }
    }
    if(type != AST_NONE) {
        int error_offset = getScannerOffset(scanner);
        Ast* value = parseFunctionLikeBinaryExpression(scanner, mem);
        if (value == NULL) {
            return (Ast*)createError(error_offset, mem);
        } else if (value->type == AST_ERROR) {
            return value;
        }
        AstUnary* ret = (AstUnary*)allocAligned(mem, sizeof(AstUnary));
        ret->type = type;
        ret->value = value;
        return (Ast*)ret;
    } else {
        return parseFunctionLikeBinaryExpression(scanner, mem);
    }
}

static Ast* parsePowExpression(Scanner* scanner, StackAllocator* mem) {
    Ast* ret = parseUnaryExpression(scanner, mem);
    if(ret == NULL || ret->type == AST_ERROR) {
        return ret;
    }
    if (acceptToken(scanner, TOKEN_CARET)) {
        Ast* second = parsePowExpression(scanner, mem);
        if (second == NULL) {
            return (Ast*)createError(getScannerOffset(scanner), mem);
        } else if (second->type == AST_ERROR) {
            return second;
        }
        AstBinary* parent = (AstBinary*)allocAligned(mem, sizeof(AstBinary));
        parent->type = AST_POW;
        parent->first = ret;
        parent->second = second;
        ret = (Ast*)parent;
    }
    return (Ast*)ret;
}

static Ast* parseMultiplicativeExpression(Scanner* scanner, StackAllocator* mem) {
    Ast* ret = parsePowExpression(scanner, mem);
    if (ret == NULL || ret->type == AST_ERROR) {
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
            Ast* second = parsePowExpression(scanner, mem);
            if (second == NULL) {
                return (Ast*)createError(getScannerOffset(scanner), mem);
            } else if (second->type == AST_ERROR) {
                return second;
            }
            AstBinary* parent = (AstBinary*)allocAligned(mem, sizeof(AstBinary));
            parent->type = type;
            parent->first = ret;
            parent->second = second;
            ret = (Ast*)parent;
        }
    } while(type != AST_NONE);
    return (Ast*)ret;
}

static Ast* parseAdditiveExpression(Scanner* scanner, StackAllocator* mem) {
    Ast* ret = parseMultiplicativeExpression(scanner, mem);
    if (ret == NULL || ret->type == AST_ERROR) {
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
            Ast* second = parseMultiplicativeExpression(scanner, mem);
            if (second == NULL) {
                return (Ast*)createError(getScannerOffset(scanner), mem);
            } else if (second->type == AST_ERROR) {
                return second;
            }
            AstBinary* parent = (AstBinary*)allocAligned(mem, sizeof(AstBinary));
            parent->type = type;
            parent->first = ret;
            parent->second = second;
            ret = (Ast*)parent;
        }
    } while(type != AST_NONE);
    return (Ast*)ret;
}

static Ast* parseConditionExpression(Scanner* scanner, StackAllocator* mem) {
    Ast* ret = parseAdditiveExpression(scanner, mem);
    if (ret == NULL || ret->type == AST_ERROR) {
        return ret;
    }
    AstType type;
    do {
        type = AST_NONE;
        if(acceptToken(scanner, TOKEN_EQ)) {
            type = AST_EQ;
        } else if(acceptToken(scanner, TOKEN_LT)) {
            type = AST_LT;
        } else if(acceptToken(scanner, TOKEN_GT)) {
            type = AST_GT;
        } else if(acceptToken(scanner, TOKEN_LE)) {
            type = AST_LE;
        } else if(acceptToken(scanner, TOKEN_GE)) {
            type = AST_GE;
        } else if(acceptToken(scanner, TOKEN_NE)) {
            type = AST_NE;
        }
        if (type != AST_NONE) {
            Ast* second = parseAdditiveExpression(scanner, mem);
            if (second == NULL) {
                return (Ast*)createError(getScannerOffset(scanner), mem);
            } else if (second->type == AST_ERROR) {
                return second;
            }
            AstBinary* parent = (AstBinary*)allocAligned(mem, sizeof(AstBinary));
            parent->type = type;
            parent->first = ret;
            parent->second = second;
            ret = (Ast*)parent;
        }
    } while(type != AST_NONE);
    return (Ast*)ret;
}


static Ast* parseAndExpression(Scanner* scanner, StackAllocator* mem) {
    Ast* ret = parseConditionExpression(scanner, mem);
    if (ret == NULL || ret->type == AST_ERROR) {
        return ret;
    }
    AstType type;
    do {
        type = AST_NONE;
        if (acceptToken(scanner, TOKEN_AND)) {
            type = AST_AND;
        }
        if (type != AST_NONE) {
            Ast* second = parseConditionExpression(scanner, mem);
            if (second == NULL) {
                return (Ast*)createError(getScannerOffset(scanner), mem);
            } else if (second->type == AST_ERROR) {
                return second;
            }
            AstBinary* parent = (AstBinary*)allocAligned(mem, sizeof(AstBinary));
            parent->type = type;
            parent->first = ret;
            parent->second = second;
            ret = (Ast*)parent;
        }
    } while(type != AST_NONE);
    return (Ast*)ret;
}

static Ast* parseXorExpression(Scanner* scanner, StackAllocator* mem) {
    Ast* ret = parseAndExpression(scanner, mem);
    if (ret == NULL || ret->type == AST_ERROR) {
        return ret;
    }
    AstType type;
    do {
        type = AST_NONE;
        if (acceptToken(scanner, TOKEN_XOR)) {
            type = AST_XOR;
        }
        if (type != AST_NONE) {
            Ast* second = parseAndExpression(scanner, mem);
            if (second == NULL) {
                return (Ast*)createError(getScannerOffset(scanner), mem);
            } else if (second->type == AST_ERROR) {
                return second;
            }
            AstBinary* parent = (AstBinary*)allocAligned(mem, sizeof(AstBinary));
            parent->type = type;
            parent->first = ret;
            parent->second = second;
            ret = (Ast*)parent;
        }
    } while(type != AST_NONE);
    return (Ast*)ret;
}

static Ast* parseOrExpression(Scanner* scanner, StackAllocator* mem) {
    Ast* ret = parseXorExpression(scanner, mem);
    if (ret == NULL || ret->type == AST_ERROR) {
        return ret;
    }
    AstType type;
    do {
        type = AST_NONE;
        if (acceptToken(scanner, TOKEN_OR)) {
            type = AST_OR;
        }
        if (type != AST_NONE) {
            Ast* second = parseXorExpression(scanner, mem);
            if (second == NULL) {
                return (Ast*)createError(getScannerOffset(scanner), mem);
            } else if (second->type == AST_ERROR) {
                return second;
            }
            AstBinary* parent = (AstBinary*)allocAligned(mem, sizeof(AstBinary));
            parent->type = type;
            parent->first = ret;
            parent->second = second;
            ret = (Ast*)parent;
        }
    } while(type != AST_NONE);
    return (Ast*)ret;
}

static inline Ast* parseExpression(Scanner* scanner, StackAllocator* mem) {
    return parseOrExpression(scanner, mem);
}

static Ast* parseSleepStatement(Scanner* scanner, StackAllocator* mem) {
    if (acceptToken(scanner, TOKEN_SLEEP)) {
        Ast* value = parseExpression(scanner, mem);
        if (value == NULL) {
            return (Ast*)createError(getScannerOffset(scanner), mem);
        } else if (value->type == AST_ERROR) {
            return value;
        } else {
            AstUnary* ret = (AstUnary*)allocAligned(mem, sizeof(AstUnary));
            ret->type = AST_SLEEP;
            ret->value = value;
            return (Ast*)ret;
        }
    } else {
        return NULL;
    }
}

static Ast* parseEndStatement(Scanner* scanner, StackAllocator* mem) {
    if (acceptToken(scanner, TOKEN_END)) {
        Ast* value = parseExpression(scanner, mem);
        if (value != NULL && value->type == AST_ERROR) {
            return value;
        } else {
            AstUnary* ret = (AstUnary*)allocAligned(mem, sizeof(AstUnary));
            ret->type = AST_END;
            ret->value = value;
            return (Ast*)ret;
        }
    } else {
        return NULL;
    }
}

static Ast* parseSaveStatement(Scanner* scanner, StackAllocator* mem) {
    AstType type = AST_NONE;
    if (acceptToken(scanner, TOKEN_SAVE)) {
        type = AST_SAVE;
    } else if (acceptToken(scanner, TOKEN_LOAD)) {
        type = AST_LOAD;
    }
    if (type != AST_NONE) {
        int error_offset = getScannerOffset(scanner);
        Ast* value = parseBaseExpression(scanner, mem);
        if (value == NULL) {
            return (Ast*)createError(getScannerOffset(scanner), mem);
        } else if (value->type == AST_ERROR) {
            return value;
        } else if (value->type != AST_STRING) {
            return (Ast*)createError(error_offset, mem);
        } else {
            AstUnary* ret = (AstUnary*)allocAligned(mem, sizeof(AstUnary));
            ret->type = type;
            ret->value = value;
            return (Ast*)ret;
        }
    } else {
        return NULL;
    }
}

static Ast* parseUnaryStatement(Scanner* scanner, StackAllocator* mem) {
    AstType type = AST_NONE;
    if(acceptToken(scanner, TOKEN_GOTO)) {
        type = AST_GOTO;    
    } else if(acceptToken(scanner, TOKEN_GOSUB)) {
        type = AST_GOSUB;
    } else if(acceptToken(scanner, TOKEN_NEXT)) {
        type = AST_NEXT;
    } else if(acceptToken(scanner, TOKEN_RESTORE)) {
        type = AST_RESTORE;
    } else if(acceptToken(scanner, TOKEN_LIST)) {
        type = AST_LIST;
    }
    if(type != AST_NONE) {
        int error_offset = getScannerOffset(scanner);
        Ast* value = parseExpression(scanner, mem);
        if (value == NULL) {
            if(type != AST_RESTORE && type != AST_LIST) {
                return (Ast*)createError(error_offset, mem);
            }
        } else if (value->type == AST_ERROR) {
            return value;
        } else if(value->type != AST_VAR && value->type != AST_INTEGER) {
            return (Ast*)createError(error_offset, mem);
        } else if(type == AST_LIST && value->type != AST_INTEGER) {
            return (Ast*)createError(error_offset, mem);
        } else if(value->type != AST_VAR && type == AST_NEXT) {
            return (Ast*)createError(error_offset, mem);
        } else if((type == AST_GOSUB || type == AST_GOTO || type == AST_RESTORE) &&
            value->type == AST_VAR && ((AstVar*)value)->var_type != VAR_UNDEF)
        {
            return (Ast*)createError(error_offset, mem);
        }
        AstUnary* ret = (AstUnary*)allocAligned(mem, sizeof(AstUnary));
        ret->type = type;
        ret->value = value;
        return (Ast*)ret;
    } else {
        return NULL;
    }
}

static Ast* parseSimpleStatement(Scanner* scanner, StackAllocator* mem) {
    AstType type = AST_NONE;
    if(acceptToken(scanner, TOKEN_STOP)) {
        type = AST_STOP;
    } else if(acceptToken(scanner, TOKEN_RETURN)) {
        type = AST_RETURN;
    } else if(acceptToken(scanner, TOKEN_BEEP)) {
        type = AST_BEEP;
    } else if(acceptToken(scanner, TOKEN_RUN)) {
        type = AST_RUN;
    } else if(acceptToken(scanner, TOKEN_NEW)) {
        type = AST_NEW;
    }
    if(type != AST_NONE) {
        Ast* ret = (Ast*)allocAligned(mem, sizeof(Ast));
        ret->type = type;
        return ret;
    } else {
        return NULL;
    }
}

static Ast* parseLetStatmentAfterName(Scanner* scanner, StackAllocator* mem, Token name) {
    AstVar* ast_name = (AstVar*)allocAligned(mem, sizeof(AstVar));    
    ast_name->type = AST_VAR;
    ast_name->name = copyIdentifier(scanner->input + name.start, name.len, mem);
    ast_name->var_type = VAR_UNDEF;
    if(acceptToken(scanner, TOKEN_DOT)) {
        ast_name->var_type = VAR_FLOAT;
    } else if(acceptToken(scanner, TOKEN_DOLLAR)) {
        ast_name->var_type = VAR_STR;
    } else if(acceptToken(scanner, TOKEN_PERCENT)) {
        ast_name->var_type = VAR_INT;
    } else if(acceptToken(scanner, TOKEN_QUESTION_MARK)) {
        ast_name->var_type = VAR_BOOL;
    }
    Ast* ast_name_ret = (Ast*)ast_name;
    if(acceptToken(scanner, TOKEN_BRAC_OPEN)) {
        int count = 0;
        int capacity = INITIAL_LIST_LENGTH;
        Ast** tmp_data = (Ast**)malloc(sizeof(Ast*) * capacity);
        do {
            if (count == capacity) {
                capacity *= 2;
                tmp_data = (Ast**)realloc(tmp_data, sizeof(Ast*) * capacity);
            }
            tmp_data[count] = parseExpression(scanner, mem);
            if (tmp_data[count] != NULL) {
                if (tmp_data[count]->type == AST_ERROR) {
                    free(tmp_data);
                    return tmp_data[count];
                }
                count++;
            }
        } while (acceptToken(scanner, TOKEN_COMMA));
        if(acceptToken(scanner, TOKEN_BRAC_CLOSE)) {
            AstIndex* index = (AstIndex*)allocAligned(mem, sizeof(AstIndex));
            index->type = AST_INDEX;
            index->name = ast_name;
            index->count = count;
            index->size = (Ast**)allocAligned(mem, sizeof(Ast*) * count);
            for(int i = 0; i < count; i++) {
                index->size[i] = tmp_data[i];
            }
            ast_name_ret = (Ast*)index;
            free(tmp_data);
        } else {
            free(tmp_data);
            return (Ast*)createError(getScannerOffset(scanner), mem);
        }
    }
    if(!acceptToken(scanner, TOKEN_EQ)) {
        return (Ast*)createError(getScannerOffset(scanner), mem);
    } else {
        Ast* value = parseExpression(scanner, mem);
        if (value == NULL) {
            return (Ast*)createError(getScannerOffset(scanner), mem);
        } else if (value->type == AST_ERROR) {
            return value;
        }
        AstLet* ret = (AstLet*)allocAligned(mem, sizeof(AstLet));
        ret->type = AST_LET;
        ret->name = ast_name_ret;
        ret->value = value;
        return (Ast*)ret;
    }
}

static Ast* parseLetStatmentOrLabel(Scanner* scanner, StackAllocator* mem) {
    Token name;
    if(acceptToken(scanner, TOKEN_LET)) {
        if(!consumeToken(scanner, TOKEN_IDENTIFIER, &name)) {
            return (Ast*)createError(getScannerOffset(scanner), mem);
        } else {
            return parseLetStatmentAfterName(scanner, mem, name);
        }
    } else if(consumeToken(scanner, TOKEN_IDENTIFIER, &name)) {
        if(testToken(scanner, TOKEN_COLON)) {
            AstString* ret = (AstString*)allocAligned(mem, sizeof(AstString));
            ret->type = AST_LABEL;
            ret->str = copyIdentifier(scanner->input + name.start, name.len, mem);
            return (Ast*)ret;
        } else {
            return parseLetStatmentAfterName(scanner, mem, name);
        }
    } else {
        return NULL;
    }
}

static Ast* parseInputOrPrintOrDataOrReadStatement(Scanner* scanner, StackAllocator* mem) {
    AstType type = AST_NONE;
    if(acceptToken(scanner, TOKEN_INPUT)) {
        type = AST_INPUT;    
    } else if(acceptToken(scanner, TOKEN_PRINT)) {
        type = AST_PRINT;
    } else if(acceptToken(scanner, TOKEN_DATA)) {
        type = AST_DATA;
    } else if(acceptToken(scanner, TOKEN_READ)) {
        type = AST_READ;
    }
    if(type != AST_NONE) {
        int count = 0;
        int capacity = INITIAL_LIST_LENGTH;
        Ast** tmp_data = (Ast**)malloc(sizeof(Ast*) * capacity);
        bool open_end = false;
        do {
            if (count == capacity) {
                capacity *= 2;
                tmp_data = (Ast**)realloc(tmp_data, sizeof(Ast*) * capacity);
            }
            int error_offset = getScannerOffset(scanner);
            tmp_data[count] = parseExpression(scanner, mem);
            if (tmp_data[count] != NULL) {
                if (tmp_data[count]->type == AST_ERROR) {
                    free(tmp_data);
                    return tmp_data[count];
                } else if(type == AST_INPUT && tmp_data[count]->type != AST_STRING && tmp_data[count]->type != AST_VAR) {
                    free(tmp_data);
                    return (Ast*)createError(error_offset, mem);
                } else if(type == AST_DATA && tmp_data[count]->type != AST_STRING && tmp_data[count]->type != AST_INTEGER && tmp_data[count]->type != AST_FLOAT && tmp_data[count]->type != AST_TRUE && tmp_data[count]->type != AST_FALSE) {
                    free(tmp_data);
                    return (Ast*)createError(error_offset, mem);
                } else if(type == AST_READ && tmp_data[count]->type != AST_VAR && tmp_data[count]->type != AST_INDEX) {
                    free(tmp_data);
                    return (Ast*)createError(error_offset, mem);
                }
                count++;
                open_end = false;
            } else {
                open_end = true;
            }
        } while (acceptToken(scanner, TOKEN_COMMA));
        if (count > 0) {
            AstVariable* ret = (AstVariable*)allocAligned(mem, sizeof(AstVariable));
            ret->type = type;
            ret->count = count;
            ret->open_end = open_end;
            ret->values = (Ast**)allocAligned(mem, sizeof(Ast*) * count);
            for (int i = 0; i < count; i++) {
                ret->values[i] = tmp_data[i];
            }
            free(tmp_data);
            return (Ast*)ret;
        } else {
            free(tmp_data);
            return (Ast*)createError(getScannerOffset(scanner), mem);
        }
    } else {
        return NULL;
    }
}

static Ast* parseSwitchStatement(Scanner* scanner, StackAllocator* mem) {
    if(acceptToken(scanner, TOKEN_ON)) {
        Ast* value = parseExpression(scanner, mem);
        if (value == NULL) {
            return (Ast*)createError(getScannerOffset(scanner), mem);
        } else if (value->type == AST_ERROR) {
            return value;
        }
        AstType type = AST_NONE;
        if(acceptToken(scanner, TOKEN_GOTO)) {
            type = AST_ON_GOTO;    
        } else if(acceptToken(scanner, TOKEN_GOSUB)) {
            type = AST_ON_GOSUB;
        } else {
            return (Ast*)createError(getScannerOffset(scanner), mem);
        }
        int count = 0;
        int capacity = INITIAL_LIST_LENGTH;
        Ast** tmp_data = (Ast**)malloc(sizeof(Ast*) * capacity);
        do {
            if (count == capacity) {
                capacity *= 2;
                tmp_data = (Ast**)realloc(tmp_data, sizeof(Ast*) * capacity);
            }
            int error_offset = getScannerOffset(scanner);
            tmp_data[count] = parseExpression(scanner, mem);
            if (tmp_data[count] != NULL) {
                if (tmp_data[count]->type == AST_ERROR) {
                    free(tmp_data);
                    return tmp_data[count];
                } else if (
                    (tmp_data[count]->type != AST_VAR || ((AstVar*)tmp_data[count])->var_type != VAR_UNDEF)
                    && tmp_data[count]->type != AST_INTEGER
                ) {
                    free(tmp_data);
                    return (Ast*)createError(error_offset, mem);
                }
            }
            count++;
        } while (acceptToken(scanner, TOKEN_COMMA));
        if (count > 0) {
            AstSwitch* ret = (AstSwitch*)allocAligned(mem, sizeof(AstSwitch));
            ret->type = type;
            ret->count = count;
            ret->value = value;
            ret->locations = (Ast**)allocAligned(mem, sizeof(Ast*) * count);
            for (int i = 0; i < count; i++) {
                ret->locations[i] = tmp_data[i];
            }
            free(tmp_data);
            return (Ast*)ret;
        } else {
            free(tmp_data);
            return (Ast*)createError(getScannerOffset(scanner), mem);
        }
    } else {
        return NULL;
    }
}

static Ast* parseIfThenElseStatement(Scanner* scanner, StackAllocator* mem) {
    if(acceptToken(scanner, TOKEN_IF)) {
        Ast* condition = parseExpression(scanner, mem);
        if (condition == NULL) {
            return (Ast*)createError(getScannerOffset(scanner), mem);
        } else if (condition->type == AST_ERROR) {
            return condition;
        } else {
            if(!acceptToken(scanner, TOKEN_THEN)) {
                return (Ast*)createError(getScannerOffset(scanner), mem);
            } else {
                Ast* if_true = parseMultiple(scanner, mem);
                if(if_true == NULL) {
                    int error_offset = getScannerOffset(scanner);
                    return (Ast*)createError(error_offset, mem);
                } else if (if_true->type == AST_ERROR) {
                    return if_true;
                }
                Ast* if_false = NULL;
                if(acceptToken(scanner, TOKEN_ELSE)) {
                    if_false = parseMultiple(scanner, mem);
                    if (if_false == NULL) {
                        int error_offset = getScannerOffset(scanner);
                        return (Ast*)createError(error_offset, mem);
                    } else if (if_false->type == AST_ERROR) {
                        return if_false;
                    }
                }
                AstIfThenElse* ret = (AstIfThenElse*)allocAligned(mem, sizeof(AstIfThenElse));
                ret->type = AST_IF_THEN_ELSE;
                ret->condition = condition;
                ret->if_true = if_true;
                ret->if_false = if_false;
                return (Ast*)ret;
            }
        }
    } else {
        return NULL;
    }
}

static Ast* parseAssertStatement(Scanner* scanner, StackAllocator* mem) {
    if (acceptToken(scanner, TOKEN_ASSERT)) {
        Ast* condition = parseExpression(scanner, mem);
        if (condition == NULL) {
            return (Ast*)createError(getScannerOffset(scanner), mem);
        } else if (condition->type == AST_ERROR) {
            return condition;
        } else {
            AstUnary* ret = (AstUnary*)allocAligned(mem, sizeof(AstUnary));
            ret->type = AST_ASSERT;
            ret->value = condition;
            return (Ast*)ret;
        }
    } else {
        return NULL;
    }
}

static Ast* parseForStatement(Scanner* scanner, StackAllocator* mem) {
    if(acceptToken(scanner, TOKEN_FOR)) {
        int error_offset = getScannerOffset(scanner);
        Ast* variable = parseBaseExpression(scanner, mem);
        if (variable == NULL) {
            return (Ast*)createError(error_offset, mem);
        } else if (variable->type == AST_ERROR) {
            return variable;
        } else if (variable->type != AST_VAR) {
            return (Ast*)createError(error_offset, mem);
        }
        if(acceptToken(scanner, TOKEN_EQ)) {
            Ast* initial = parseExpression(scanner, mem);
            if (initial == NULL) {
                return (Ast*)createError(error_offset, mem);
            } else if (initial->type == AST_ERROR) {
                return initial;
            }
            if(acceptToken(scanner, TOKEN_TO)) {
                Ast* end = parseExpression(scanner, mem);
                if (end == NULL) {
                    return (Ast*)createError(error_offset, mem);
                } else if (end->type == AST_ERROR) {
                    return end;
                }
                Ast* step = NULL;
                if(acceptToken(scanner, TOKEN_STEP)) {
                    step = parseExpression(scanner, mem);
                    if (step== NULL) {
                        return (Ast*)createError(error_offset, mem);
                    } else if (step->type == AST_ERROR) {
                        return step;
                    }
                } 
                AstFor* ret = (AstFor*)allocAligned(mem, sizeof(AstFor));
                ret->type = AST_FOR;
                ret->variable = (AstVar*)variable;
                ret->start = initial;
                ret->end = end;
                ret->step = step;
                return (Ast*)ret;
            } else {
                return (Ast*)createError(getScannerOffset(scanner), mem);
            }
        } else {
            return (Ast*)createError(getScannerOffset(scanner), mem);
        }
    } else {
        return NULL;
    }
}

static Ast* parseDimStatment(Scanner* scanner, StackAllocator* mem) {
    if(acceptToken(scanner, TOKEN_DIM)) {
        Token identifier;
        if(consumeToken(scanner, TOKEN_IDENTIFIER, &identifier)) {
            char* name = copyIdentifier(scanner->input + identifier.start, identifier.len, mem);
            AstVar* name_ast = (AstVar*)allocAligned(mem, sizeof(AstVar));
            name_ast->type = AST_VAR;
            name_ast->name = name;
            name_ast->var_type = VAR_UNDEF;
            if (acceptToken(scanner, TOKEN_DOT)) {
                name_ast->var_type = VAR_FLOAT;
            } else if (acceptToken(scanner, TOKEN_DOLLAR)) {
                name_ast->var_type = VAR_STR;
            } else if (acceptToken(scanner, TOKEN_PERCENT)) {
                name_ast->var_type = VAR_INT;
            } else if(acceptToken(scanner, TOKEN_QUESTION_MARK)) {
                name_ast->var_type = VAR_BOOL;
            }
            if(acceptToken(scanner, TOKEN_BRAC_OPEN)) {
                Token size;
                int count = 0;
                int capacity = INITIAL_LIST_LENGTH;
                Ast** tmp_data = (Ast**)malloc(sizeof(Ast*) * capacity);
                while((count == 0 || acceptToken(scanner, TOKEN_COMMA)) && consumeToken(scanner, TOKEN_INTEGER, &size)) {
                    if (count == capacity) {
                        capacity *= 2;
                        tmp_data = (Ast**)realloc(tmp_data, sizeof(Ast*) * capacity);
                    }
                    AstInt* ret = (AstInt*)allocAligned(mem, sizeof(AstInt));
                    ret->type = AST_INTEGER;
                    ret->value = stringToInt(scanner->input + size.start, size.len);
                    tmp_data[count] = (Ast*)ret;
                    count++;
                }
                if(acceptToken(scanner, TOKEN_BRAC_CLOSE)) {
                    AstIndex* ret = (AstIndex*)allocAligned(mem, sizeof(AstIndex));
                    ret->type = AST_DIM;
                    ret->name = name_ast;
                    ret->count = count;
                    ret->size = (Ast**)allocAligned(mem, sizeof(Ast*) * count);
                    for (int i = 0; i < count; i++) {
                        ret->size[i] = tmp_data[i];
                    }
                    free(tmp_data);
                    return (Ast*)ret;
                } else {
                    free(tmp_data);
                    return (Ast*)createError(getScannerOffset(scanner), mem);
                }
            } else {
                return (Ast*)createError(getScannerOffset(scanner), mem);
            }
        } else {
            return (Ast*)createError(getScannerOffset(scanner), mem);
        }
    }
    return NULL;
}

static Ast* parseSingleOperation(Scanner* scanner, StackAllocator* mem) {
    Ast* ret = NULL;
    if((ret = parseSimpleStatement(scanner, mem)) != NULL ||
       (ret = parseUnaryStatement(scanner, mem)) != NULL ||
       (ret = parseAssertStatement(scanner, mem)) != NULL ||
       (ret = parseEndStatement(scanner, mem)) != NULL ||
       (ret = parseSaveStatement(scanner, mem)) != NULL ||
       (ret = parseSleepStatement(scanner, mem)) != NULL ||
       (ret = parseSwitchStatement(scanner, mem)) != NULL ||
       (ret = parseIfThenElseStatement(scanner, mem)) != NULL ||
       (ret = parseForStatement(scanner, mem)) != NULL ||
       (ret = parseDimStatment(scanner, mem)) != NULL ||
       (ret = parseInputOrPrintOrDataOrReadStatement(scanner, mem)) != NULL ||
       (ret = parseLetStatmentOrLabel(scanner, mem)) != NULL);
    return ret;
}

static Ast* parseMultiple(Scanner* scanner, StackAllocator* mem) {
    int count = 0;
    int capacity = INITIAL_LIST_LENGTH;
    Ast** tmp_data = (Ast**)malloc(sizeof(Ast*) * capacity);
    bool open_end = false;
    do {
        if (count == capacity) {
            capacity *= 2;
            tmp_data = (Ast**)realloc(tmp_data, sizeof(Ast*) * capacity);
        }
        tmp_data[count] = parseSingleOperation(scanner, mem);
        if (tmp_data[count] != NULL) {
            if (tmp_data[count]->type == AST_ERROR) {
                return tmp_data[count];
            }
            count++;
            open_end = false;
        } else {
            open_end = true;
        }
    } while(acceptToken(scanner, TOKEN_COLON));
    if(count > 1) {
        AstVariable* ret = (AstVariable*)allocAligned(mem, sizeof(AstVariable));
        ret->type = AST_MULTIPLE;
        ret->count = count;
        ret->open_end = open_end;
        ret->values = (Ast**)allocAligned(mem, sizeof(Ast*) * count);
        for (int i = 0; i < count; i++) {
            ret->values[i] = tmp_data[i];
        }
        free(tmp_data);
        return (Ast*)ret;
    } else if(count != 0) {
        Ast* ret = tmp_data[0];
        free(tmp_data);
        return ret;
    } else {
        free(tmp_data);
        return NULL;
    }
}

static Ast* parseLineRoot(Scanner* scanner, StackAllocator* mem) {
    Token number;
    if(consumeToken(scanner, TOKEN_INTEGER, &number)) {
        Ast* line = parseMultiple(scanner, mem);
        if (line != NULL && line->type == AST_ERROR) {
            return line;
        }
        AstLineNum* ret = (AstLineNum*)allocAligned(mem, sizeof(AstLineNum));
        ret->type = AST_LINENUM;
        ret->number = stringToInt(scanner->input + number.start, number.len);
        ret->line = line;
        return (Ast*)ret;
    } else {
        return parseMultiple(scanner, mem);
    }
}

Ast* parseLine(const char* line, StackAllocator* mem) {
    Scanner scanner = {
        .input = line,
        .offset = 0,
        .token_is_cached = false,
    };
    Ast* ret = parseLineRoot(&scanner, mem);
    if (!acceptToken(&scanner, TOKEN_EOF)) {
        return (Ast*)createError(getScannerOffset(&scanner), mem);
    } else {
        return ret;
    }
}

Ast* parseExpressionLine(const char* line, StackAllocator* mem) {
    Scanner scanner = {
        .input = line,
        .offset = 0,
        .token_is_cached = false,
    };
    Ast* ret;
    Ast* exp = parseExpression(&scanner, mem);
    if(exp != NULL && exp->type != AST_ERROR) {
        AstVariable* prnt = (AstVariable*)allocAligned(mem, sizeof(AstVariable));
        prnt->type = AST_PRINT;
        prnt->open_end = false;
        prnt->count = 1;
        prnt->values = (Ast**)allocAligned(mem, sizeof(Ast*));
        prnt->values[0] = exp;
        ret = (Ast*)prnt;
    } else if(exp == NULL) {
        ret = parseLineRoot(&scanner, mem);
    } else {
        ret = exp;
    }
    if (!acceptToken(&scanner, TOKEN_EOF)) {
        return (Ast*)createError(getScannerOffset(&scanner), mem);
    } else {
        return ret;
    }
}
