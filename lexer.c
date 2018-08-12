typedef enum token_type {
    Token_EOF = 0,
    // NOTE(nox): Reserved tokens for ASCII
    Token_LastChar = 127,
    Token_Identifier,
    Token_Int,
    Token_LeftShift,
    Token_RightShift,
    Token_Pow,
} token_type;

typedef struct {
    token_type Type;
    char *Start;
    char *End;
    union {
        uint64_t Value;
    };
} token;

static char *Stream;
static token Token;

static int CharToDigitLookup[256] = {
    ['0'] = 0,
    ['1'] = 1,
    ['2'] = 2,
    ['3'] = 3,
    ['4'] = 4,
    ['5'] = 5,
    ['6'] = 6,
    ['7'] = 7,
    ['8'] = 8,
    ['9'] = 9,
    ['a'] = 10, ['A'] = 10,
    ['b'] = 11, ['B'] = 11,
    ['c'] = 12, ['C'] = 12,
    ['d'] = 13, ['D'] = 13,
    ['e'] = 14, ['E'] = 14,
    ['f'] = 15, ['F'] = 15,
};

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
            Token.Type = Token_Identifier;
            ++Stream;
            while(isalnum(*Stream) || *Stream == '_') {
                ++Stream;
            }
        } break;

        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
        {
            Token.Type = Token_Int;
            uint64_t Base = 10;
            if(*Stream == '0') {
                switch(*(++Stream)) {
                    case 'b': case 'B': {
                        Base = 2;
                        ++Stream;
                    } break;

                    case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7':
                    {
                        Base = 8;
                    } break;

                    case 'x': case 'X': {
                        Base = 16;
                        ++Stream;
                    } break;

                    default: {
                        syntaxError("Wrong integer prefix 0%c. Assuming decimal...", *Stream);
                        ++Stream;
                    } break;
                }
            }

            Token.Value = 0;
            for(;;) {
                uint64_t Digit = CharToDigitLookup[(int)*Stream];
                if(Digit == 0 && *Stream != '0') {
                    break;
                }

                if(Digit >= Base) {
                    syntaxError("Digit '%c' out of range for base %ld", *Stream, Base);
                    Digit = 0;
                }

                if(Token.Value > (UINT64_MAX - Digit)/Base) {
                    syntaxError("Integer literal overflow", *Stream, Base);
                    while(Digit != 0 || *Stream == '0') {
                        ++Stream;
                    }
                    Token.Value = 0;
                    break;
                }

                Token.Value = Base*Token.Value + Digit;
                ++Stream;
            }
        } break;

        case '<': {
            ++Stream;
            if(*Stream != '<') {
                syntaxError("Missing <. Assuming <<...");
            } else {
                ++Stream;
            }
            Token.Type = Token_LeftShift;
        } break;

        case '>': {
            ++Stream;
            if(*Stream != '>') {
                syntaxError("Missing >. Assuming >>...");
            } else {
                ++Stream;
            }
            Token.Type = Token_RightShift;
        } break;

        case '*': {
            ++Stream;
            if(*Stream == '*') {
                ++Stream;
                Token.Type = Token_Pow;
            } else {
                Token.Type = '*';
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

static inline bool isToken(token_type Type) {
    return (Token.Type == Type);
}

static bool matchToken(token_type Type) {
    if(isToken(Type)) {
        nextToken();
        return true;
    } else {
        return false;
    }
}

static bool expectToken(token_type Type) {
    if(isToken(Type)) {
        nextToken();
        return true;
    } else {
        fatalSyntaxError("Expected different token");
        return false;
    }
}

#define assertToken(Val) assert(matchToken(Val))
#define assertInt(Val) assert((Val) == Token.Value && matchToken(Token_Int))

static void lexTest() {
    initStream("-1+0x34+0123");
    assertToken('-');
    assertInt(1);
    assertToken('+');
    assertInt(0x34);
    assertToken('+');
    assertInt(0123);
    assertToken(Token_EOF);

    initStream("8349>>1839<<1983");
    assertInt(8349);
    assertToken(Token_RightShift);
    assertInt(1839);
    assertToken(Token_LeftShift);
    assertInt(1983);
    assertToken(Token_EOF);
}

#undef assertToken
#undef assertInt
