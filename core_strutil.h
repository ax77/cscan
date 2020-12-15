#ifndef CORE_STRUTIL_H_
#define CORE_STRUTIL_H_

#include "cheaders.h"

typedef struct strbuilder StrBuilder;

struct strbuilder {
    size_t len, alloc;
    char *str;
};

StrBuilder *sb_new();
StrBuilder *sb_news(char * str);
void sb_addc(StrBuilder *s, char c);
void sb_adds(StrBuilder *s, char *news);
StrBuilder *sb_copy(StrBuilder *what);
StrBuilder *sb_left(StrBuilder *from, size_t much);
StrBuilder *sb_right(StrBuilder *from, size_t much);
StrBuilder *sb_mid(StrBuilder *from, size_t begin, size_t much);
StrBuilder *sb_trim(StrBuilder *from);

int strstarts(char *what, char *with);
int strends(char *what, char *with);

#endif /* CORE_STRUTIL_H_ */
