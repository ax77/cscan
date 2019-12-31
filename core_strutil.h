#ifndef CORE_STRUTIL_H_
#define CORE_STRUTIL_H_

#include "cheaders.h"

int strstarts(char *what, char *with);
int strends(char *what, char *with);
struct strbuilder *pathnormalize(char *where);

struct strbuilder {
  size_t len, alloc;
  char *str;
};

struct strbuilder *sb_new();
void sb_addc(struct strbuilder *s, char c);
void sb_adds(struct strbuilder *s, char *news);
struct strbuilder *sb_left(struct strbuilder *from, size_t much) ;
struct strbuilder *sb_right(struct strbuilder *from, size_t much) ;
struct strbuilder *sb_mid(struct strbuilder *from, size_t begin, size_t much);

#endif /* CORE_STRUTIL_H_ */
