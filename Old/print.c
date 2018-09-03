static void printExpr(expr *Expr) {
    switch(Expr->Type) {
        case Expr_Int: {
            printf("%ld", Expr->IntValue);
        } break;

        case Expr_Unary: {
            printf("(%c ", Expr->Unary.Op);
            printExpr(Expr->Unary.Expr);
            printf(")");
        } break;

        case Expr_Binary: {
            printf("(%c ", Expr->Binary.Op);
            printExpr(Expr->Binary.Left);
            printf(" ");
            printExpr(Expr->Binary.Right);
            printf(")");
        } break;

        default: {
            perror("Unexpected expression type");
            exit(-1);
        } break;
    }
}

static void parsedPrintTest(void) {
    initStream("-1+2*(3+4)");
    printExpr(parseExpr());
    printf("\n");
    initStream("12*34 + 45/56 + ~25");
    printExpr(parseExpr());
    printf("\n");
    initStream("12**1235**-2+1+4+2");
    printExpr(parseExpr());
    printf("\n");
}
