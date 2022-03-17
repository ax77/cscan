#ifndef FDESC_H_
#define FDESC_H_

#include "hdrs.h"

char* hb_readfile(const char *filename, size_t *szout);

int hb_read_byte(int fd);
int hb_open(char *filename);
int hb_close(int fd);
int hb_check_is_file(int fd);
ssize_t hb_read_bytes(int fd, void *buffer, size_t count);

#endif
