#include "core_strutil.h"
#include "core_mem.h"

int strstarts(char *what, char *with) {
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

int strends(char *what, char *with) {
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

StrBuilder *sb_new() {
    StrBuilder *rv = malloc(sizeof(StrBuilder));
    rv->len = 0;
    rv->alloc = 8;
    rv->str = malloc(rv->alloc * sizeof(char));
    rv->str[rv->len] = '\0';
    return rv;
}

StrBuilder *sb_news(char * str) {
    StrBuilder *rv = sb_new();
    sb_adds(rv, str);
    return rv;
}

static void sb_grow(StrBuilder *s) {
    s->alloc *= 2;
    s->str = realloc(s->str, s->alloc * sizeof(char));
}

void sb_addc(StrBuilder *s, char c) {
    if (!c) {
        return;
    }
    if ((s->len + 2) == s->alloc) {
        sb_grow(s);
    }
    s->str[s->len++] = c;
    s->str[s->len] = '\0';
}

void sb_adds(StrBuilder *s, char *news) {
    if (!news) {
        return;
    }
    for (size_t i = 0; news[i]; i++) {
        sb_addc(s, news[i]);
    }
}

StrBuilder *sb_copy(StrBuilder *what) {
    StrBuilder *res = sb_new();
    sb_adds(res, what->str);
    return res;
}

StrBuilder *sb_left(StrBuilder *from, size_t much) {
    assert(from && from->str);

    StrBuilder *res = sb_new();
    // I) empty one or another.
    if (from->len == 0 || much == 0) {
        return res;
    }
    // II) overflow, return full content of src
    if (much >= from->len) {
        return sb_copy(from);
    }
    // III) normal cases
    for (size_t i = 0; i < much; i++) {
        sb_addc(res, from->str[i]);
    }
    return res;
}

StrBuilder *sb_right(StrBuilder *from, size_t much) {
    assert(from && from->str);

    StrBuilder *res = sb_new();
    // I) empty one or another.
    if (from->len == 0 || much == 0) {
        return res;
    }
    // II) overflow, return full content of src
    if (much >= from->len) {
        return sb_copy(from);
    }
    //III) normal cases
    size_t start = from->len - much;
    for (size_t i = start; i < from->len; i++) {
        sb_addc(res, from->str[i]);
    }
    return res;
}

StrBuilder *sb_mid(StrBuilder *from, size_t begin, size_t much) {
    assert(from && from->str);

    StrBuilder *res = sb_new();
    // I) empty
    if (begin >= from->len) {
        return res;
    }
    // II) overflow, return full content of src from begin to .len
    if (much >= from->len) {
        much = from->len;
    }
    size_t end = begin + much;
    if (end >= from->len) {
        end = from->len;
    }
    for (size_t i = begin; i < end; i++) {
        sb_addc(res, from->str[i]);
    }
    return res;
}

StrBuilder *sb_trim(StrBuilder *from) {
    assert(from && from->str);

    StrBuilder *res = sb_new();
    if (from->len == 0) {
        return res;
    }

    size_t start = 0;
    size_t end = 0;

    for (start = 0; start < from->len; start++) {
        int c = from->str[start];
        if (c > ' ') {
            break;
        }
    }

    for (end = from->len; end != 0; end--) {
        int c = from->str[end];
        if (c > ' ') {
            break;
        }
    }

    for (size_t i = start; i <= end; i++) {
        sb_addc(res, from->str[i]);
    }
    return res;
}

static void sb_free(StrBuilder ** sb) {
    if (*sb) {

        // string, which contains inside
        free((*sb)->str);
        (*sb)->str = NULL;

        // buffer as is
        (*sb)->alloc = 0;
        (*sb)->len = 0;

        free(*sb);
        *sb = NULL;
    }
}

LinkedList * sb_split_char(StrBuilder * where, char sep, bool include_empty) {
    LinkedList * lines = list_new();
    size_t len = where->len;

    if (len == 0) {
        return lines;
    }

    StrBuilder * sb = sb_new();
    for (size_t i = 0; i < len; i++) {
        char c = where->str[i];
        if (c == sep) {
            if (sb->len > 0 || (sb->len == 0 && include_empty)) {
                list_push_back(lines, cc_strdup(sb->str));
            }

            sb_free(&sb);
            sb = sb_new();

            continue;
        }
        sb_addc(sb, c);
    }

    if (sb->len > 0 || (sb->len == 0 && include_empty)) {
        list_push_back(lines, cc_strdup(sb->str));
    }

    sb_free(&sb);
    return lines;
}

