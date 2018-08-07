#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define max(a, b) ((a) > (b) ? (a) : (b))

static void *xRealloc(void *Addr, size_t Size) {
    void *Return = realloc(Addr, Size);
    if(!Return) {
        perror("xRealloc failed");
        exit(-1);
    }
    return Return;
}

typedef struct {
    size_t Length;
    size_t Capacity;
    char Buffer[];
} buffer_header;

#define bufHeader_(b) ((buffer_header *)(b) - 1)

#define bufLength(b) ((b) ? (bufHeader_(b)->Length) : 0)
#define bufCapacity(b) ((b) ? (bufHeader_(b)->Capacity) : 0)
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

typedef enum {
    Token_EOF = 0,
    // Reserved tokens for ASCII
    Token_LastChar = 127,
    Token_Int,
} token_type;

typedef struct {
    token_type Type;
    char *Start;
    char *End;
    union {
        uint64_t Value;
    };
} token;

static token Token;
static char *Stream;

static void nextToken() {
    while(*Stream == ' ' || *Stream == '\n' || *Stream == '\t' || *Stream == '\r' || *Stream == '\v') {
        ++Stream;
    }

    Token.Start = Stream;

    switch(*Stream) {
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
        case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
        case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
        case 'V': case 'W': case 'X': case 'Y': case 'Z': case 'a': case 'b':
        case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i':
        case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p':
        case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w':
        case 'x': case 'y': case 'z': case '_':
        {
            ++Stream;
            while(isalnum(*Stream) || *Stream == '_') {
                ++Stream;
            }
        } break;

        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
        {
            Token.Type = Token_Int;
            Token.Value = *Stream - '0';
            while(isdigit(*++Stream)) {
                Token.Value = 10*Token.Value + *Stream - '0';
            }
        } break;

        default: {
            Token.Type = *Stream;
            ++Stream;
        } break;
    }

    Token.End = Stream;
}

static void initStream(char *String) {
    Stream = String;
    nextToken();
}

static bool matchToken(token_type Type) {
    if(Token.Type == Type) {
        nextToken();
        return true;
    } else {
        return false;
    }
}

#define assertToken(Val) assert(matchToken(Val))
#define assertInt(Val) assert((Val) == Token.Value && matchToken(Token_Int))

static void lexTest() {
    initStream("-1+34+123");
    assertToken('-');
    assertInt(1);
    assertToken('+');
    assertInt(34);
    assertToken('+');
    assertInt(123);
}

#undef assertToken
#undef assertInt

int main() {
    bufferTest();
    lexTest();

    return 0;
}
