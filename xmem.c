#include "xmem.h"

void internal_fatal(const char *_file, int _line, const char *_func,
		const char *fmt, ...) {
	va_list vl;
	char buffer[512];

	va_start(vl, fmt);
	vsprintf(buffer, fmt, vl);
	va_end(vl);

	fprintf( stderr, "FATAL: (%s:[%5d]:%s()) : %s\n", _file, _line, _func,
			buffer);
	exit(128);
}

void* intrernal_realloc(void *ptr, size_t newsize, const char *file, int line) {
	assert(newsize);
	assert(newsize <= INT_MAX);

	void *ret = NULL;
	ret = realloc(ptr, newsize);
	if (ret == NULL) {
		ret = realloc(ptr, newsize);
		if (ret == NULL) {
			ret = realloc(ptr, newsize);
		}
	}

	if (ret == NULL) {
		cc_fatal("OOM realloc fail: %s:%d\n", file, line);
	}
	return ret;
}

void* internal_malloc(size_t size, const char *file, int line) {
	assert(size);
	assert(size <= INT_MAX);

	void *ret = NULL;
	ret = calloc(1u, size);
	if (ret == NULL) {
		ret = calloc(1u, size);
		if (ret == NULL) {
			ret = calloc(1u, size);
		}
	}

	if (ret == NULL) {
		cc_fatal("OOM malloc fail: %s:%d\n", file, line);
	}
	return ret;
}

char* internal_strdup(char *str, const char *file, int line) {
	assert(str);
	size_t newlen = strlen(str) + 1;
	char *newstr = (char*) cc_malloc(newlen);
	strcpy(newstr, str);
	return newstr;
}
