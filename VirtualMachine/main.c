#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include <instruction_table.h>
#include <common.c>

#define push(x) *Top++ = (x)
#define pop() *--Top
#define pushes(x) assert(Top+(x) - Stack <= StackSize)
#define pops(x) assert(Top - (x) >= Stack)

#define unaOpCase(M, Op)                        \
    case M: {                                   \
        pops(1);                                \
        int32_t val = pop();                    \
        pushes(1);                              \
        push(Op val);                           \
    } break

#define binOpCase(M, Op)                        \
    case M: {                                   \
        pops(2);                                \
        int32_t rhs = pop();                    \
        int32_t lhs = pop();                    \
        pushes(1);                              \
        push(lhs Op rhs);                       \
    } break

#define binFnCase(M, Fun)                       \
    case M: {                                   \
        pops(2);                                \
        int32_t rhs = pop();                    \
        int32_t lhs = pop();                    \
        pushes(1);                              \
        push(Fun(lhs, rhs));                    \
    } break


static int32_t executeVm(uint8_t *Code) {
    enum { StackSize = 1<<10 };
    int32_t Stack[StackSize];
    int32_t *Top = Stack;

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
            unaOpCase(NOT,  ~);
            binOpCase(LSH, <<);
            binOpCase(RSH, >>);
            binOpCase(MOD,  %);
            unaOpCase(SYM,  -);
            binFnCase(POW, pow);

            case NOP: {} break;

            default:
            {
                fatalError("Illegal opcode.");
                break;
            } break;
        }
    }

    return 0;
}

int main(int ArgCount, char *ArgVal[]) {
    if(ArgCount < 2) {
        fprintf(stderr, "Usage: %s FILE\n", ArgVal[0]);
        exit(1);
    }

    uint8_t *Code = readEntireFile(ArgVal[1]);

    printf("Result: %d\n", executeVm(Code));

    return 0;
}
