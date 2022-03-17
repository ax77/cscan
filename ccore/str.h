#ifndef STR_H_
#define STR_H_

#include "hdrs.h"
#include "vec.h"

typedef struct strbuilder Str;

struct strbuilder {
    size_t len, alloc, offset;
    char *str;
};

Str *sb_new();
Str *sb_news(char * str);
void sb_addc(Str *s, char c);
void sb_adds(Str *s, char *news);
Str *sb_copy(Str *what);
Str *sb_left(Str *from, size_t much);
Str *sb_right(Str *from, size_t much);
Str *sb_mid(Str *from, size_t begin, size_t much);
Str *sb_trim(Str *from);

vec * sb_split_char(Str * where, char sep, bool include_empty); // list_of(char*)
Str * sb_replace(Str * input, char *pattern, char *replacement);

// when we have to iterate over characters
// and save the state
int sb_nextc(Str *buf);
int sb_peekc(Str *buf);

int strstarts(char *what, char *with);
int strends(char *what, char *with);

static int strequal(void *a, void *b)
{
    char *str_1 = (char*) a;
    char *str_2 = (char*) b;
    return strcmp(str_1, str_2) == 0;
}

char *normalize(char *given);
Str *normalize_slashes(char *s);
int is_abs_win(char *s);
int is_abs_unix(char *s);
int is_abs_path(char *s);

#endif /* STR_H_ */
