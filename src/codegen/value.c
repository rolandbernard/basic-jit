
#include "codegen/value.h"

static const char* error_to_name[] = {
    [ERROR_NONE] = "No error",
    [ERROR_SYNTAX] = "Syntax error",
    [ERROR_TYPE] = "Type error",
    [ERROR_VARIABLE_NOT_DEF] = "Variable not defined",
    [ERROR_NO_MATCHING_FOR] = "No matching for",
    [ERROR_ARRAY_NOT_DEF] = "Array not defined",
    [ERROR_ARRAY_DIM_COUNT_MISMATCH] = "Array size mismatch",
    [ERROR_DUBLICATE_LABEL] = "Dublicate label name",
    [ERROR_UNINDEXED_ARRAY] = "Unindexed array use",
    [ERROR_FUNC_NOT_DEF] = "Function not defined",
    [ERROR_TO_MANY_PARAMS] = "To many function arguments",
    [ERROR_TO_FEW_PARAMS] = "To few function arguments",
    [ERROR_NOT_SUPPORTED] = "Operation not supported",
};

const char* getErrorName(Error e) {
    return error_to_name[e];
}
