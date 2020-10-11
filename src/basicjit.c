
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "parser/parser.h"
#include "parser/stackalloc.h"

int main(int argc, char** argv) {

    Ast* ast = parseLine("If 5 < 10 Then Print 42");
    if(ast != NULL && ast->type == AST_ERROR) {
        fprintf(stderr, "Error at: %i\n", ((AstError*)ast)->offset);
    }

    free_complete_stack();
    return 0;
}
