#ifndef CCORE_EVAL_H_
#define CCORE_EVAL_H_

#include <stddef.h>

size_t evdecimal(char *dec, unsigned base);
double evalhexfloat(char *dec, char *mnt, char *exp, char sig);
double evaldecfloat(char *dec, char *mnt, char *exp, char sig);

#endif /* CCORE_EVAL_H_ */
