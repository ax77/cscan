#ifndef DRCC_H_
#define DRCC_H_

#include "ccore/fdesc.h"
#include "ccore/fnptr.h"
#include "ccore/hdrs.h"
#include "ccore/map.h"
#include "ccore/vec.h"
#include "ccore/str.h"
#include "ccore/buf.h"
#include "ccore/ascii.h"
#include "ccore/xmem.h"

typedef enum T {
    TOKEN_EOF, TOKEN_ERROR,

    // General identifier, may be a user-defined-name, or a keyword.
    TOKEN_IDENT,
    TOKEN_NUMBER,
    TOKEN_CHAR,
    TOKEN_STRING,
    TOKEN_COMMENT,

    T_RSHIFT_EQUAL, // >>=
    T_LSHIFT_EQUAL, // <<=
    T_DOT_DOT_DOT, // ...
    T_ARROW, // ->
    T_MINUS_MINUS, // --
    T_MINUS_EQUAL, // -=
    T_NE, // !=
    T_DOT_DOT, // ..
    T_TIMES_EQUAL, // *=
    T_DIVIDE_EQUAL, // /=
    T_AND_EQUAL, // &=
    T_AND_AND, // &&
    T_SHARP_SHARP, // ##
    T_PERCENT_EQUAL, // %=
    T_XOR_EQUAL, // ^=
    T_PLUS_PLUS, // ++
    T_PLUS_EQUAL, // +=
    T_LE, // <=
    T_LSHIFT, // <<
    T_EQ, // ==
    T_GE, // >=
    T_RSHIFT, // >>
    T_OR_OR, // ||
    T_OR_EQUAL, // |=
    T_COMMA, // ,
    T_MINUS, // -
    T_SEMI_COLON, // ;
    T_COLON, // :
    T_EXCLAMATION, // !
    T_QUESTION, // ?
    T_DOT, // .
    T_LEFT_PAREN, // (
    T_RIGHT_PAREN, // )
    T_LEFT_BRACKET, // [
    T_RIGHT_BRACKET, // ]
    T_LEFT_BRACE, // {
    T_RIGHT_BRACE, // }
    T_TIMES, // *
    T_DIVIDE, // /
    T_AND, // &
    T_SHARP, // #
    T_PERCENT, // %
    T_XOR, // ^
    T_PLUS, // +
    T_LT, // <
    T_ASSIGN, // =
    T_GT, // >
    T_OR, // |
    T_TILDE, // ~
    T_DOLLAR_SIGN, // $
    T_AT_SIGN, // @
    T_GRAVE_ACCENT, // `
    T_BACKSLASH, // "\"
} T;

struct PpSym;
struct Ident;
struct Token;

vec_proto(struct Token*, token);
extern struct Token *EOF_TOKEN_ENTRY;

enum string_encoding {
    // string encoding prefix
    STR_ENC_NONE,
    STR_ENC_CH16_u,
    STR_ENC_CH32_U,
    STR_ENC_UTF8_u8,
    STR_ENC_WIDE_L,
};

typedef struct PpSym {
    char *name;
    vec(token) *repl;
    vec(token) *parm;
    vec(u32) *usage;
    int is_hidden;
    int is_vararg;
    int arity;
} PpSym;

typedef struct Ident {
    char *name;
    unsigned ns; // namespace
    PpSym *sym;
} Ident;

typedef struct Token {
    T type;
    char *value;
    unsigned int fcategory;
    unsigned int fposition;
    int argnum;
    Ident *ident;
    struct {
        char *filename;
        int line;
        int column;
    } pos;
    struct {
        char *buffer;
        enum string_encoding enc;
    } str;
} Token;

Token *token_new(T type, char *value);

// Token Category
#define formal     (1u << 0u)
#define scanned    (1u << 1u)
#define stringized (1u << 2u)
#define unscanned  (1u << 3u)
#define hashhash   (1u << 4u)
#define commaopt   (1u << 5u)

// Token Position
#define fnewline   (1u << 0u)
#define fleadws    (1u << 1u)
#define fatbol     (1u << 2u)
#define fpainted   (1u << 3u)

HashMap *make_ops_map();
HashMap *make_idents_map();
char *toktype_tos(T t);

// Identifiers

extern Ident *auto_ident;
extern Ident *break_ident;
extern Ident *case_ident;
extern Ident *char_ident;
extern Ident *const_ident;
extern Ident *continue_ident;
extern Ident *default_ident;
extern Ident *do_ident;
extern Ident *double_ident;
extern Ident *else_ident;
extern Ident *enum_ident;
extern Ident *extern_ident;
extern Ident *float_ident;
extern Ident *for_ident;
extern Ident *goto_ident;
extern Ident *if_ident;
extern Ident *inline_ident;
extern Ident *int_ident;
extern Ident *long_ident;
extern Ident *register_ident;
extern Ident *restrict_ident;
extern Ident *return_ident;
extern Ident *short_ident;
extern Ident *signed_ident;
extern Ident *sizeof_ident;
extern Ident *static_ident;
extern Ident *struct_ident;
extern Ident *switch_ident;
extern Ident *typedef_ident;
extern Ident *union_ident;
extern Ident *unsigned_ident;
extern Ident *void_ident;
extern Ident *volatile_ident;
extern Ident *while_ident;
extern Ident *_Alignas_ident;
extern Ident *_Alignof_ident;
extern Ident *_Atomic_ident;
extern Ident *_Bool_ident;
extern Ident *_Complex_ident;
extern Ident *_Decimal128_ident;
extern Ident *_Decimal32_ident;
extern Ident *_Decimal64_ident;
extern Ident *_Generic_ident;
extern Ident *_Imaginary_ident;
extern Ident *_Noreturn_ident;
extern Ident *_Static_assert_ident;
extern Ident *_Thread_local_ident;
extern Ident *asm_ident;
extern Ident *__asm_ident;
extern Ident *__asm___ident;
extern Ident *__alignof_ident;
extern Ident *__alignof___ident;
extern Ident *__attribute_ident;
extern Ident *__attribute___ident;
extern Ident *__complex_ident;
extern Ident *__complex___ident;
extern Ident *__const_ident;
extern Ident *__const___ident;
extern Ident *__inline_ident;
extern Ident *__inline___ident;
extern Ident *__restrict_ident;
extern Ident *__restrict___ident;
extern Ident *__signed_ident;
extern Ident *__signed___ident;
extern Ident *__thread_ident;
extern Ident *typeof_ident;
extern Ident *__typeof_ident;
extern Ident *__typeof___ident;
extern Ident *__volatile_ident;
extern Ident *__volatile___ident;
extern Ident *__label___ident;
extern Ident *__extension___ident;

#endif /* DRCC_H_ */
