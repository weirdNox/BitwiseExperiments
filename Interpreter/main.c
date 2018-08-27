// This is an interpreter which uses table-driven Precedence Climbing to parse the expression.
//
// Expression syntax:
//   INT = 0 | [1-9][0-9]* | 0[xX][0-9a-fA-F]+ | 0[0-7]+ | 0[bB][0-1]+
//
//   unary_expr = [~-+] unary_expr | '(' expression ')' | INT
//   factor     = unary_expr ('^' factor)?
//   mul_op     = '*' | '/' | '%' | '<<' | '>>' | '&'
//   mul_expr   = factor   (mul_op factor)*
//   add_expr   = mul_expr ([+-|^] mul_expr)*
//   expression = add_expr
//
// References:
//   - Bitwise, https://bitwise.handmade.network
//   - Parsing Expressions by Recursive Descent, https://www.engr.mun.ca/~theo/Misc/exp_parsing.htm

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define InvalidCodePath assert(!"InvalidCodePath")
#define InvalidDefaultCase default: { InvalidCodePath; } break

static void parseError(char *Format, ...) {
    va_list Args;
    va_start(Args, Format);
    vfprintf(stderr, Format, Args);
    va_end(Args);
}


// Lexer
typedef enum token_type {
    Token_Unknown,
    Token_EOF,

    Token_Add,
    Token_Subtract,
    Token_BitOr,
    Token_BitXor,

    Token_Multiply,
    Token_Divide,
    Token_Mod,
    Token_LShift,
    Token_RShift,
    Token_BitAnd,

    Token_Power,

    Token_UnaryPlus,
    Token_UnaryMinus,
    Token_BitNot,

    Token_LParen,
    Token_RParen,

    Token_Int,

    Token_Count
} token_type;

typedef struct token {
    token_type Type;
    union {
        uint64_t IntValue;
    };
} token;

static char *Stream;
static token Token;

static int CharToDigit[256] = {
    ['0'] = 0,
    ['1'] = 1,
    ['2'] = 2,
    ['3'] = 3,
    ['4'] = 4,
    ['5'] = 5,
    ['6'] = 6,
    ['7'] = 7,
    ['8'] = 8,
    ['9'] = 9,
    ['a'] = 10, ['A'] = 10,
    ['b'] = 11, ['B'] = 11,
    ['c'] = 12, ['C'] = 12,
    ['d'] = 13, ['D'] = 13,
    ['e'] = 14, ['E'] = 14,
    ['f'] = 15, ['F'] = 15,
};

#define case1(C, Type1)                         \
    case (C): {                                 \
        Token.Type = (Type1); ++Stream;         \
    } break

#define case2(C, Type2)                                  \
    case (C): {                                          \
        if(*(Stream+1) == (C)) {                         \
            Token.Type = (Type2);                        \
        } else {                                         \
            Token.Type = Token_Unknown;                  \
        }                                                \
        Stream += 2;                                     \
    } break

#define case1and2(C, Type1, Type2)                       \
    case (C): {                                          \
        ++Stream;                                        \
        if(*Stream == (C)) {                             \
            Token.Type = (Type2);                        \
            ++Stream;                                    \
        } else {                                         \
            Token.Type = (Type1);                        \
        }                                                \
    } break

static void nextToken() {
    while(*Stream == ' ' || *Stream == '\n' || *Stream == '\t' || *Stream == '\r' || *Stream == '\v') {
        ++Stream;
    }

    switch(*Stream) {
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
        {
            Token.Type = Token_Int;

            int Base = 10;
            if(*Stream == '0') {
                switch(*(Stream+1)) {
                    case 'b': case 'B': {
                        Base = 2;
                        Stream += 2;
                    } break;

                    case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': {
                        Base = 8;
                        ++Stream;
                    } break;

                    case 'x': case 'X': {
                        Base = 16;
                        Stream += 2;
                    } break;
                }
            }

            int Digit;
            uint64_t Value = 0;
            bool Overflow = false;
            while((Digit = CharToDigit[(int)*Stream]) || *Stream == '0') {
                if(Digit >= Base) {
                    parseError("Digit is greater than base.\n");
                    Digit = 0;
                }

                if(!Overflow && Value > (UINT64_MAX - Digit)/Base) {
                    parseError("Integer overflow.\n");
                    Value = 0;
                    Overflow = true;
                }

                if(!Overflow) {
                    Value = Value * Base + Digit;
                }

                ++Stream;
            }

            Token.IntValue = Value;
        } break;

        case1('\0', Token_EOF);

        case1('+', Token_Add);
        case1('-', Token_Subtract);
        case1('|', Token_BitOr);
        case1('^', Token_BitXor);

        case1and2('*', Token_Multiply, Token_Power);
        case1('/', Token_Divide);
        case1('%', Token_Mod);
        case2('<', Token_LShift);
        case2('>', Token_RShift);
        case1('&', Token_BitAnd);

        case1('~', Token_BitNot);

        case1('(', Token_LParen);
        case1(')', Token_RParen);

        default: {
            Token.Type = Token_Unknown;
            ++Stream;
        } break;
    }
}

static bool matchToken(token_type Type) {
    if(Token.Type == Type) {
        nextToken();
        return true;
    }

    return false;
}

static bool expectToken(token_type Type) {
    if(Token.Type == Type) {
        nextToken();
        return true;
    }

    parseError("Token is not of the expected kind!\n");
    exit(1);
    return false;
}


// Parser and evaluator
typedef enum operator_assoc {
    Assoc_Left,
    Assoc_Right,
} operator_assoc;

typedef enum operator_kind {
    Operator_NoOp,
    Operator_Unary,
    Operator_Binary,
} operator_kind;

typedef struct operator {
    operator_kind Kind;
    int Precedence;
    operator_assoc Associativity;
} operator;

static operator Table[Token_Count] = {
    [Token_Add]        = {Operator_Binary, 0, Assoc_Left},
    [Token_Subtract]   = {Operator_Binary, 0, Assoc_Left},
    [Token_BitOr]      = {Operator_Binary, 0, Assoc_Left},
    [Token_BitXor]     = {Operator_Binary, 0, Assoc_Left},

    [Token_Multiply]   = {Operator_Binary, 1, Assoc_Left},
    [Token_Divide]     = {Operator_Binary, 1, Assoc_Left},
    [Token_Mod]        = {Operator_Binary, 1, Assoc_Left},
    [Token_LShift]     = {Operator_Binary, 1, Assoc_Left},
    [Token_RShift]     = {Operator_Binary, 1, Assoc_Left},
    [Token_BitAnd]     = {Operator_Binary, 1, Assoc_Left},

    [Token_Power]      = {Operator_Binary, 2, Assoc_Right},

    [Token_UnaryPlus]  = {Operator_Unary,  3, Assoc_Right},
    [Token_UnaryMinus] = {Operator_Unary,  3, Assoc_Right},
    [Token_BitNot]     = {Operator_Unary,  3, Assoc_Right},
};

static bool isBinaryOp() {
    return Table[Token.Type].Kind == Operator_Binary;
}

static bool isUnaryOp() {
    if(Token.Type == Token_Add) {
        Token.Type = Token_UnaryPlus;
    }
    else if(Token.Type == Token_Subtract) {
        Token.Type = Token_UnaryMinus;
    }
    return Table[Token.Type].Kind == Operator_Unary;
}

static int64_t evaluate(int);

static int64_t parseUnary() {
    int64_t Result = 0;

    if(isUnaryOp()) {
        token_type Type = Token.Type;

        nextToken();
        Result = evaluate(Table[Type].Precedence);

        switch(Type) {
            case Token_UnaryPlus: {} break;

            case Token_UnaryMinus: {
                Result = -Result;
            } break;

            case Token_BitNot: {
                Result = ~Result;
            } break;

            InvalidDefaultCase;
        }
    }
    else if(matchToken(Token_LParen)) {
        Result = evaluate(0);
        expectToken(Token_RParen);
    }
    else if(Token.Type == Token_Int) {
        Result = Token.IntValue;
        nextToken();
    }
    else {
        parseError("No expected token available.\n");
        exit(1);
    }

    return Result;
}

static int64_t evaluate(int Precedence) {
    int64_t Result = parseUnary();

    while(Table[Token.Type].Precedence >= Precedence &&
          (Token.Type != Token_EOF && Token.Type != Token_RParen))
    {
        if(!isBinaryOp()) {
            parseError("Missing expected binary operator.\n");
            exit(1);
        }

        token_type OpType = Token.Type;

        nextToken();

        int64_t Rhs;
        if(Table[OpType].Associativity == Assoc_Left) {
            Rhs = evaluate(Table[OpType].Precedence + 1);
        } else {
            assert(Table[OpType].Associativity == Assoc_Right);
            Rhs = evaluate(Table[OpType].Precedence);
        }

        switch(OpType) {
            case Token_Add:      { Result +=  Rhs; } break;
            case Token_Subtract: { Result -=  Rhs; } break;
            case Token_BitOr:    { Result |=  Rhs; } break;
            case Token_BitXor:   { Result ^=  Rhs; } break;

            case Token_Multiply: { Result *=  Rhs; } break;
            case Token_Divide:   { Result /=  Rhs; } break;
            case Token_Mod:      { Result %=  Rhs; } break;
            case Token_LShift:   { Result <<= Rhs; } break;
            case Token_RShift:   { Result <<= Rhs; } break;
            case Token_BitAnd:   { Result &=  Rhs; } break;

            case Token_Power:    { Result = pow(Result, Rhs); } break;

            InvalidDefaultCase;
        }
    }

    return Result;
}

int main() {
    Stream = "10%4 + 2**2**3 + 2 + (+3 * 2**2 * (3 + 1) + 1|2) & 0b10101010101";
    nextToken();
    printf("Result: %ld\n", evaluate(0));

    return 0;
}
