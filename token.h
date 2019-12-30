#ifndef TOKEN_H_
#define TOKEN_H_

void init_tokens_bt();

struct ident;

struct toktype {
    char tp[8];
};

struct token {
    int fcat, fpos;
    struct toktype *type;
    union {
        struct ident *ident;
        char *cstring;
    };
};

struct toktype * TOKEN_EOF;
struct toktype * TOKEN_ERROR;
struct toktype * TOKEN_STREAMBEGIN;
struct toktype * TOKEN_STREAMEND;
struct toktype * TOKEN_IDENT;
struct toktype * TOKEN_NUMBER;
struct toktype * TOKEN_CHAR;
struct toktype * TOKEN_STRING;
struct toktype * T_SPEC_PLACEMARKER;
struct toktype * T_SPEC_UNHIDE;
struct toktype * T_ARROW;
struct toktype * T_MINUS_MINUS;
struct toktype * T_MINUS_EQUAL;
struct toktype * T_NE;
struct toktype * T_DOT_DOT;
struct toktype * T_TIMES_EQUAL;
struct toktype * T_DIVIDE_EQUAL;
struct toktype * T_AND_EQUAL;
struct toktype * T_AND_AND;
struct toktype * T_SHARP_SHARP;
struct toktype * T_PERCENT_EQUAL;
struct toktype * T_XOR_EQUAL;
struct toktype * T_PLUS_PLUS;
struct toktype * T_PLUS_EQUAL;
struct toktype * T_LE;
struct toktype * T_LSHIFT;
struct toktype * T_EQ;
struct toktype * T_GE;
struct toktype * T_RSHIFT;
struct toktype * T_OR_OR;
struct toktype * T_OR_EQUAL;
struct toktype * T_COMMA;
struct toktype * T_MINUS;
struct toktype * T_SEMI_COLON;
struct toktype * T_COLON;
struct toktype * T_EXCLAMATION;
struct toktype * T_QUESTION;
struct toktype * T_DOT;
struct toktype * T_LEFT_PAREN;
struct toktype * T_RIGHT_PAREN;
struct toktype * T_LEFT_BRACKET;
struct toktype * T_RIGHT_BRACKET;
struct toktype * T_LEFT_BRACE;
struct toktype * T_RIGHT_BRACE;
struct toktype * T_TIMES;
struct toktype * T_DIVIDE;
struct toktype * T_AND;
struct toktype * T_SHARP;
struct toktype * T_PERCENT;
struct toktype * T_XOR;
struct toktype * T_PLUS;
struct toktype * T_LT;
struct toktype * T_ASSIGN;
struct toktype * T_GT;
struct toktype * T_OR;
struct toktype * T_TILDE;
struct toktype * T_DOT_DOT_DOT;
struct toktype * T_LSHIFT_EQUAL;
struct toktype * T_RSHIFT_EQUAL;
struct toktype * T_DOLLAR_SIGN;
struct toktype * T_AT_SIGN;
struct toktype * T_GRAVE_ACCENT;
struct toktype * T_BACKSLASH;

#endif /* TOKEN_H_ */
