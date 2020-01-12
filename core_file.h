#ifndef CORE_FILE_H
#define CORE_FILE_H

#include "cheaders.h"

// from         = "C:\\//_tmp7\\\\astyle.options"
// name         = "astyle.options"
// basename     = "astyle"
// fullname     = "C:/_tmp7/astyle.options"
// path         = "C:/_tmp7/"
// extension    = ".options"

struct FileWrapper {
    char *from;

    struct strbuilder *name;
    struct strbuilder *basename;
    struct strbuilder *fullname;
    struct strbuilder *path;
    struct strbuilder *extension;

    size_t size;
    int isdir, exists;
};

struct FileWrapper *FileWrapper_new(char *given);
char *readfile(const char *filename, size_t *szout);

#endif
