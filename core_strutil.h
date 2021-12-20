#ifndef CORE_STRUTIL_H_
#define CORE_STRUTIL_H_

#include "cheaders.h"
#include "core_list.h"

typedef struct strbuilder StringBuilder;

struct strbuilder {
    size_t len, alloc, offset;
    char *str;
};

StringBuilder *sb_new();
StringBuilder *sb_news(char * str);
void sb_addc(StringBuilder *s, char c);
void sb_adds(StringBuilder *s, char *news);
StringBuilder *sb_copy(StringBuilder *what);
StringBuilder *sb_left(StringBuilder *from, size_t much);
StringBuilder *sb_right(StringBuilder *from, size_t much);
StringBuilder *sb_mid(StringBuilder *from, size_t begin, size_t much);
StringBuilder *sb_trim(StringBuilder *from);

LinkedList * sb_split_char(StringBuilder * where, char sep, bool include_empty); // list_of(char*)
StringBuilder * sb_replace(StringBuilder * input, char *pattern, char *replacement);

// when we have to iterate over characters
// and save the state
int sb_nextc(struct strbuilder *buf);
int sb_peekc(struct strbuilder *buf);

int strstarts(char *what, char *with);
int strends(char *what, char *with);

#endif /* CORE_STRUTIL_H_ */
