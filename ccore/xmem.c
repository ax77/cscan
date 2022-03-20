#include "xmem.h"

void internal_fatal(const char *_file, int _line, const char *_func,
		const char *fmt, ...) {
    va_list args;
    static char buffer[512];

    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

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
	size_t len = strlen(str) + 1;
	char *newstr = (char*) cc_malloc(len);
	strcpy(newstr, str);
	newstr[len - 1] = '\0';
	//assert(strcmp(str, newstr) == 0);
	return newstr;
}

