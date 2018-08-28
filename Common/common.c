#define arrayCount(A) sizeof(A)/sizeof(*A)

#define InvalidCodePath assert(!"InvalidCodePath")
#define InvalidDefaultCase default: { InvalidCodePath; } break

static void parseError(char *Format, ...) {
    va_list Args;
    va_start(Args, Format);
    fprintf(stderr, "Parsing error: ");
    vfprintf(stderr, Format, Args);
    fprintf(stderr, "\n");
    va_end(Args);
}

static void fatalError(char *Fmt, ...) {
    va_list Args;
    va_start(Args, Fmt);
    fprintf(stderr, "Fatal error: ");
    vfprintf(stderr, Fmt, Args);
    fprintf(stderr, "\n");
    va_end(Args);
    exit(1);
}

static void *xMalloc(size_t NumBytes) {
    void *Result = malloc(NumBytes);
    if(!Result) {
        fprintf(stderr, "Insufficient space available\n");
        exit(1);
    }

    return Result;
}

static uint8_t *readEntireFile(char *Path) {
    FILE *File = fopen(Path, "rb");
    if(!File) {
        fprintf(stderr, "Error opening file %s", Path);
        exit(1);
    }

    fseek(File, 0, SEEK_END);
    uint32_t NumBytes = ftell(File);
    uint8_t *Buffer = xMalloc(NumBytes+1);

    fseek(File, 0, SEEK_SET);
    fread(Buffer, 1, NumBytes, File);
    fclose(File);
    Buffer[NumBytes] = 0;

    return Buffer;
}
