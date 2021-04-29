#ifndef _VALUE_H_
#define _VALUE_H_

#include "codegen/instructions.h"

typedef enum {
    VALUE_ERROR,
    VALUE_NONE,
    VALUE_INT,
    VALUE_FLOAT,
    VALUE_STRING,
    VALUE_BOOLEAN,
} ValueType;

typedef enum {
    ERROR_NONE,
    ERROR_SYNTAX,
    ERROR_TYPE,
    ERROR_VARIABLE_NOT_DEF,
    ERROR_NO_MATCHING_FOR,
    ERROR_ARRAY_NOT_DEF,
    ERROR_ARRAY_DIM_COUNT_MISMATCH,
    ERROR_DUBLICATE_LABEL,
    ERROR_UNINDEXED_ARRAY,
    ERROR_FUNC_NOT_DEF,
    ERROR_TO_MANY_PARAMS,
    ERROR_TO_FEW_PARAMS,
    ERROR_NOT_SUPPORTED,
    ERROR_LOAD,
    ERROR_DL_LOAD,
} Error;

typedef struct {
    ValueType type;
    union {
        Error error;
        Register reg;
    };
} Value;

const char* getErrorName(Error e);

#endif
