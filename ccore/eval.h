#ifndef CCORE_EVAL_H_
#define CCORE_EVAL_H_

long evdecimal(char *n, unsigned base);
double evalhexfloat(char *dec, char *mnt, char *exp, char sig);
double evaldecfloat(char *dec, char *mnt, char *exp, char sig);

#endif /* CCORE_EVAL_H_ */
