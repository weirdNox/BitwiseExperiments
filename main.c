#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.c"
#include "stretchy.c"
#include "memory.c"
#include "lexer.c"
#include "ast.c"
#include "parser.c"
#include "print.c"

int main() {
    bufferTest();
    lexTest();
    parsedPrintTest();

    return 0;
}
