#include "drcc.h"

vec_impl(struct Token*, token);

Token *EOF_TOKEN_ENTRY = &(Token ) { .type = TOKEN_EOF, .value = "eof" };

Token *token_new(T type, char *value)
{
    Token *t = cc_malloc(sizeof(struct Token));
    t->type = type;
    t->value = cc_strdup(value);
    return t;
}

// Builtin-names
//
Ident *auto_ident = &(Ident ) { .name = "auto", .ns = 0, .sym = NULL };
Ident *break_ident = &(Ident ) { .name = "break", .ns = 0, .sym = NULL };
Ident *case_ident = &(Ident ) { .name = "case", .ns = 0, .sym = NULL };
Ident *char_ident = &(Ident ) { .name = "char", .ns = 0, .sym = NULL };
Ident *const_ident = &(Ident ) { .name = "const", .ns = 0, .sym = NULL };
Ident *continue_ident = &(Ident ) { .name = "continue", .ns = 0, .sym = NULL };
Ident *default_ident = &(Ident ) { .name = "default", .ns = 0, .sym = NULL };
Ident *do_ident = &(Ident ) { .name = "do", .ns = 0, .sym = NULL };
Ident *double_ident = &(Ident ) { .name = "double", .ns = 0, .sym = NULL };
Ident *else_ident = &(Ident ) { .name = "else", .ns = 0, .sym = NULL };
Ident *enum_ident = &(Ident ) { .name = "enum", .ns = 0, .sym = NULL };
Ident *extern_ident = &(Ident ) { .name = "extern", .ns = 0, .sym = NULL };
Ident *float_ident = &(Ident ) { .name = "float", .ns = 0, .sym = NULL };
Ident *for_ident = &(Ident ) { .name = "for", .ns = 0, .sym = NULL };
Ident *goto_ident = &(Ident ) { .name = "goto", .ns = 0, .sym = NULL };
Ident *if_ident = &(Ident ) { .name = "if", .ns = 0, .sym = NULL };
Ident *inline_ident = &(Ident ) { .name = "inline", .ns = 0, .sym = NULL };
Ident *int_ident = &(Ident ) { .name = "int", .ns = 0, .sym = NULL };
Ident *long_ident = &(Ident ) { .name = "long", .ns = 0, .sym = NULL };
Ident *register_ident = &(Ident ) { .name = "register", .ns = 0, .sym = NULL };
Ident *restrict_ident = &(Ident ) { .name = "restrict", .ns = 0, .sym = NULL };
Ident *return_ident = &(Ident ) { .name = "return", .ns = 0, .sym = NULL };
Ident *short_ident = &(Ident ) { .name = "short", .ns = 0, .sym = NULL };
Ident *signed_ident = &(Ident ) { .name = "signed", .ns = 0, .sym = NULL };
Ident *sizeof_ident = &(Ident ) { .name = "sizeof", .ns = 0, .sym = NULL };
Ident *static_ident = &(Ident ) { .name = "static", .ns = 0, .sym = NULL };
Ident *struct_ident = &(Ident ) { .name = "struct", .ns = 0, .sym = NULL };
Ident *switch_ident = &(Ident ) { .name = "switch", .ns = 0, .sym = NULL };
Ident *typedef_ident = &(Ident ) { .name = "typedef", .ns = 0, .sym = NULL };
Ident *union_ident = &(Ident ) { .name = "union", .ns = 0, .sym = NULL };
Ident *unsigned_ident = &(Ident ) { .name = "unsigned", .ns = 0, .sym = NULL };
Ident *void_ident = &(Ident ) { .name = "void", .ns = 0, .sym = NULL };
Ident *volatile_ident = &(Ident ) { .name = "volatile", .ns = 0, .sym = NULL };
Ident *while_ident = &(Ident ) { .name = "while", .ns = 0, .sym = NULL };
Ident *_Alignas_ident = &(Ident ) { .name = "_Alignas", .ns = 0, .sym = NULL };
Ident *_Alignof_ident = &(Ident ) { .name = "_Alignof", .ns = 0, .sym = NULL };
Ident *_Atomic_ident = &(Ident ) { .name = "_Atomic", .ns = 0, .sym = NULL };
Ident *_Bool_ident = &(Ident ) { .name = "_Bool", .ns = 0, .sym = NULL };
Ident *_Complex_ident = &(Ident ) { .name = "_Complex", .ns = 0, .sym = NULL };
Ident *_Decimal128_ident = &(Ident ) { .name = "_Decimal128", .ns = 0, .sym = NULL };
Ident *_Decimal32_ident = &(Ident ) { .name = "_Decimal32", .ns = 0, .sym = NULL };
Ident *_Decimal64_ident = &(Ident ) { .name = "_Decimal64", .ns = 0, .sym = NULL };
Ident *_Generic_ident = &(Ident ) { .name = "_Generic", .ns = 0, .sym = NULL };
Ident *_Imaginary_ident = &(Ident ) { .name = "_Imaginary", .ns = 0, .sym = NULL };
Ident *_Noreturn_ident = &(Ident ) { .name = "_Noreturn", .ns = 0, .sym = NULL };
Ident *_Static_assert_ident = &(Ident ) { .name = "_Static_assert", .ns = 0, .sym = NULL };
Ident *_Thread_local_ident = &(Ident ) { .name = "_Thread_local", .ns = 0, .sym = NULL };
Ident *asm_ident = &(Ident ) { .name = "asm", .ns = 0, .sym = NULL };
Ident *__asm_ident = &(Ident ) { .name = "__asm", .ns = 0, .sym = NULL };
Ident *__asm___ident = &(Ident ) { .name = "__asm__", .ns = 0, .sym = NULL };
Ident *__alignof_ident = &(Ident ) { .name = "__alignof", .ns = 0, .sym = NULL };
Ident *__alignof___ident = &(Ident ) { .name = "__alignof__", .ns = 0, .sym = NULL };
Ident *__attribute_ident = &(Ident ) { .name = "__attribute", .ns = 0, .sym = NULL };
Ident *__attribute___ident = &(Ident ) { .name = "__attribute__", .ns = 0, .sym = NULL };
Ident *__complex_ident = &(Ident ) { .name = "__complex", .ns = 0, .sym = NULL };
Ident *__complex___ident = &(Ident ) { .name = "__complex__", .ns = 0, .sym = NULL };
Ident *__const_ident = &(Ident ) { .name = "__const", .ns = 0, .sym = NULL };
Ident *__const___ident = &(Ident ) { .name = "__const__", .ns = 0, .sym = NULL };
Ident *__inline_ident = &(Ident ) { .name = "__inline", .ns = 0, .sym = NULL };
Ident *__inline___ident = &(Ident ) { .name = "__inline__", .ns = 0, .sym = NULL };
Ident *__restrict_ident = &(Ident ) { .name = "__restrict", .ns = 0, .sym = NULL };
Ident *__restrict___ident = &(Ident ) { .name = "__restrict__", .ns = 0, .sym = NULL };
Ident *__signed_ident = &(Ident ) { .name = "__signed", .ns = 0, .sym = NULL };
Ident *__signed___ident = &(Ident ) { .name = "__signed__", .ns = 0, .sym = NULL };
Ident *__thread_ident = &(Ident ) { .name = "__thread", .ns = 0, .sym = NULL };
Ident *typeof_ident = &(Ident ) { .name = "typeof", .ns = 0, .sym = NULL };
Ident *__typeof_ident = &(Ident ) { .name = "__typeof", .ns = 0, .sym = NULL };
Ident *__typeof___ident = &(Ident ) { .name = "__typeof__", .ns = 0, .sym = NULL };
Ident *__volatile_ident = &(Ident ) { .name = "__volatile", .ns = 0, .sym = NULL };
Ident *__volatile___ident = &(Ident ) { .name = "__volatile__", .ns = 0, .sym = NULL };
Ident *__label___ident = &(Ident ) { .name = "__label__", .ns = 0, .sym = NULL };
Ident *__extension___ident = &(Ident ) { .name = "__extension__", .ns = 0, .sym = NULL };

static int *g(int en)
{
    int *r = cc_malloc(sizeof(int));
    *r = en;
    return r;
}

HashMap *make_ops_map()
{
    HashMap *m = HashMap_new_str();

    HashMap_put(m, "%:%:", g(T_SHARP_SHARP));
    HashMap_put(m, "<:", g(T_LEFT_BRACKET));
    HashMap_put(m, ":>", g(T_RIGHT_BRACKET));
    HashMap_put(m, "<%", g(T_LEFT_BRACE));
    HashMap_put(m, "%>", g(T_RIGHT_BRACE));
    HashMap_put(m, "%:", g(T_SHARP));

    HashMap_put(m, ">>=", g(T_RSHIFT_EQUAL));
    HashMap_put(m, "<<=", g(T_LSHIFT_EQUAL));
    HashMap_put(m, "...", g(T_DOT_DOT_DOT));
    HashMap_put(m, "->", g(T_ARROW));
    HashMap_put(m, "--", g(T_MINUS_MINUS));
    HashMap_put(m, "-=", g(T_MINUS_EQUAL));
    HashMap_put(m, "!=", g(T_NE));
    HashMap_put(m, "..", g(T_DOT_DOT));
    HashMap_put(m, "*=", g(T_TIMES_EQUAL));
    HashMap_put(m, "/=", g(T_DIVIDE_EQUAL));
    HashMap_put(m, "&=", g(T_AND_EQUAL));
    HashMap_put(m, "&&", g(T_AND_AND));
    HashMap_put(m, "##", g(T_SHARP_SHARP));
    HashMap_put(m, "%=", g(T_PERCENT_EQUAL));
    HashMap_put(m, "^=", g(T_XOR_EQUAL));
    HashMap_put(m, "++", g(T_PLUS_PLUS));
    HashMap_put(m, "+=", g(T_PLUS_EQUAL));
    HashMap_put(m, "<=", g(T_LE));
    HashMap_put(m, "<<", g(T_LSHIFT));
    HashMap_put(m, "==", g(T_EQ));
    HashMap_put(m, ">=", g(T_GE));
    HashMap_put(m, ">>", g(T_RSHIFT));
    HashMap_put(m, "||", g(T_OR_OR));
    HashMap_put(m, "|=", g(T_OR_EQUAL));
    HashMap_put(m, ",", g(T_COMMA));
    HashMap_put(m, "-", g(T_MINUS));
    HashMap_put(m, ";", g(T_SEMI_COLON));
    HashMap_put(m, ":", g(T_COLON));
    HashMap_put(m, "!", g(T_EXCLAMATION));
    HashMap_put(m, "?", g(T_QUESTION));
    HashMap_put(m, ".", g(T_DOT));
    HashMap_put(m, "(", g(T_LEFT_PAREN));
    HashMap_put(m, ")", g(T_RIGHT_PAREN));
    HashMap_put(m, "[", g(T_LEFT_BRACKET));
    HashMap_put(m, "]", g(T_RIGHT_BRACKET));
    HashMap_put(m, "{", g(T_LEFT_BRACE));
    HashMap_put(m, "}", g(T_RIGHT_BRACE));
    HashMap_put(m, "*", g(T_TIMES));
    HashMap_put(m, "/", g(T_DIVIDE));
    HashMap_put(m, "&", g(T_AND));
    HashMap_put(m, "#", g(T_SHARP));
    HashMap_put(m, "%", g(T_PERCENT));
    HashMap_put(m, "^", g(T_XOR));
    HashMap_put(m, "+", g(T_PLUS));
    HashMap_put(m, "<", g(T_LT));
    HashMap_put(m, "=", g(T_ASSIGN));
    HashMap_put(m, ">", g(T_GT));
    HashMap_put(m, "|", g(T_OR));
    HashMap_put(m, "~", g(T_TILDE));
    HashMap_put(m, "$", g(T_DOLLAR_SIGN));
    HashMap_put(m, "@", g(T_AT_SIGN));
    HashMap_put(m, "`", g(T_GRAVE_ACCENT));
    HashMap_put(m, "\\", g(T_BACKSLASH));

    return m;
}

HashMap *make_idents_map()
{
    HashMap *m = HashMap_new_str();

    HashMap_put(m, "auto", auto_ident);
    HashMap_put(m, "break", break_ident);
    HashMap_put(m, "case", case_ident);
    HashMap_put(m, "char", char_ident);
    HashMap_put(m, "const", const_ident);
    HashMap_put(m, "continue", continue_ident);
    HashMap_put(m, "default", default_ident);
    HashMap_put(m, "do", do_ident);
    HashMap_put(m, "double", double_ident);
    HashMap_put(m, "else", else_ident);
    HashMap_put(m, "enum", enum_ident);
    HashMap_put(m, "extern", extern_ident);
    HashMap_put(m, "float", float_ident);
    HashMap_put(m, "for", for_ident);
    HashMap_put(m, "goto", goto_ident);
    HashMap_put(m, "if", if_ident);
    HashMap_put(m, "inline", inline_ident);
    HashMap_put(m, "int", int_ident);
    HashMap_put(m, "long", long_ident);
    HashMap_put(m, "register", register_ident);
    HashMap_put(m, "restrict", restrict_ident);
    HashMap_put(m, "return", return_ident);
    HashMap_put(m, "short", short_ident);
    HashMap_put(m, "signed", signed_ident);
    HashMap_put(m, "sizeof", sizeof_ident);
    HashMap_put(m, "static", static_ident);
    HashMap_put(m, "struct", struct_ident);
    HashMap_put(m, "switch", switch_ident);
    HashMap_put(m, "typedef", typedef_ident);
    HashMap_put(m, "union", union_ident);
    HashMap_put(m, "unsigned", unsigned_ident);
    HashMap_put(m, "void", void_ident);
    HashMap_put(m, "volatile", volatile_ident);
    HashMap_put(m, "while", while_ident);
    HashMap_put(m, "_Alignas", _Alignas_ident);
    HashMap_put(m, "_Alignof", _Alignof_ident);
    HashMap_put(m, "_Atomic", _Atomic_ident);
    HashMap_put(m, "_Bool", _Bool_ident);
    HashMap_put(m, "_Complex", _Complex_ident);
    HashMap_put(m, "_Decimal128", _Decimal128_ident);
    HashMap_put(m, "_Decimal32", _Decimal32_ident);
    HashMap_put(m, "_Decimal64", _Decimal64_ident);
    HashMap_put(m, "_Generic", _Generic_ident);
    HashMap_put(m, "_Imaginary", _Imaginary_ident);
    HashMap_put(m, "_Noreturn", _Noreturn_ident);
    HashMap_put(m, "_Static_assert", _Static_assert_ident);
    HashMap_put(m, "_Thread_local", _Thread_local_ident);
    HashMap_put(m, "asm", asm_ident);
    HashMap_put(m, "__asm", __asm_ident);
    HashMap_put(m, "__asm__", __asm___ident);
    HashMap_put(m, "__alignof", __alignof_ident);
    HashMap_put(m, "__alignof__", __alignof___ident);
    HashMap_put(m, "__attribute", __attribute_ident);
    HashMap_put(m, "__attribute__", __attribute___ident);
    HashMap_put(m, "__complex", __complex_ident);
    HashMap_put(m, "__complex__", __complex___ident);
    HashMap_put(m, "__const", __const_ident);
    HashMap_put(m, "__const__", __const___ident);
    HashMap_put(m, "__inline", __inline_ident);
    HashMap_put(m, "__inline__", __inline___ident);
    HashMap_put(m, "__restrict", __restrict_ident);
    HashMap_put(m, "__restrict__", __restrict___ident);
    HashMap_put(m, "__signed", __signed_ident);
    HashMap_put(m, "__signed__", __signed___ident);
    HashMap_put(m, "__thread", __thread_ident);
    HashMap_put(m, "typeof", typeof_ident);
    HashMap_put(m, "__typeof", __typeof_ident);
    HashMap_put(m, "__typeof__", __typeof___ident);
    HashMap_put(m, "__volatile", __volatile_ident);
    HashMap_put(m, "__volatile__", __volatile___ident);
    HashMap_put(m, "__label__", __label___ident);
    HashMap_put(m, "__extension__", __extension___ident);

    return m;
}
