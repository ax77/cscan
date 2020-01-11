#include "core_file.h"

static int is_dir(const char* target)
{
   struct stat statbuf;
   stat(target, &statbuf);
   return S_ISDIR(statbuf.st_mode);
}

static int fileinfo(char *given, size_t *size, int *exists) {
    FILE *fp = fopen(given, "rb");
    if(fp) {
        fseek(fp, 0, SEEK_END);
        *size = ftell(fp);
        *exists = 1;
        
        fclose(fp);
        return 1;
    }
    return 0;
}

struct FileWrapper *FileWrapper_new(char *given) {
    struct FileWrapper *file = malloc(sizeof(struct FileWrapper));
    assert(file);
    
    size_t size = 0;
    int exists = 0;
    int status = fileinfo(given, &size, &exists);
    
    //file->from = strdup(given);
    file->size = size;
    file->exists = exists;
    
    file->isdir = 0;
    if(status) {
        //file->isdir = is_dir(given);
    }
    
    return file;
}

char* readfile(const char *filename, size_t *szout) {
	FILE *fp = NULL;
	size_t n, sz;

	char *data = NULL;
	fp = fopen(filename, "rb");
	assert(fp && "file does not exists.");

	fseek(fp, 0, SEEK_END);
	sz = ftell(fp);
	rewind(fp);

	*szout = sz;

	data = malloc(sz + 1);
	assert(data && "malloc fail");

	data[sz] = '\0';
	n = fread(data, 1, sz, fp);
	if (n != sz) {
		goto fail;
	}

	fclose(fp);
	return data;

	fail: if (fp) {
		fclose(fp);
	}
	if (data) {
		free(data);
	}

	assert(0);
	return NULL;
}

