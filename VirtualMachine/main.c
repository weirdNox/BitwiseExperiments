#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define InvalidCodePath assert(!"InvalidCodePath")
#define InvalidDefaultCase default: { InvalidCodePath; } break

void fatalError(char *Fmt, ...) {
    va_list Args;
    va_start(Args, Fmt);
    fprintf(stderr, "Fatal error: ");
    vfprintf(stderr, Fmt, Args);
    fprintf(stderr, "\n");
    va_end(Args);
    exit(1);
}

int32_t executeVm(uint8_t *Code) {
    enum { StackSize = 1<<10 };
    int32_t Stack[StackSize];
    int32_t *Top = Stack;

#include "instruction_table.c"
#define push(x) *Top++ = (x)
#define pop() *--Top
#define pushes(x) assert(Top+(x) - Stack <= StackSize)
#define pops(x) assert(Top - (x) >= Stack)
#define binOpCase(M, Op)                        \
    case M: {                                   \
        pops(2);                                \
        int32_t rhs = pop();                    \
        int32_t lhs = pop();                    \
        pushes(1);                              \
        push(lhs Op rhs);                       \
    } break
#define binFnCase(M, Fun)                      \
    case M: {                                   \
        pops(2);                                \
        int32_t rhs = pop();                    \
        int32_t lhs = pop();                    \
        pushes(1);                              \
        push(Fun(lhs, rhs));                    \
    } break

    for(;;) {
        mnemonic Op = *Code++;
        switch(Op) {
            case HALT:
            {
                pops(1);
                return pop();
            } break;

            case LIT:
            {
                pushes(1);
                int32_t Value;
                Value  = (*Code++) <<  0;
                Value |= (*Code++) <<  8;
                Value |= (*Code++) << 16;
                Value |= (*Code++) << 24;
                push(Value);
            } break;

            binOpCase(ADD,  +);
            binOpCase(SUB,  -);
            binOpCase(MUL,  *);
            binOpCase(DIV,  /);
            binOpCase(OR,   |);
            binOpCase(XOR,  ^);
            binOpCase(AND,  &);
            binOpCase(LSH, <<);
            binOpCase(RSH, >>);
            binOpCase(MOD,  %);
            binFnCase(POW, pow);

            default:
            {
                fatalError("Illegal opcode.");
                break;
            } break;
        }
    }

    return 0;
}

int main() {
    uint8_t Code[] = {
        0x01, 0x10, 0x00, 0x00, 0x00,   // 16
        0x01, 0x05, 0x00, 0x00, 0x00,   // 5
        0x22,                           // MUL
        0x00
    };
    printf("Result: %d\n", executeVm(Code));
    return 0;
}
