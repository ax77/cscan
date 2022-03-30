#include "drcc.h"

vec_impl(struct Token*, token);

map_impl(char*, Ident*, idents);
map_impl(char*, int, operators);

Token *EOF_TOKEN_ENTRY = &(Token ) { .type = TOKEN_EOF, .value = "eof" };

Token* token_new(T type, char *value)
{
    Token *t = cc_malloc(sizeof(struct Token));
    t->type = type;
    t->value = cc_strdup(value);
    t->noexpand = 0;
    return t;
}

Token* token_copy(Token *another)
{
    Token *t = cc_malloc(sizeof(struct Token));
    *t = *another;
    return t;
}

PpSym* sym_new(Token *macid, vec(token) *repl)
{
    Token *unhide = token_copy(macid);
    unhide->type = T_SPEC_UNHIDE;
    vec_push_back(repl, unhide);

    PpSym *s = cc_malloc(sizeof(PpSym));
    s->macid = macid;
    s->repl = repl;
    s->is_hidden = 0;
    s->is_vararg = 0;
    return s;
}

// Builtin-names
//

#define kw(n, namespc) Ident * n##_ident = &(Ident) { .name = STR(n), .ns = namespc, .sym = NULL };
#include "ops"


map(operators)* make_ops_map()
{
    map(operators) *m = map_new(operators, &hashmap_hash_str, &hashmap_equal_str);

#   define op_digr(op, en) map_put(m, op, en);
#   define op(op, en) map_put(m, op, en);
#   include "ops"

    return m;
}

map(idents)* make_idents_map()
{
    map(idents) *m = map_new(idents, &hashmap_hash_str, &hashmap_equal_str);

#   define kw(n, namespc) map_put(m, STR(n), n##_ident);
#   include "ops"

    return m;
}

char* toktype_tos(T t)
{

#   define op_spec(op, en) if(t == en) { return op; };
#   define op(op, en) if(t == en) { return op; };
#   define prepr(op, en) if(t == en) { return op; };
#   include "ops"

    return "<unknown-token-type>";
}
