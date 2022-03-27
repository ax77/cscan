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

#define kw(n) Ident * n##_ident = &(Ident) { .name = STR(n), .ns = 0, .sym = NULL };
#include "ops"

static int *g(int en)
{
    int *r = cc_malloc(sizeof(int));
    *r = en;
    return r;
}

HashMap *make_ops_map()
{
    HashMap *m = HashMap_new_str();

#   define op_digr(op, en) HashMap_put(m, op, g(en));
#   define op(op, en) HashMap_put(m, op, g(en));
#   include "ops"

    return m;
}

HashMap *make_idents_map()
{
    HashMap *m = HashMap_new_str();

#   define kw(n) HashMap_put(m, STR(n), n##_ident);
#   include "ops"

    return m;
}

char *toktype_tos(T t) {

#   define op_spec(op, en) if(t == en) { return op; };
#   define op(op, en) if(t == en) { return op; };
#   include "ops"

    return "<unknown-token-type>";
}
