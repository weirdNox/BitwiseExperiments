static void *xMalloc(size_t Size) {
    void *Result = malloc(Size);
    if(!Result) {
        perror("xMalloc failed");
        exit(-1);
    }
    return Result;
}

static void *xRealloc(void *Addr, size_t Size) {
    void *Result = realloc(Addr, Size);
    if(!Result) {
        perror("xRealloc failed");
        exit(-1);
    }
    return Result;
}

typedef struct arena {
    char *Next;
    char *End;
    char **Blocks;
} arena;

#define ARENA_ALIGNMENT 1<<3
#define ARENA_BLOCK_SIZE 1024

static void arenaGrow(arena *Arena, size_t MinimumSize) {
    size_t Size = alignUp(max(MinimumSize, ARENA_BLOCK_SIZE), ARENA_ALIGNMENT);
    Arena->Next = xMalloc(Size);
    Arena->End = Arena->Next + Size;
    bufPush(Arena->Blocks, Arena->Next);
}

static void *arenaAlloc(arena *Arena, size_t Size) {
    if(Size > (size_t)(Arena->End - Arena->Next)) {
        arenaGrow(Arena, Size);
    }
    void *Result = Arena->Next;
    Arena->Next = alignPointerUp(Arena->Next + Size, ARENA_ALIGNMENT);

    return Result;
}

static void arenaFree(arena *Arena) {
    for(char **Iter = Arena->Blocks; Iter != bufEnd(Arena->Blocks); ++Iter) {
        free(*Iter);
    }
    *Arena = (arena){};
}
