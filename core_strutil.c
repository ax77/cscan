#include "core_strutil.h"

struct strbuilder *pathnormalize(char *where) {
  assert(where);
  
  size_t slen = strlen(where);
  struct strbuilder *out = sb_new();
  
  if(slen == 0) {
      sb_adds(out, "./");
      return out;
  }
  
  int prevc = '\0';
  for(size_t i = 0; where[i]; i++) {
    int c = where[i];
    if(c == '\\' || c == '/') {
      if(prevc == '\\' || prevc == '/') {
        prevc = c;
        continue;
      }
    }
    int nc = (c == '\\' ? '/' : c);
    sb_addc(out, nc);
    prevc = c;
  }
  
  return out;
}

int strstarts(char *what, char *with) {
	assert(what);
	assert(with);

	size_t L1 = strlen(what);
	size_t L2 = strlen(with);
	if (L1 == 0 || L2 == 0 || (L2 > L1)) {
		return 0;
	}

	for (size_t i = 0; i < L2; i++) {
		int c1 = what[i];
		int c2 = with[i];
		if (c1 != c2) {
			return 0;
		}
	}
	return 1;
}

int strends(char *what, char *with) {
	assert(what);
	assert(with);

	size_t L1 = strlen(what);
	size_t L2 = strlen(with);
	if (L1 == 0 || L2 == 0 || (L2 > L1)) {
		return 0;
	}

	for (ptrdiff_t i = L1, j = L2; --i >= 0 && --j >= 0;) {
		int c1 = what[i];
		int c2 = with[j];
		if (c1 != c2) {
			return 0;
		}
	}
	return 1;
}

struct strbuilder *sb_new() {
  struct strbuilder *rv = malloc(sizeof(struct strbuilder));
  rv->len = 0;
  rv->alloc = 8;
  rv->str = malloc(rv->alloc * sizeof(char));
  rv->str[rv->len] = '\0';
  return rv;
}

static void sb_grow(struct strbuilder *s) {
  s->alloc *= 2;
  s->str = realloc(s->str, s->alloc * sizeof(char));
}

void sb_addc(struct strbuilder *s, char c) {
  if (!c) {
    return;
  }
  if ((s->len + 2) == s->alloc) {
    sb_grow(s);
  }
  s->str[s->len++] = c;
  s->str[s->len] = '\0';
}

void sb_adds(struct strbuilder *s, char *news) {
  if (!news) {
    return;
  }
  for (size_t i = 0; news[i]; i++) {
    sb_addc(s, news[i]);
  }
}

