#ifndef _PARSER_H_
#define _PARSER_H_

#include "parser/ast.h"
#include "common/stackalloc.h"

Ast* parseLine(const char* line, StackAllocator* mem);

Ast* parseExpressionLine(const char* line, StackAllocator* mem);

#endif