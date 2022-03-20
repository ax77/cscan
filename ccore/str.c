#include "str.h"
#include "xmem.h"
#include "ascii.h"
#include "vec.h"

char EMPTY_STRBUF_STR[1];

static void sb_check_allocated(Str *s)
{
    assert(s);
    assert(s->buf);
    assert(s->buf != EMPTY_STRBUF_STR);
    assert(s->alloc > 0);
}

void sb_reset(Str *s)
{
    assert(s);
    assert(s->buf);
    s->buf = EMPTY_STRBUF_STR;
    s->len = 0;
    s->alloc = 0;
}

static size_t sb_grow(Str *s, size_t extra)
{
    assert(s);
    assert(s->buf);
    assert(extra);
    assert(extra < 3); // grow for extra 1 or 2 elements

    // Two cases: whether the buf is set as default global varible
    // or alloc is set to zero. It means that the buffer is used as
    // a value, not a pointer. We should support the correct allocation
    // at the first time the buffer is used.
    if (s->buf == EMPTY_STRBUF_STR) {
        assert(s->len == 0);
        assert(s->alloc == 0);
        s->alloc = 8;
        s->buf = cc_malloc(s->alloc * sizeof(char));
        return s->alloc;
    }
    if (s->alloc == 0) {
        assert(s->len == 0);
        assert(s->buf == EMPTY_STRBUF_STR);
        s->alloc = 8;
        s->buf = cc_malloc(s->alloc * sizeof(char));
        return s->alloc;
    }

    // must be allocated here.
    sb_check_allocated(s);

    // And here we'll check whether the allocated size
    // suits for the data we have, with adding a little
    // extra we need to append to the buffer.
    if ((s->len + extra) >= s->alloc) {
        s->alloc += 2;
        s->alloc *= 2;
        s->buf = cc_realloc(s->buf, s->alloc * sizeof(char));
    }

    return s->alloc;
}

int sb_addc(Str *s, char c)
{
    assert(s);
    assert(s->buf);

    if (!c) {
        return 0;
    }

    sb_grow(s, 2);

    s->buf[s->len++] = c;
    s->buf[s->len] = '\0';

    return 1;
}

size_t sb_adds(Str *s, char *news)
{
    if (!news) {
        return 0;
    }

    size_t i = 0;
    for (i = 0; news[i]; i++) {
        sb_addc(s, news[i]);
    }
    return i;
}

Str* sb_new()
{
    Str *rv = cc_malloc(sizeof(Str));
    rv->len = 0;
    rv->alloc = 8;
    rv->buf = cc_malloc(rv->alloc * sizeof(char));
    rv->buf[rv->len] = '\0';
    return rv;
}

Str* sb_news(char *str)
{
    Str *rv = sb_new();
    sb_adds(rv, str);
    return rv;
}

char* sb_left(char *from, size_t much)
{
    assert(from);
    size_t len = strlen(from);

    // I) empty one or another.
    if (len == 0 || much == 0) {
        return cc_strdup("");
    }

    // II) overflow, return full content of src
    if (much >= len) {
        return cc_strdup(from);
    }

    // III) normal cases
    Str res = STR_INIT;
    for (size_t i = 0; i < much && from[i]; i++) {
        sb_addc(&res, from[i]);
    }

    sb_check_allocated(&res);
    return res.buf;
}

char* sb_right(char *from, size_t much)
{
    assert(from);
    size_t len = strlen(from);

    // I) empty one or another.
    if (len == 0 || much == 0) {
        return cc_strdup("");
    }

    // II) overflow, return full content of src
    if (much >= len) {
        return cc_strdup(from);
    }

    //III) normal cases
    Str res = STR_INIT;
    assert(len > much);

    size_t start = len - much;
    for (size_t i = start; i < len && from[i]; i++) {
        sb_addc(&res, from[i]);
    }

    sb_check_allocated(&res);
    return res.buf;
}

char* sb_mid(char *from, size_t begin, size_t much)
{
    assert(from);
    size_t len = strlen(from);

    // I) empty
    if (begin >= len || (len == 0 || much == 0)) {
        return cc_strdup("");
    }
    // II) overflow, return full content of src from begin to .len
    if (much >= len) {
        much = len;
    }
    size_t end = begin + much;
    if (end >= len) {
        end = len;
    }

    Str res = STR_INIT;
    for (size_t i = begin; i < end && from[i]; i++) {
        sb_addc(&res, from[i]);
    }

    sb_check_allocated(&res);
    return res.buf;
}

char* sb_trim(char *from)
{
    assert(from);
    size_t len = strlen(from);

    if (len == 0) {
        return cc_strdup("");
    }

    size_t start = 0;
    size_t end = 0;

    for (start = 0; start < len; start++) {
        int c = from[start];
        if (c > ' ') {
            break;
        }
    }

    for (end = len; end != 0; end--) {
        int c = from[end];
        if (c > ' ') {
            break;
        }
    }

    Str res = STR_INIT;
    for (size_t i = start; i <= end && from[i]; i++) {
        sb_addc(&res, from[i]);
    }

    sb_check_allocated(&res);
    return res.buf;
}

static bool next_is(char *input, char *pattern, size_t input_len,
        size_t begin_index, size_t end_index, size_t pattern_len)
{
    if (end_index > input_len) {
        return false;
    }
    // TODO: we may do this in a much simple way, without any allocations :)
    char *substring = sb_mid(input, begin_index, pattern_len);
    int res = strcmp(substring, pattern) == 0;
    free(substring);
    return res;
}

char* sb_replace(char *input, char *pattern, char *replacement)
{
    if (input == NULL || strlen(input) == 0) {
        // it is more clear and simple to return empty value instead of null or exception.
        // because otherwise you should check the return value that it isn't null, and so on.
        // who cares about that? null or not null, we return empty string here.
        // and we'll work with this empty string in invocation point instead of that null.
        return cc_strdup("");
    }

    if (pattern == NULL || (strlen(pattern) == 0) || replacement == NULL) {
        return cc_strdup(input);
    }
    size_t input_len = strlen(input);
    size_t pattern_len = strlen(pattern);
    size_t repl_len = strlen(replacement);

    Str sb = STR_INIT;

    for (size_t offset = 0; offset < input_len;) {
        size_t end_index = pattern_len + offset;
        if (next_is(input, pattern, input_len, offset, end_index,
                pattern_len)) {
            if (repl_len > 0) {
                sb_adds(&sb, replacement);
            }
            offset += pattern_len;
        } else {
            sb_addc(&sb, input[offset]);
            offset++;
        }
    }

    sb_check_allocated(&sb);
    return sb.buf;
}

char* normalize_slashes(char *s)
{
    assert(s);

    size_t len = strlen(s);
    if (len == 0) {
        return cc_strdup("");
    }

    Str sb = STR_INIT;

    char p = '\0';
    for (size_t i = 0; i < len && s[i]; i++) {
        char c = s[i];
        if (c == '\\' || c == '/') {
            if (p == '\\' || p == '/') {
                p = c;
                continue;
            }
            if (c == '\\') {
                p = c;
                sb_addc(&sb, '/');
                continue;
            }
        }
        sb_addc(&sb, c);
        p = c;
    }

    sb_check_allocated(&sb);
    return sb.buf;
}

int is_abs_win(char *s)
{
    assert(s);
    if (strlen(s) >= 3) {
        return is_letter(s[0]) && s[1] == ':' && (s[2] == '\\' || s[2] == '/');
    }
    return 0;
}

int is_abs_unix(char *s)
{
    assert(s);
    return strlen(s) >= 1 && s[0] == '/';
}

int is_abs_path(char *s)
{
    assert(s);
    return is_abs_win(s) || is_abs_unix(s);
}

int strstarts(char *what, char *with)
{
    assert(what);
    assert(with);

    size_t L1 = strlen(what);
    size_t L2 = strlen(with);
    if (L1 == 0 || L2 == 0 || (L2 > L1)) {
        return 0;
    }

    for (size_t i = 0; i < L2; i++) {
        int c1 = what[i];
        int c2 = with[i];
        if (c1 != c2) {
            return 0;
        }
    }
    return 1;
}

int strends(char *what, char *with)
{
    assert(what);
    assert(with);

    size_t L1 = strlen(what);
    size_t L2 = strlen(with);
    if (L1 == 0 || L2 == 0 || (L2 > L1)) {
        return 0;
    }

    for (ptrdiff_t i = L1, j = L2; --i >= 0 && --j >= 0;) {
        int c1 = what[i];
        int c2 = with[j];
        if (c1 != c2) {
            return 0;
        }
    }
    return 1;
}

int strequal(void *a, void *b)
{
    char *str_1 = (char*) a;
    char *str_2 = (char*) b;
    return strcmp(str_1, str_2) == 0;
}

char* sb_buf_or_empty(Str *sb)
{
    if (sb->len == 0) {
        return cc_strdup("");
    }
    return sb->buf;
}

vec* sb_split_char(char *where, char sep, int include_empty)
{
    assert(where);

    vec *lines = vec_new();
    size_t len = strlen(where);

    if (len == 0) {
        return lines;
    }

    Str sb = STR_INIT;
    for (size_t i = 0; i < len && where[i]; i++) {
        char c = where[i];
        if (c == sep) {
            if (sb.len > 0 || (sb.len == 0 && include_empty)) {
                vec_push(lines, sb_buf_or_empty(&sb));
            }
            sb_reset(&sb);
            continue;
        }
        sb_addc(&sb, c);
    }

    if (sb.len > 0 || (sb.len == 0 && include_empty)) {
        vec_push(lines, sb_buf_or_empty(&sb));
    }
    return lines;
}

// TODO: simplify mem-use
char* normalize(char *given)
{
    char *tmp = normalize_slashes(given);
    vec *splitten = sb_split_char(tmp, '/', 1);

    vec *worklist = vec_new();
    if (is_abs_unix(tmp)) {
        vec_push(worklist, cc_strdup("/"));
    }

    for (size_t i = 0; i < splitten->size; i++) {
        Str *part = sb_news(vec_get(splitten, i));
        if (part->len == 0) {
            continue;
        }
        if (part->len == 1 && part->buf[0] == '.') {
            continue;
        }
        if (strequal(part->buf, "..")) {
            if (worklist->size != 0) {
                size_t lastidx = worklist->size - 1;
                char *last = vec_get(worklist, lastidx);
                if (!is_abs_path(last)) {
                    vec_pop(worklist);
                    continue;
                }
            }
        }
        if (i < (splitten->size - 1)) {
            sb_addc(part, '/');
        }
        vec_push(worklist, cc_strdup(part->buf));
    }

    Str sb = STR_INIT;
    for (size_t i = 0; i < worklist->size; i++) {
        char *s = vec_get(worklist, i);
        sb_adds(&sb, s);
    }

    return sb_buf_or_empty(&sb);
}

int sb_pop(Str *buf)
{
    assert(buf);
    assert(buf->len);
    char c = buf->buf[buf->len - 1];
    buf->len--;
    buf->buf[buf->len] = '\0';
    return c;
}

int sb_adds_rev(Str *buf, char *input)
{
    assert(buf);
    assert(input);

    const size_t len = strlen(input);
    if (len == 0) {
        return 0;
    }

    const ptrdiff_t last = len - 1;
    ptrdiff_t n = 0;
    for (ptrdiff_t i = last; i >= 0 && input[i]; i--, n++) {
        sb_addc(buf, input[i]);
    }

    return n;
}

int sb_char_at(Str *buf, size_t index)
{
    assert(buf);
    assert(buf->buf);
    assert(buf->len);
    assert(index < buf->len);
    return buf->buf[index];
}

int sb_is_empty(Str *buf)
{
    assert(buf);
    return buf->len == 0;
}

int sb_peek_last(Str *buf)
{
    assert(buf);
    assert(buf->len);
    return buf->buf[buf->len - 1];
}

