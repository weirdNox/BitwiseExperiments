#define max(a, b) ((a) > (b) ? (a) : (b))
#define arrayCount(arr) (sizeof(arr)/sizeof(*arr))

// NOTE(nox): Only for m = 2^k
#define alignDown(n, m) ((n) & ~((m)-1))
#define alignUp(n, m) alignDown((n)+(m)-1, (m))
#define alignPointerDown(n, m) (void *)((uintptr_t)(n) & ~((m)-1))
#define alignPointerUp(n, m) (void *)alignDown((uintptr_t)(n)+(m)-1, (m))

static void syntaxError(char *Fmt, ...) {
    va_list Args;
    va_start(Args, Fmt);
    printf("Syntax error: ");
    vprintf(Fmt, Args);
    printf("\n");
    va_end(Args);
}

static void fatalSyntaxError(char *Fmt, ...) {
    va_list Args;
    va_start(Args, Fmt);
    printf("Syntax error: ");
    vprintf(Fmt, Args);
    printf("\n");
    va_end(Args);
    exit(-1);
}
