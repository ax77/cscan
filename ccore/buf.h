#ifndef BUF_H_
#define BUF_H_

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

typedef struct char_buf CharBuf;

struct char_buf {
    char *buf;
    size_t size, offset;
    size_t line, column;
    int prevc, eofs;
};

CharBuf *charbuf_new(char *from);
int charbuf_nextc(CharBuf *b);
int charbuf_peekc(CharBuf *b);
int* charbuf_next4(CharBuf *buf);

#endif /* BUF_H_ */
