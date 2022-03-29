#ifndef STR_H_
#define STR_H_

#include "hdrs.h"
#include "vec.h"

#define STR_INIT VEC_INIT(i8)

int sb_addc(Str *s, char c);
size_t sb_adds(Str *s, char *news);
Str* sb_new();
Str* sb_news(char *str);
void sb_reset(Str *s);
char* sb_left(char *from, size_t much);
char* sb_right(char *from, size_t much);
char* sb_mid(char *from, size_t begin, size_t much);
char* sb_trim(char *from);
char* sb_replace(char *input, char *pattern, char *replacement);
char* normalize_slashes(char *s);
int is_abs_win(char *s);
int is_abs_unix(char *s);
int is_abs_path(char *s);
int strstarts(char *what, char *with);
int strends(char *what, char *with);

vec(str)* sb_split_char(char *where, char sep, int include_empty);
char* normalize(char *given);
int strequal(void *a, void *b);
char* sb_buf_or_empty(Str *sb);

int sb_pop(Str *buf);
int sb_adds_rev(Str *buf, char *input);
int sb_is_empty(Str *buf);
int sb_peek_last(Str *buf);
int sb_char_at(Str *buf, size_t index);

ptrdiff_t sb_find(char *s, char *p);
vec(str) *sb_split_str(char *input, char *sep, int include_empty);

#endif /* STR_H_ */

