static expr *parseExpr();

static expr *parseBaseExpr() {
    expr *Result;
    switch((int)Token.Type) {
        case Token_Int: {
            uint64_t Value = Token.Value;
            nextToken();
            Result = exprInt(Value);
        } break;

        case '(': {
            nextToken();
            Result = parseExpr();
            expectToken(')');
        } break;

        default: {
            fatalSyntaxError("Unexpected token type while parsing base expression");
        } break;
    }

    return Result;
}

static bool isUnaryOp() {
    return isToken('~') || isToken('-') || isToken('+');
}

static expr *parseUnaryExpr() {
    expr *Result;
    if(isUnaryOp()) {
        token_type Op = Token.Type;
        nextToken();
        Result = exprUnary(Op, parseUnaryExpr());
    } else {
        Result = parseBaseExpr();
    }

    return Result;
}

static expr *parsePowExpr() {
    expr *Result = parseUnaryExpr();
    if(matchToken(Token_Pow)) {
        Result = exprBinary(Token_Pow, Result, parsePowExpr());
    }

    return Result;
}

static bool isMulOp() {
    return (isToken('*') || isToken('/') || isToken('%') || isToken(Token_LeftShift) ||
            isToken(Token_RightShift));
}

static expr *parseMulExpr() {
    expr *Result = parsePowExpr();
    while(isMulOp()) {
        token_type Op = Token.Type;
        nextToken();
        Result = exprBinary(Op, Result, parsePowExpr());
    }

    return Result;
}

static bool isAddOp() {
    return isToken('+') || isToken('-') || isToken('|') || isToken('^');
}

static expr *parseAddExpr() {
    expr *Result = parseMulExpr();
    while(isAddOp()) {
        token_type Op = Token.Type;
        nextToken();
        Result = exprBinary(Op, Result, parseMulExpr());
    }

    return Result;
}

static expr *parseExpr() {
    return parseAddExpr();
}
