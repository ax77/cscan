#ifndef CORE_STRUTIL_H_
#define CORE_STRUTIL_H_

#include "cheaders.h"

int strstarts(char *what, char *with);
int strends(char *what, char *with);
char *pathnormalize(char *where);

#endif /* CORE_STRUTIL_H_ */
