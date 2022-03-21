#ifndef XMEM_H_
#define XMEM_H_

#include "hdrs.h"

#define cc_fatal(fmt, ...) internal_fatal(__FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
void internal_fatal(const char *_file, int _line, const char *_function, const char *fmt, ...);

#define cc_realloc(ptr, size) intrernal_realloc(ptr, size, __FILE__, __LINE__)
void *intrernal_realloc(void *ptr, size_t size, const char *file, int line);

#define cc_malloc(size) internal_malloc(size, __FILE__, __LINE__)
void *internal_malloc(size_t size, const char *file, int line);

#define cc_strdup(str) internal_strdup(str, __FILE__, __LINE__)
char *internal_strdup(char *str, const char *file, int line);

#define cc_free(ptr) internal_free(ptr, __FILE__, __LINE__)
void internal_free(void *ptr, const char *file, int line);

#endif
