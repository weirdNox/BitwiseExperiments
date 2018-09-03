#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include <instruction_table.h>
#include <common.c>
#include <lexer.c>

// AST
typedef enum expression_type {
    Expression_Int,
    Expression_Unary,
    Expression_Binary,
} expression_type;

typedef struct expression {
    expression_type Type;
    union {
        uint32_t IntValue;

        struct {
            token_type Op;
            struct expression *Expr;
        } Unary;

        struct {
            token_type Op;
            struct expression *Lhs;
            struct expression *Rhs;
        } Binary;
    };
} expression;

static expression *expressionNew(expression_type Type) {
    expression *Result = malloc(sizeof(*Result));
    Result->Type = Type;
    return Result;
}

static expression *expressionIntNew(uint32_t Value) {
    expression *Result = expressionNew(Expression_Int);
    Result->IntValue = Value;
    return Result;
}

static expression *expressionUnaryNew(token_type Op, expression *Expr) {
    expression *Result = expressionNew(Expression_Unary);
    Result->Unary.Op = Op;
    Result->Unary.Expr = Expr;
    return Result;
}

static expression *expressionBinaryNew(token_type Op, expression *Lhs, expression *Rhs) {
    expression *Result = expressionNew(Expression_Binary);
    Result->Binary.Op = Op;
    Result->Binary.Lhs = Lhs;
    Result->Binary.Rhs = Rhs;
    return Result;
}


// Parser
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

static expression *parse(int);

static expression *parseUnary(void) {
    expression *Result;

    if(isUnaryOp()) {
        token_type Op = Token.Type;

        nextToken();
        Result = expressionUnaryNew(Op, parse(Table[Op].Precedence));
    }
    else if(matchToken(Token_LParen)) {
        Result = parse(0);
        expectToken(Token_RParen);
    }
    else if(Token.Type == Token_Int) {
        Result = expressionIntNew(Token.IntValue);
        nextToken();
    }
    else {
        parseError("No expected token available.\n");
        exit(1);
    }

    return Result;
}

static expression *parse(int Precedence) {
    expression *Result = parseUnary();

    while(Table[Token.Type].Precedence >= Precedence &&
          (Token.Type != Token_EOF && Token.Type != Token_RParen))
    {
        if(!isBinaryOp()) {
            parseError("Missing expected binary operator.\n");
            exit(1);
        }

        token_type Op = Token.Type;

        nextToken();
        expression *Rhs;
        if(Table[Op].Associativity == Assoc_Left) {
            Rhs = parse(Table[Op].Precedence + 1);
        } else {
            assert(Table[Op].Associativity == Assoc_Right);
            Rhs = parse(Table[Op].Precedence);
        }

        Result = expressionBinaryNew(Op, Result, Rhs);
    }

    return Result;
}


// Binary generator
#define caseInstr(C, I) case C: { Instr = I; } break;
static void printBinary(FILE *File, expression *Node) {
    switch(Node->Type) {
        case Expression_Int: {
            uint8_t Data[] = {
                LIT,
                (Node->IntValue >>  0) & 0xFF,
                (Node->IntValue >>  8) & 0xFF,
                (Node->IntValue >> 16) & 0xFF,
                (Node->IntValue >> 24) & 0xFF,
            };
            fwrite(Data, 1, sizeof(Data), File);
        } break;

        case Expression_Unary: {
            printBinary(File, Node->Unary.Expr);

            uint8_t Instr = NOP;
            switch(Node->Unary.Op) {
                case Token_UnaryPlus: {} break;
                caseInstr(Token_UnaryMinus, SYM);
                caseInstr(Token_BitNot, NOT);

                InvalidDefaultCase;
            }

            if(Instr != NOP) {
                fwrite(&Instr, 1, sizeof(Instr), File);
            }
        } break;

        case Expression_Binary: {
            printBinary(File, Node->Binary.Lhs);
            printBinary(File, Node->Binary.Rhs);

            uint8_t Instr = NOP;
            switch(Node->Binary.Op) {
                caseInstr(Token_Add,      ADD);
                caseInstr(Token_Subtract, SUB);
                caseInstr(Token_BitOr,    OR);
                caseInstr(Token_BitXor,   XOR);
                caseInstr(Token_Multiply, MUL);
                caseInstr(Token_Divide,   DIV);
                caseInstr(Token_Mod,      MOD);
                caseInstr(Token_LShift,   LSH);
                caseInstr(Token_RShift,   RSH);
                caseInstr(Token_BitAnd,   AND);
                caseInstr(Token_Power,    POW);

                InvalidDefaultCase;
            }

            assert(Instr != NOP);
            fwrite(&Instr, 1, sizeof(Instr), File);
        } break;
    }
}

static void outputBinary(char *Path, expression *Ast) {
    FILE *File = fopen(Path, "wb");

    printBinary(File, Ast);
    fwrite((uint8_t[]){HALT}, 1, 1, File);

    fclose(File);
}

int main(int ArgCount, char *ArgVal[]) {
    if(ArgCount != 3) {
        fprintf(stderr, "Usage: %s EXPR OUTPUT\n", ArgVal[0]);
        exit(1);
    }

    Stream = ArgVal[1];
    nextToken();
    expression *Ast = parse(0);

    outputBinary(ArgVal[2], Ast);

    return 0;
}
