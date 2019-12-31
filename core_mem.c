#include "core_mem.h"

void internal_fatal(const char *_file, int _line, const char *_func, const char *fmt, ...) {
  va_list vl;
  char buffer[512];

  va_start(vl, fmt);
  vsprintf(buffer, fmt, vl);
  va_end(vl);

  fprintf( stderr, "FATAL: (%s:[%5d]:%s()) : %s\n", _file, _line, _func,
      buffer);
  exit(128);
}

void *intrernal_realloc(void *ptr, size_t size, const char *file, int line) {
  void *ret = realloc(ptr, size);
  if (!ret && !size) {
    ret = realloc(ptr, 1);
    if (!ret) {
      cc_fatal("OOM realloc fail: %s:%d\n", file, line);
    }
  }
  return ret;
}

void *internal_malloc(size_t size, const char *file, int line) {
  void *ret = malloc(size);
  if (!ret && !size) {
    ret = malloc(1);
    if (!ret) {
      cc_fatal("OOM malloc fail: %s:%d\n", file, line);
    }
  }
  return ret;
}