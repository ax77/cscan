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

#define STR(x) #x

typedef enum T {
#   define op_spec(op, en) en,
#   define op(op, en) en,
#   define prepr(op, en) en,
#   include "ops"
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
Token *token_copy(Token *another);

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

#define kw(n) extern Ident * n##_ident;
#include "ops"

#endif /* DRCC_H_ */
