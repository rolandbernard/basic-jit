
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "parser/parser.h"
#include "common/stackalloc.h"

int main(int argc, char** argv) {
    StackAllocator mem = STACK_ALLOCATOR_INITIALIZER;
    Ast* ast = parseLine("If 5 < 10 Then Print 42", &mem);
    if(ast != NULL && ast->type == AST_ERROR) {
        fprintf(stderr, "Error at: %i\n", ((AstError*)ast)->offset);
    }
    free_stack(&mem);
    return 0;
}
