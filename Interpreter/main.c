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

#include <common.c>
#include <lexer.c>

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

static bool isBinaryOp(void) {
    return Table[Token.Type].Kind == Operator_Binary;
}

static bool isUnaryOp(void) {
    if(Token.Type == Token_Add) {
        Token.Type = Token_UnaryPlus;
    }
    else if(Token.Type == Token_Subtract) {
        Token.Type = Token_UnaryMinus;
    }
    return Table[Token.Type].Kind == Operator_Unary;
}

static int64_t evaluate(int);

static int64_t parseUnary(void) {
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
        fatalError("No expected token available.");
    }

    return Result;
}

static int64_t evaluate(int Precedence) {
    int64_t Result = parseUnary();

    while(Table[Token.Type].Precedence >= Precedence &&
          (Token.Type != Token_EOF && Token.Type != Token_RParen))
    {
        if(!isBinaryOp()) {
            fatalError("Missing expected binary operator.");
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

int main(int ArgCount, char *ArgVal[]) {
    if(ArgCount != 2) {
        fprintf(stderr, "Usage: %s EXPR\n", ArgVal[0]);
        exit(1);
    }

    Stream = ArgVal[1];
    nextToken();

    printf("Result: %ld\n", evaluate(0));

    return 0;
}
