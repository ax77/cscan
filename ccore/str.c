#include "str.h"
#include "xmem.h"
#include "ascii.h"
#include "vec.h"

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

Str *sb_new()
{
    Str *rv = cc_malloc(sizeof(Str));
    rv->len = 0;
    rv->alloc = 8;
    rv->offset = 0;
    rv->str = cc_malloc(rv->alloc * sizeof(char));
    rv->str[rv->len] = '\0';
    return rv;
}

Str *sb_news(char * str)
{
    Str *rv = sb_new();
    sb_adds(rv, str);
    return rv;
}

static void sb_grow(Str *s)
{
    s->alloc *= 2;
    s->str = cc_realloc(s->str, s->alloc * sizeof(char));
}

void sb_addc(Str *s, char c)
{
    if (!c) {
        return;
    }
    if ((s->len + 2) == s->alloc) {
        sb_grow(s);
    }
    s->str[s->len++] = c;
    s->str[s->len] = '\0';
}

void sb_adds(Str *s, char *news)
{
    if (!news) {
        return;
    }
    for (size_t i = 0; news[i]; i++) {
        sb_addc(s, news[i]);
    }
}

Str *sb_copy(Str *what)
{
    Str *res = sb_new();
    sb_adds(res, what->str);
    return res;
}

Str *sb_left(Str *from, size_t much)
{
    assert(from && from->str);

    Str *res = sb_new();
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

Str *sb_right(Str *from, size_t much)
{
    assert(from && from->str);

    Str *res = sb_new();
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

Str *sb_mid(Str *from, size_t begin, size_t much)
{
    assert(from && from->str);

    Str *res = sb_new();
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

Str *sb_trim(Str *from)
{
    assert(from && from->str);

    Str *res = sb_new();
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

static void sb_free(Str ** sb)
{
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

vec * sb_split_char(Str * where, char sep, bool include_empty)
{
    vec * lines = vec_new();
    size_t len = where->len;

    if (len == 0) {
        return lines;
    }

    Str * sb = sb_new();
    for (size_t i = 0; i < len; i++) {
        char c = where->str[i];
        if (c == sep) {
            if (sb->len > 0 || (sb->len == 0 && include_empty)) {
                vec_push(lines, cc_strdup(sb->str));
            }

            sb_free(&sb);
            sb = sb_new();

            continue;
        }
        sb_addc(sb, c);
    }

    if (sb->len > 0 || (sb->len == 0 && include_empty)) {
        vec_push(lines, cc_strdup(sb->str));
    }

    sb_free(&sb);
    return lines;
}

// Выражение Значение  Тип
// СтрЗаменить("012", "", ".") "012" Строка
// СтрЗаменить("012", "", "")  "012" Строка
// СтрЗаменить("012", неопределено, ".") "012" Строка
// СтрЗаменить("012", неопределено, неопределено)  "012" Строка
// СтрЗаменить("012", "0", неопределено) "12"  Строка

static bool next_is(Str * input, char * pattern, size_t input_len, size_t begin_index,
        size_t end_index, size_t patternLen)
{
    if (end_index > input_len) {
        return false;
    }
    Str * substring = sb_mid(input, begin_index, patternLen);
    return strcmp(substring->str, pattern) == 0;
}

Str * sb_replace(Str * input, char * pattern, char *replacement)
{
    if (input == NULL) {
        // it is more clear and simple to return empty value instead of null or exception.
        // because otherwise you should check the return value that it isn't null, and so on.
        // who cares about that? null or not null, we return empty string here.
        // and we'll work with this empty string in invocation point instead of that null.
        return sb_new();
    }

    if (pattern == NULL || strlen(pattern) == 0) {
        return sb_news(input->str);
    }
    size_t input_len = input->len;
    size_t pattern_len = strlen(pattern);
    size_t repl_len = strlen(replacement);

    Str * sb = sb_new();

    for (size_t offset = 0; offset < input_len;) {
        size_t end_index = pattern_len + offset;
        if (next_is(input, pattern, input_len, offset, end_index, pattern_len)) {
            if (repl_len > 0) {
                sb_adds(sb, replacement);
            }
            offset += pattern_len;
        } else {
            sb_addc(sb, input->str[offset]);
            offset++;
        }
    }
    return sb;
}

int sb_nextc(Str *buf)
{
    assert(buf && buf->str);
    if (buf->offset >= buf->len) {
        return -1;
    }
    return buf->str[buf->offset++];
}

int sb_peekc(Str *buf)
{
    assert(buf && buf->str);
    if (buf->offset >= buf->len) {
        return -1;
    }
    return buf->str[buf->offset];
}

Str *normalize_slashes(char *s)
{
    Str *sb = sb_new();
    size_t len = strlen(s);

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
                sb_addc(sb, '/');
                continue;
            }
        }
        sb_addc(sb, c);
        p = c;
    }

    return sb;
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

// public static String normalize(final String given) {
//    final String tmp = normalizeSlashes(given);
//    final List<String> splitten = strSplitChar(tmp, '/', true);
//
//    List<String> worklist = new ArrayList<String>();
//    if (isAbsoluteUnix(tmp)) {
//      worklist.add("/");
//    }
//
//    for (int i = 0; i < splitten.size(); i++) {
//      String part = splitten.get(i).trim();
//
//      if (part.isEmpty()) {
//        continue;
//      }
//      if (part.equals(".")) { // ./
//        continue;
//      }
//
//      if (part.equals("..")) { // ../
//        if (!worklist.isEmpty()) {
//          final int lastIndex = worklist.size() - 1;
//          String last = worklist.get(lastIndex);
//          if (!isAbsolutePath(last)) {
//            worklist.remove(lastIndex);
//            continue;
//          }
//        }
//      }
//
//      if (i < splitten.size() - 1) {
//        part += "/";
//      }
//      worklist.add(part);
//    }
//
//    StringBuilder sb = new StringBuilder();
//    for (String s : worklist) {
//      sb.append(s);
//    }
//    return sb.toString();
// }

char *normalize(char *given)
{
    Str * tmp = normalize_slashes(given);
    vec * splitten = sb_split_char(tmp, '/', 1);

    vec * worklist = vec_new();
    if (is_abs_unix(tmp->str)) {
        vec_push(worklist, cc_strdup("/"));
    }

    for (size_t i = 0; i < splitten->size; i++) {
        Str *part = sb_news(vec_get(splitten, i));
        part = sb_trim(part);
        if (part->len == 0) {
            continue;
        }
        if (part->len == 1 && part->str[0] == '.') {
            continue;
        }
        if (strequal(part->str, "..")) {
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
        vec_push(worklist, cc_strdup(part->str));
    }

    Str * sb = sb_new();
    for (size_t i = 0; i < worklist->size; i++) {
        char *s = vec_get(worklist, i);
        sb_adds(sb, s);
    }
    return sb->str;
}

