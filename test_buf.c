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
#include "core_buf.h"

int main(int argc, char **argv) {
	char *s = "a\\\nb\\\nc";
	Cbuffer *buf = ccbuf_new(s);

	int *lookup = next4(buf);
	printf("%c ", lookup[0]);
	printf("%c ", lookup[1]);
	printf("%c ", lookup[2]);
	printf("%c ", lookup[3]);

	printf("\n:ok:\n");
	return 0;
}
