#ifndef CORE_BUF_H_
#define CORE_BUF_H_

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HC_FEOF (-1)

typedef struct ccbuf Cbuffer;

struct ccbuf {
    char *buf;
    size_t size, offset;
    size_t line, column;
    int prevc, eofs;
};

Cbuffer *ccbuf_new(char *from);
int nextc(Cbuffer *b);
int* next4(Cbuffer *buf);

#endif /* CORE_BUF_H_ */
