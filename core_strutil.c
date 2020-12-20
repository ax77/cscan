#include "core_strutil.h"
#include "core_mem.h"

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

StringBuilder *sb_new()
{
    StringBuilder *rv = malloc(sizeof(StringBuilder));
    rv->len = 0;
    rv->alloc = 8;
    rv->str = malloc(rv->alloc * sizeof(char));
    rv->str[rv->len] = '\0';
    return rv;
}

StringBuilder *sb_news(char * str)
{
    StringBuilder *rv = sb_new();
    sb_adds(rv, str);
    return rv;
}

static void sb_grow(StringBuilder *s)
{
    s->alloc *= 2;
    s->str = realloc(s->str, s->alloc * sizeof(char));
}

void sb_addc(StringBuilder *s, char c)
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

void sb_adds(StringBuilder *s, char *news)
{
    if (!news) {
        return;
    }
    for (size_t i = 0; news[i]; i++) {
        sb_addc(s, news[i]);
    }
}

StringBuilder *sb_copy(StringBuilder *what)
{
    StringBuilder *res = sb_new();
    sb_adds(res, what->str);
    return res;
}

StringBuilder *sb_left(StringBuilder *from, size_t much)
{
    assert(from && from->str);

    StringBuilder *res = sb_new();
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

StringBuilder *sb_right(StringBuilder *from, size_t much)
{
    assert(from && from->str);

    StringBuilder *res = sb_new();
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

StringBuilder *sb_mid(StringBuilder *from, size_t begin, size_t much)
{
    assert(from && from->str);

    StringBuilder *res = sb_new();
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

StringBuilder *sb_trim(StringBuilder *from)
{
    assert(from && from->str);

    StringBuilder *res = sb_new();
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

static void sb_free(StringBuilder ** sb)
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

static bool strequal(void *a, void *b)
{
    char *str_1 = (char*) a;
    char *str_2 = (char*) b;
    return strcmp(str_1, str_2) == 0;
}

LinkedList * sb_split_char(StringBuilder * where, char sep, bool include_empty)
{
    LinkedList * lines = list_new(strequal);
    size_t len = where->len;

    if (len == 0) {
        return lines;
    }

    StringBuilder * sb = sb_new();
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

//  public boolean nextIs(final String input, final String pattern, int inputLen, int beginIndex, int endIndex) {
//    if (endIndex > inputLen) {
//      return false;
//    }
//    final String substring = input.substring(beginIndex, endIndex);
//    return substring.equals(pattern);
//  }
//
//  //  Выражение Значение  Тип
//  //  СтрЗаменить("012", "", ".") "012" Строка
//  //  СтрЗаменить("012", "", "")  "012" Строка
//  //  СтрЗаменить("012", неопределено, ".") "012" Строка
//  //  СтрЗаменить("012", неопределено, неопределено)  "012" Строка
//  //  СтрЗаменить("012", "0", неопределено) "12"  Строка
//
//  private String str_replace(final String input, final String pattern, final String replacement) {
//
//    if (input == null) {
//      // it is more clear and simple to return empty value instead of null or exception.
//      // because otherwise you should check the return value that it isn't null, and so on.
//      // who cares about that? null or not null, we return empty string here.
//      // and we'll work with this empty string in invocation point instead of that null.
//      return "";
//    }
//
//    if (pattern == null || pattern.length() == 0) {
//      return new String(input);
//    }
//
//    String replacementInternal = replacement; // only to keep parameter as 'final'
//    if (replacementInternal == null) {
//      replacementInternal = "";
//    }
//
//    final int inputLen = input.length();
//    final int patternLen = pattern.length();
//    final int replacementLen = replacementInternal.length();
//
//    StringBuilder sb = new StringBuilder();
//
//    for (int offset = 0; offset < inputLen;) {
//      final int endIndex = patternLen + offset;
//      if (nextIs(input, pattern, inputLen, offset, endIndex)) {
//        if (replacementLen > 0) {
//          sb.append(replacementInternal);
//        }
//        offset += patternLen;
//      } else {
//        sb.append(input.charAt(offset));
//        offset++;
//      }
//    }
//    return sb.toString();
//  }

//  public boolean nextIs(final String input, final String pattern, int inputLen, int beginIndex, int endIndex) {
//    if (endIndex > inputLen) {
//      return false;
//    }
//    final String substring = input.substring(beginIndex, endIndex);
//    return substring.equals(pattern);
//  }

static bool next_is(StringBuilder * input, char * pattern, size_t input_len, size_t begin_index,
        size_t end_index, size_t patternLen)
{
    if (end_index > input_len) {
        return false;
    }
    StringBuilder * substring = sb_mid(input, begin_index, patternLen);
    return strcmp(substring->str, pattern) == 0;
}

StringBuilder * sb_replace(StringBuilder * input, char * pattern, char *replacement)
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
    size_t inputLen = input->len;
    size_t patternLen = strlen(pattern);
    size_t replacementLen = strlen(replacement);

    StringBuilder * sb = sb_new();

    for (size_t offset = 0; offset < inputLen;) {
        size_t endIndex = patternLen + offset;
        if (next_is(input, pattern, inputLen, offset, endIndex, patternLen)) {
            if (replacementLen > 0) {
                sb_adds(sb, replacement);
            }
            offset += patternLen;
        } else {
            sb_addc(sb, input->str[offset]);
            offset++;
        }
    }
    return sb;
}

