
#include "codegen/value.h"

static const char* error_to_name[] = {
    [ERROR_NONE] = "No error",
    [ERROR_SYNTAX] = "Syntax error",
    [ERROR_TYPE] = "Type error",
    [ERROR_VARIABLE_NOT_DEF] = "Variable not defined",
    [ERROR_NO_MATCHING_FOR] = "No matching for",
    [ERROR_ARRAY_NOT_DEF] = "Array not defined",
    [ERROR_ARRAY_DIM_COUNT_MISMATCH] = "Array size mismatch",
    [ERROR_DUPLICATE_LABEL] = "Duplicate label name",
    [ERROR_NON_INDEXED_ARRAY] = "Non-indexed array use",
    [ERROR_FUNC_NOT_DEF] = "Function not defined",
    [ERROR_TO_MANY_PARAMS] = "To many function arguments",
    [ERROR_TO_FEW_PARAMS] = "To few function arguments",
    [ERROR_NOT_SUPPORTED] = "Operation not supported",
    [ERROR_LOAD] = "Failed to load file",
    [ERROR_DL_LOAD] = "Failed to load library",
};

const char* getErrorName(Error e) {
    return error_to_name[e];
}
