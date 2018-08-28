typedef enum token_type {
    Token_Unknown,
    Token_EOF,

    Token_Add,
    Token_Subtract,
    Token_BitOr,
    Token_BitXor,

    Token_Multiply,
    Token_Divide,
    Token_Mod,
    Token_LShift,
    Token_RShift,
    Token_BitAnd,

    Token_Power,

    Token_UnaryPlus,
    Token_UnaryMinus,
    Token_BitNot,

    Token_LParen,
    Token_RParen,

    Token_Int,

    Token_Count
} token_type;

typedef struct token {
    token_type Type;
    union {
        uint32_t IntValue;
    };
} token;

static char *Stream;
static token Token;

static int CharToDigit[256] = {
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

#define case1(C, Type1)                         \
    case (C): {                                 \
        Token.Type = (Type1); ++Stream;         \
    } break

#define case2(C, Type2)                         \
    case (C): {                                 \
        if(*(Stream+1) == (C)) {                \
            Token.Type = (Type2);               \
        } else {                                \
            Token.Type = Token_Unknown;         \
        }                                       \
        Stream += 2;                            \
    } break

#define case1and2(C, Type1, Type2)              \
    case (C): {                                 \
        ++Stream;                               \
        if(*Stream == (C)) {                    \
            Token.Type = (Type2);               \
            ++Stream;                           \
        } else {                                \
            Token.Type = (Type1);               \
        }                                       \
    } break

static void nextToken() {
    while(*Stream == ' ' || *Stream == '\n' || *Stream == '\t' || *Stream == '\r' || *Stream == '\v') {
        ++Stream;
    }

    switch(*Stream) {
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
        {
            Token.Type = Token_Int;

            int Base = 10;
            if(*Stream == '0') {
                switch(*(Stream+1)) {
                    case 'b': case 'B': {
                        Base = 2;
                        Stream += 2;
                    } break;

                    case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': {
                        Base = 8;
                        ++Stream;
                    } break;

                    case 'x': case 'X': {
                        Base = 16;
                        Stream += 2;
                    } break;
                }
            }

            int Digit;
            uint32_t Value = 0;
            bool Overflow = false;
            while((Digit = CharToDigit[(int)*Stream]) || *Stream == '0') {
                if(Digit >= Base) {
                    parseError("Digit is greater than base.\n");
                    Digit = 0;
                }

                if(!Overflow && Value > (UINT32_MAX - Digit)/Base) {
                    parseError("Integer overflow.\n");
                    Value = 0;
                    Overflow = true;
                }

                if(!Overflow) {
                    Value = Value * Base + Digit;
                }

                ++Stream;
            }

            Token.IntValue = Value;
        } break;

        case1('\0', Token_EOF);

        case1('+', Token_Add);
        case1('-', Token_Subtract);
        case1('|', Token_BitOr);
        case1('^', Token_BitXor);

        case1and2('*', Token_Multiply, Token_Power);
        case1('/', Token_Divide);
        case1('%', Token_Mod);
        case2('<', Token_LShift);
        case2('>', Token_RShift);
        case1('&', Token_BitAnd);

        case1('~', Token_BitNot);

        case1('(', Token_LParen);
        case1(')', Token_RParen);

        default: {
            Token.Type = Token_Unknown;
            ++Stream;
        } break;
    }
}

static bool matchToken(token_type Type) {
    if(Token.Type == Type) {
        nextToken();
        return true;
    }

    return false;
}

static bool expectToken(token_type Type) {
    if(Token.Type == Type) {
        nextToken();
        return true;
    }

    parseError("Token is not of the expected kind!\n");
    exit(1);
    return false;
}
