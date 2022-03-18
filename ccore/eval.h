#ifndef CCORE_EVAL_H_
#define CCORE_EVAL_H_

#include <stddef.h>

size_t eval_integer(char *dec, unsigned base);
double eval_float_16(char *dec, char *mnt, char *exp, char sig);
double eval_float_10(char *dec, char *mnt, char *exp, char sig);

#endif /* CCORE_EVAL_H_ */
