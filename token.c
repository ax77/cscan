#include "token.h"

static int initialized = 0;

void init_tokens_bt()
{
    if (initialized) {
        //error
    }
    initialized = 1;

    TOKEN_EOF = &(struct toktype ) { .tp = "" };
    TOKEN_ERROR = &(struct toktype ) { .tp = "" };
    TOKEN_STREAMBEGIN = &(struct toktype ) { .tp = "" };
    TOKEN_STREAMEND = &(struct toktype ) { .tp = "" };
    TOKEN_IDENT = &(struct toktype ) { .tp = "" };
    TOKEN_NUMBER = &(struct toktype ) { .tp = "" };
    TOKEN_CHAR = &(struct toktype ) { .tp = "" };
    TOKEN_STRING = &(struct toktype ) { .tp = "" };
    T_SPEC_PLACEMARKER = &(struct toktype ) { .tp = "" };
    T_SPEC_UNHIDE = &(struct toktype ) { .tp = "" };
    T_ARROW = &(struct toktype ) { .tp = "->" };
    T_MINUS_MINUS = &(struct toktype ) { .tp = "--" };
    T_MINUS_EQUAL = &(struct toktype ) { .tp = "-=" };
    T_NE = &(struct toktype ) { .tp = "!=" };
    T_DOT_DOT = &(struct toktype ) { .tp = ".." };
    T_TIMES_EQUAL = &(struct toktype ) { .tp = "*=" };
    T_DIVIDE_EQUAL = &(struct toktype ) { .tp = "/=" };
    T_AND_EQUAL = &(struct toktype ) { .tp = "&=" };
    T_AND_AND = &(struct toktype ) { .tp = "&&" };
    T_SHARP_SHARP = &(struct toktype ) { .tp = "##" };
    T_PERCENT_EQUAL = &(struct toktype ) { .tp = "%=" };
    T_XOR_EQUAL = &(struct toktype ) { .tp = "^=" };
    T_PLUS_PLUS = &(struct toktype ) { .tp = "++" };
    T_PLUS_EQUAL = &(struct toktype ) { .tp = "+=" };
    T_LE = &(struct toktype ) { .tp = "<=" };
    T_LSHIFT = &(struct toktype ) { .tp = "<<" };
    T_EQ = &(struct toktype ) { .tp = "==" };
    T_GE = &(struct toktype ) { .tp = ">=" };
    T_RSHIFT = &(struct toktype ) { .tp = ">>" };
    T_OR_OR = &(struct toktype ) { .tp = "||" };
    T_OR_EQUAL = &(struct toktype ) { .tp = "|=" };
    T_COMMA = &(struct toktype ) { .tp = "," };
    T_MINUS = &(struct toktype ) { .tp = "-" };
    T_SEMI_COLON = &(struct toktype ) { .tp = ";" };
    T_COLON = &(struct toktype ) { .tp = ":" };
    T_EXCLAMATION = &(struct toktype ) { .tp = "!" };
    T_QUESTION = &(struct toktype ) { .tp = "?" };
    T_DOT = &(struct toktype ) { .tp = "." };
    T_LEFT_PAREN = &(struct toktype ) { .tp = "(" };
    T_RIGHT_PAREN = &(struct toktype ) { .tp = ")" };
    T_LEFT_BRACKET = &(struct toktype ) { .tp = "[" };
    T_RIGHT_BRACKET = &(struct toktype ) { .tp = "]" };
    T_LEFT_BRACE = &(struct toktype ) { .tp = "{" };
    T_RIGHT_BRACE = &(struct toktype ) { .tp = "}" };
    T_TIMES = &(struct toktype ) { .tp = "*" };
    T_DIVIDE = &(struct toktype ) { .tp = "/" };
    T_AND = &(struct toktype ) { .tp = "&" };
    T_SHARP = &(struct toktype ) { .tp = "#" };
    T_PERCENT = &(struct toktype ) { .tp = "%" };
    T_XOR = &(struct toktype ) { .tp = "^" };
    T_PLUS = &(struct toktype ) { .tp = "+" };
    T_LT = &(struct toktype ) { .tp = "<" };
    T_ASSIGN = &(struct toktype ) { .tp = "=" };
    T_GT = &(struct toktype ) { .tp = ">" };
    T_OR = &(struct toktype ) { .tp = "|" };
    T_TILDE = &(struct toktype ) { .tp = "~" };
    T_DOT_DOT_DOT = &(struct toktype ) { .tp = "..." };
    T_LSHIFT_EQUAL = &(struct toktype ) { .tp = ">>=" };
    T_RSHIFT_EQUAL = &(struct toktype ) { .tp = "<<=" };
    T_DOLLAR_SIGN = &(struct toktype ) { .tp = "$" };
    T_AT_SIGN = &(struct toktype ) { .tp = "@" };
    T_GRAVE_ACCENT = &(struct toktype ) { .tp = "`" };
    T_BACKSLASH = &(struct toktype ) { .tp = "\\" };

}
