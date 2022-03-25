#include "drcc.h"

typedef struct Context {
    char *filename;
    CharBuf *buffer;
    HashMap *ident_hash;
    HashMap *operators;
    vec(token) *tokenlist;
} Context;

Context *make_context(char *filename)
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
Token WSP_TOKEN = { };
Token EOL_TOKEN = { };

static Ident *ctx_make_ident(Context *ctx, char *name);
static Token* parse_ident_token(Context* ctx);
static Token *ctx_make_token(Context *ctx, T type, char *value);

static Ident *ctx_make_ident(Context *ctx, char *name)
{
    Ident *id = HashMap_get(ctx->ident_hash, name);
    if (id) {
        return id;
    }
    Ident *newid = cc_malloc(sizeof(struct Ident));
    newid->name = cc_strdup(name);
    HashMap_put(ctx->ident_hash, name, newid);
    return newid;
}

static Token *ctx_make_token(Context *ctx, T type, char *value)
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

static Token* parse_ident_token(Context* ctx)
{
    CharBuf* buf = ctx->buffer;

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

    assert(sb.len);
    char* buffer = sb.buf;

    Token* tok = ctx_make_token(ctx, TOKEN_IDENT, buffer);
    tok->ident = ctx_make_ident(ctx, buffer);

    return tok;
}

static Token *parse_string_token(Context *ctx, enum string_encoding enc)
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
    return ctx_make_token(ctx, typeoftok, sb.buf);
}

Token *nex2(Context *ctx)
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
        /*
         * pp-number:
         *   digit
         *   . digit
         *   pp-number digit
         *   pp-number identifier-nondigit
         *   pp-number e sign
         *   pp-number E sign
         *   pp-number .
         */

        Str strbuf = STR_INIT;
        sb_addc(&strbuf, charbuf_nextc(buf));

        for (;;) {
            int peek = charbuf_peekc(buf);
            if (is_dec(peek)) {
                sb_addc(&strbuf, charbuf_nextc(buf));
                continue;
            } else if (peek == 'e' || peek == 'E' || peek == 'p' || peek == 'P') {
                sb_addc(&strbuf, charbuf_nextc(buf));

                peek = charbuf_peekc(buf);
                if (peek == '-' || peek == '+') {
                    sb_addc(&strbuf, charbuf_nextc(buf));
                }
                continue;
            } else if (peek == '.' || is_letter(peek)) {
                sb_addc(&strbuf, charbuf_nextc(buf));
                continue;
            }

            break;
        }
        return ctx_make_token(ctx, TOKEN_NUMBER, strbuf.buf);
    }

    if (is_op_start(c1)) {
        char buf4[] = { c1, c2, c3, c4, '\0' };
        char buf3[] = { c1, c2, c3, '\0' };
        char buf2[] = { c1, c2, '\0' };
        char buf1[] = { c1, '\0' };

        void * type4 = HashMap_get(ctx->operators, buf4);
        void * type3 = HashMap_get(ctx->operators, buf3);
        void * type2 = HashMap_get(ctx->operators, buf2);
        void * type1 = HashMap_get(ctx->operators, buf1);

        if (type4 != NULL) {
            charbuf_nextc(buf);
            charbuf_nextc(buf);
            charbuf_nextc(buf);
            charbuf_nextc(buf);

            int *type4en = ((int*) type4);
            return ctx_make_token(ctx, *type4en, buf4);
        }
        if (type3 != NULL) {
            charbuf_nextc(buf);
            charbuf_nextc(buf);
            charbuf_nextc(buf);

            int *type3en = ((int*) type3);
            return ctx_make_token(ctx, *type3en, buf3);
        }
        if (type2 != NULL) {
            charbuf_nextc(buf);
            charbuf_nextc(buf);

            int *type2en = ((int*) type2);
            return ctx_make_token(ctx, *type2en, buf2);
        }
        if (type1 != NULL) {
            charbuf_nextc(buf);

            int *type1en = ((int*) type1);
            return ctx_make_token(ctx, *type1en, buf1);
        }

        cc_fatal("Unrecognized operator sequence: [%s]\n", buf4);
    }

    if (is_letter(c1)) {

        Token* tok = parse_ident_token(ctx);
        return tok;

    }

    char otherascii[] = { c1, '\0' };
    void * perhaps = HashMap_get(ctx->operators, otherascii);
    if (perhaps) {
        charbuf_nextc(buf); // XXX
        int *type2en = ((int*) perhaps);
        return ctx_make_token(ctx, *type2en, otherascii);
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

vec(token) *tokenize(Context *ctx)
{
    tokenize_context(ctx);
    return ctx->tokenlist;
}

int main(int argc, char **argv)
{
    Context *ctx = make_context("input.txt");
    vec(token) *tokens = tokenize(ctx);

    for (size_t i = 0; i < tokens->size; i++) {
        Token *t = vec_get(tokens, i);
        printf("%3d:%3d BOL=%d LF=%d WS=%d %s\n", t->pos.line, t->pos.column, t->fposition & fatbol,
                t->fposition & fnewline, t->fposition & fleadws, t->value);
    }

    printf("\n:ok:\n");
    return 0;
}
