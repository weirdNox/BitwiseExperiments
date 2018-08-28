typedef enum expr_type {
    Expr_None,
    Expr_Int,
    Expr_Unary,
    Expr_Binary,
} expr_type;

typedef struct expr_unary {
    token_type Op;
    struct expr *Expr;
} expr_unary;

typedef struct expr_binary {
    token_type Op;
    struct expr *Left;
    struct expr *Right;
} expr_binary;

typedef struct expr {
    expr_type Type;
    union {
        uint64_t IntValue;
        expr_unary Unary;
        expr_binary Binary;
    };
} expr;

static arena astArena;

static void *astAlloc(size_t Size) {
    assert(Size != 0);
    void *Result = arenaAlloc(&astArena, Size);
    memset(Result, 0, Size);
    return Result;
}

static expr *exprInt(uint64_t Value) {
    expr *Result = astAlloc(sizeof(expr));
    Result->Type = Expr_Int;
    Result->IntValue = Value;

    return Result;
}

static expr *exprUnary(token_type Op, expr *Expr) {
    expr *Result = astAlloc(sizeof(expr));
    Result->Type = Expr_Unary;
    Result->Unary.Op = Op;
    Result->Unary.Expr = Expr;

    return Result;
}

static expr *exprBinary(token_type Op, expr *Left, expr *Right) {
    expr *Result = astAlloc(sizeof(expr));
    Result->Type = Expr_Binary;
    Result->Binary.Op = Op;
    Result->Binary.Left = Left;
    Result->Binary.Right = Right;

    return Result;
}
