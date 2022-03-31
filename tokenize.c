#include "drcc.h"
#include "tests.h"

typedef struct Context {
    char *filename;
    CharBuf *buffer;
    map(idents) *ident_hash;
    map(operators) *operators;
    vec(token) *tokenlist;
} Context;

Context* make_context(char *filename)
{
    assert(filename);

    Context *ctx = cc_malloc(sizeof(struct Context));
    ctx->filename = filename;
    ctx->buffer = charbuf_new(hb_readfile2(filename));
    ctx->ident_hash = make_idents_map();
    ctx->operators = make_ops_map();
    ctx->tokenlist = vec_new(token);
    return ctx;
}

// markers
static Token WSP_TOKEN = { };
static Token EOL_TOKEN = { };

static Ident* ctx_make_ident(Context *ctx, char *name);
static Token* parse_ident_token(Context *ctx);
static Token* ctx_make_token(Context *ctx, T type, char *value);

static Ident* ctx_make_ident(Context *ctx, char *name)
{
    map_result(idents) opt = map_get(ctx->ident_hash, name);
    if (opt.found) {
        return opt.value;
    }
    Ident *newid = cc_malloc(sizeof(struct Ident));
    newid->name = cc_strdup(name);
    map_put(ctx->ident_hash, name, newid);
    return newid;
}

static Token* ctx_make_token(Context *ctx, T type, char *value)
{
    assert(value);

    Token *token = token_new(type, value);
    CharBuf *buffer = ctx->buffer;
    assert(buffer);

    int column = (buffer->column - strlen(token->value)) + 1;
    token->pos.line = buffer->line;
    token->pos.column = column;

    return token;
}

static Token* parse_ident_token(Context *ctx)
{
    CharBuf *buf = ctx->buffer;

    Str sb = STR_INIT;
    sb_addc(&sb, (char) charbuf_nextc(buf));

    for (;;) {
        int peek = charbuf_peekc(buf);
        int is_identifier_tail = is_letter(peek) || is_dec(peek);
        if (!is_identifier_tail) {
            break;
        }
        sb_addc(&sb, (char) charbuf_nextc(buf));
    }

    assert(sb.size);
    char *buffer = sb.data;

    Token *tok = ctx_make_token(ctx, TOKEN_IDENT, buffer);
    tok->ident = ctx_make_ident(ctx, buffer);

    return tok;
}

static Token* parse_pp_number(Context *ctx)
{

    CharBuf *buf = ctx->buffer;

    /*
     * pp-number:
     *   digit
     *   . digit
     *   pp-number digit
     *   pp-number identifier-nondigit
     *   pp-number e sign
     *   pp-number E sign
     *   pp-number .
     *   pp-number ' digit
     *   pp-number ' nondigit
     */

    Str strbuf = STR_INIT;
    sb_addc(&strbuf, charbuf_nextc(buf));

    for (;;) {
        int *chars = charbuf_next4(buf);
        int c1 = chars[0];
        int c2 = chars[1];

        if (is_dec(c1) || is_letter(c1) || c1 == '.') {
            sb_addc(&strbuf, charbuf_nextc(buf));
            continue;
        }

        if (c1 == 'e' || c1 == 'E' || c1 == 'p' || c1 == 'P') {
            sb_addc(&strbuf, charbuf_nextc(buf));
            if (c2 == '-' || c2 == '+') {
                sb_addc(&strbuf, charbuf_nextc(buf));
            }
            continue;
        }

        if (c1 == '\'' && (is_dec(c2) || is_letter(c2))) {
            charbuf_nextc(buf); // just skip this tick
            continue;
        }

        break;
    }

    return ctx_make_token(ctx, TOKEN_NUMBER, strbuf.data);

}

static Token* parse_string_token(Context *ctx, enum string_encoding enc)
{
    CharBuf *buffer = ctx->buffer;
    assert(buffer);

    int endof = charbuf_nextc(buffer);
    T typeoftok = (endof == '\'') ? TOKEN_CHAR : TOKEN_STRING;

    Str sb = STR_INIT;
    sb_addc(&sb, endof);

    for (;;) {
        int next1 = charbuf_nextc(buffer);
        if (next1 == HC_FEOF) {
            cc_fatal("%s\n", "unexpected EOF at the middle of the string");
        }
        if (next1 == '\n') {
            cc_fatal("%s\n", "unexpected LF at the middle of the string");
        }
        if (next1 == endof) {
            break;
        }
        if (next1 != '\\') {
            sb_addc(&sb, next1);
            continue;
        }
        int next2 = charbuf_nextc(buffer);
        sb_addc(&sb, '\\');
        sb_addc(&sb, next2);
    }

    // TODO: escape the buffer here, and set escaped content to the token

    sb_addc(&sb, endof);
    return ctx_make_token(ctx, typeoftok, sb.data);
}

Token* nex2(Context *ctx)
{
    CharBuf *buf = ctx->buffer;
    int *chars = charbuf_next4(buf);
    int c1 = chars[0];
    int c2 = chars[1];
    int c3 = chars[2];
    int c4 = chars[3];

    if (c1 == HC_FEOF) {
        return EOF_TOKEN_ENTRY;
    }

    int is_whitespace = c1 == ' ' || c1 == '\t' || c1 == '\f';
    if (is_whitespace) {
        charbuf_nextc(buf);
        return &WSP_TOKEN;
    }

    if (c1 == '\n') {
        charbuf_nextc(buf);
        return &EOL_TOKEN;
    }

    // c89/99 style comments
    if (c1 == '/') {

        if (c2 == '/') {
            charbuf_nextc(buf);
            charbuf_nextc(buf);

            for (;;) {
                int tmpch = charbuf_nextc(buf);
                if (tmpch == '\n') {
                    return &EOL_TOKEN;
                }
                if (tmpch == HC_FEOF) {
                    return EOF_TOKEN_ENTRY;
                }
            }
        }

        else if (c2 == '*') {
            charbuf_nextc(buf);
            charbuf_nextc(buf);

            int prevc = '\0';
            for (;;) {
                int tmpch = charbuf_nextc(buf);
                if (tmpch == HC_FEOF) {
                    cc_fatal("%s\n", "unclosed comment");
                }
                if (tmpch == '/' && prevc == '*') {
                    return &WSP_TOKEN;
                }
                prevc = tmpch;
            }
        }
    }

    if (c1 == '\"' || c1 == '\'') {
        return parse_string_token(ctx, STR_ENC_NONE);
    }

    if (is_dec(c1) || (c1 == '.' && is_dec(c2))) {
        return parse_pp_number(ctx);
    }

    if (is_op_start(c1)) {
        char buf4[] = { c1, c2, c3, c4, '\0' };
        char buf3[] = { c1, c2, c3, '\0' };
        char buf2[] = { c1, c2, '\0' };
        char buf1[] = { c1, '\0' };

        map_result(operators) type4 = map_get(ctx->operators, buf4);
        map_result(operators) type3 = map_get(ctx->operators, buf3);
        map_result(operators) type2 = map_get(ctx->operators, buf2);
        map_result(operators) type1 = map_get(ctx->operators, buf1);

        if (type4.found) {
            charbuf_nextc(buf);
            charbuf_nextc(buf);
            charbuf_nextc(buf);
            charbuf_nextc(buf);

            return ctx_make_token(ctx, type4.value, buf4);
        }
        if (type3.found) {
            charbuf_nextc(buf);
            charbuf_nextc(buf);
            charbuf_nextc(buf);

            return ctx_make_token(ctx, type3.value, buf3);
        }
        if (type2.found) {
            charbuf_nextc(buf);
            charbuf_nextc(buf);

            return ctx_make_token(ctx, type2.value, buf2);
        }
        if (type1.found) {
            charbuf_nextc(buf);

            return ctx_make_token(ctx, type1.value, buf1);
        }

        cc_fatal("Unrecognized operator sequence: [%s]\n", buf4);
    }

    if (is_letter(c1)) {

        Token *tok = parse_ident_token(ctx);
        return tok;

    }

    char otherascii[] = { c1, '\0' };
    map_result(operators) perhaps = map_get(ctx->operators, otherascii);
    if (perhaps.found) {
        charbuf_nextc(buf); // XXX
        return ctx_make_token(ctx, perhaps.value, otherascii);
    }

    charbuf_nextc(buf); // XXX
    printf("unrecognized character: %c\n", (char) c1);

    char unknown[] = { c1, '\0' };
    return ctx_make_token(ctx, TOKEN_ERROR, unknown);
}

void tokenize_context(Context *ctx)
{
    vec(token) line = VEC_INIT(token);
    int nextws = 0;

    for (;;) {
        Token *t = nex2(ctx);
        if (t == EOF_TOKEN_ENTRY) {
            vec_add_all(ctx->tokenlist, &line);
            vec_push_back(ctx->tokenlist, t);
            break;
        }
        if (nextws) {
            t->fposition |= fleadws;
            nextws = 0;
        }
        if (t == &EOL_TOKEN) {
            if (vec_is_empty(&line)) {
                continue;
            }
            Token *first = vec_get(&line, 0);
            Token *last = vec_get(&line, line.size - 1);
            last->fposition |= fnewline;
            first->fposition |= fatbol;
            first->fposition |= fleadws;

            vec_add_all(ctx->tokenlist, &line);
            vec_reset(&line);

            continue;
        }

        if (t == &WSP_TOKEN) {
            nextws = 1;
            continue;
        }

        vec_push_back(&line, t);
    }
}

vec(token)* tokenize(Context *ctx)
{
    tokenize_context(ctx);
    return ctx->tokenlist;
}

typedef struct Scan {
    vec(token) *tokens;
    vec(token) *rescan;
    size_t size, offset;
} Scan;

Scan* scan_new(vec(token) *tokens)
{
    Scan *s = cc_malloc(sizeof(Scan));
    s->tokens = tokens;
    s->rescan = vec_new(token);

    s->size = vec_size(tokens);
    s->offset = 0;
    return s;
}

int scan_has_tokens(Scan *s)
{
    return s->offset < s->size;
}

int scan_is_empty(Scan *s)
{
    if (scan_has_tokens(s)) {
        return 0;
    }
    if (!vec_is_empty(s->rescan)) {
        return 0;
    }
    return 1;
}

Token* scan_pop_noppdirective(Scan *s)
{
    if (!vec_is_empty(s->rescan)) {
        return vec_pop_back(s->rescan);
    }
    if (s->offset >= s->size) {
        return EOF_TOKEN_ENTRY;
    }
    Token *t = vec_get(s->tokens, s->offset);
    s->offset += 1;
    return t;
}

Token* scan_pop(Scan *s)
{
    Token *t = scan_pop_noppdirective(s);
    if (t->type == T_SHARP && (t->fposition & fatbol)) {
        if (t->fposition & fnewline) {
            t->type = PT_HEOL;
            return t;
        }
        Token *pp = scan_pop_noppdirective(s);
        assert(pp->type == TOKEN_IDENT);

        Ident *directive = pp->ident;
        if (directive == define_ident) {
            pp->type = PT_HDEFINE;
        } else {
            assert(0 && "todo!");
        }
        return pp;
    }
    return t;
}

vec(token)* paste_all(Token *head, vec(token) *repl);

void replace_simple(Scan *s, Token *head, PpSym *macros)
{
    assert(!macros->is_hidden);
    macros->is_hidden = 1;

    vec(token) *res = paste_all(head, macros->repl);

    Token *tok = NULL;
    vec_foreach_rev(res, tok)
    {
        if (tok->type == T_SPEC_PLACEMARKER) {
            continue;
        }
        vec_push_back(s->rescan, tok);
    }
}

vec(token)* paste_all(Token *head, vec(token) *repl)
{
    vec(token) *rv = vec_new(token);

    Token *tok = NULL;
    vec_foreach(repl, tok)
    {
        Token *ntok = token_copy(tok);
        vec_push_back(rv, ntok);
    }

    return rv;
}

int unhide(Token *u)
{
    if (u->type == T_SPEC_UNHIDE) {
        assert(u->ident->sym->is_hidden);
        u->ident->sym->is_hidden = 0;
        return 1;
    }
    return 0;
}

int is_ppdirtype(T tp)
{
#   define prepr(op, en) if(tp == en) { return 1; };
#   include "ops"

    return 0;
}

vec(token)* scan_cut_line(Scan *s)
{
    vec(token) *rv = vec_new(token);
    while (!scan_is_empty(s)) {
        Token *t = scan_pop_noppdirective(s);
        if ((t->fposition & fnewline) || t->type == TOKEN_EOF) {
            vec_push_back(rv, t);
            break;
        }
        vec_push_back(rv, t);
    }
    return rv;
}

int dline(Scan *s, Token *t)
{
    if (t->type == PT_HDEFINE) {
        Token *name = scan_pop_noppdirective(s);
        assert(name->type == TOKEN_IDENT);

        vec(token) *repl = scan_cut_line(s);
        PpSym *m = sym_new(name, repl);
        name->ident->sym = m;
        return 1;
    }
    return 0;
}

Token* scan_get(Scan *s)
{
    restart: while (!scan_is_empty(s)) {
        Token *t = scan_pop(s);
        if (is_ppdirtype(t->type)) {
            assert(dline(s, t));
            continue;
        }
        if (unhide(t)) {
            continue;
        }
        if (t->type != TOKEN_IDENT) {
            return t;
        }
        if (t->noexpand) {
            return t;
        }
        PpSym *macros = t->ident->sym;
        if (macros == NULL) {
            return t;
        }
        if (macros->is_hidden) {
            Token *noexpand = token_copy(t);
            noexpand->noexpand = 1;
            return noexpand;
        }
        replace_simple(s, t, macros);
        goto restart;

    }
    return EOF_TOKEN_ENTRY;
}

int main(int argc, char **argv)
{
    Context *ctx = make_context("input.txt");
    vec(token) *tokens = tokenize(ctx);

    Token *t = NULL;
    vec_foreach(tokens, t)
    {
        printf("%3lu [%s]\n", __i__, t->value);
    }

    printf("\n:ok:\n");
    return 0;
}

