static void *xRealloc(void *Addr, size_t Size);

typedef struct {
    size_t Length;
    size_t Capacity;
    char Buffer[];
} buffer_header;

#define bufHeader_(b) ((buffer_header *)(b) - 1)

#define bufLength(b) ((b) ? (bufHeader_(b)->Length) : 0)
#define bufCapacity(b) ((b) ? (bufHeader_(b)->Capacity) : 0)
#define bufEnd(b) ((b) ? ((b) + bufHeader_(b)->Length) : 0)
#define bufFree(b) ((b) ? (free(bufHeader_(b)), (b)=0) : 0)
#define bufFit(b, n) ((n) > bufCapacity(b) ? ((b) = bufGrow((b), (n), sizeof(*(b)))) : 0)
#define bufPush(b, ...) (bufFit((b), 1+bufLength(b)), (b)[bufHeader_(b)->Length++] = (__VA_ARGS__))

static void *bufGrow(void *Buffer, size_t NewLength, size_t ElementSize) {
    size_t NewCapacity = max(16, max(2*bufCapacity(Buffer), NewLength));
    assert(NewLength <= NewCapacity);
    assert(NewCapacity <= (SIZE_MAX - offsetof(buffer_header, Buffer)));

    size_t NewSize = sizeof(buffer_header) + NewCapacity*ElementSize;
    buffer_header *Header = xRealloc(Buffer ? bufHeader_(Buffer) : 0, NewSize);
    if(!Buffer) {
        Header->Length = 0;
    }

    Header->Capacity = NewCapacity;
    return Header->Buffer;
}

static void bufferTest() {
    int *IntBuffer = 0;
    size_t Max = 2500;
    for(size_t I = 0; I < Max; ++I) {
        bufPush(IntBuffer, I);
    }
    assert(bufLength(IntBuffer) == Max);
    for(size_t I = 0; I < Max; ++I) {
        assert(IntBuffer[I] == (int)I);
    }
    bufFree(IntBuffer);
    assert(IntBuffer == 0);
    assert(bufLength(IntBuffer) == 0);
    assert(bufCapacity(IntBuffer) == 0);
}
