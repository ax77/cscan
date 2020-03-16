#include "core_buf.h"
#include "core_mem.h"

static size_t min(size_t a, size_t b) {
    if (a < b) {
        return a;
    }
    return b;
}

Cbuffer *ccbuf_new(char *from) {
    assert(from);

    Cbuffer *r = cc_malloc(sizeof(Cbuffer));

    char * source = cc_strdup(from);
    size_t buflen = strlen(from);

    // +32 : some little padding, when we check the buffer like this: buffer[index + 2].
    size_t alloclen = (buflen + 32) * sizeof(char);

    r->buf = (char*) cc_malloc(alloclen);
    for (size_t j = 0; j < alloclen; j++) {
        r->buf[j] = '\0';
    }

    strcpy(r->buf, source);

    r->size = buflen;
    r->offset = 0;

    r->line = 1;
    r->column = 0;

    r->prevc = 0;
    r->eofs = -1;

    return r;
}

int nextc(Cbuffer *b) {
    // when you build buffer, allocate more space to avoid IOOB check
    // for example: source = { '1', '2', '3', '\0' }, buffer = { '1', '2', '3', '\0', '\0', '\0', '\0', '\0' }

    for (;;) {

        if (b->eofs > 0) {
            cc_fatal("Infinite loop handling...");
        }

        if (b->prevc == '\n') {
            b->line++;
            b->column = 0;
        }

        if (b->buf[b->offset] == '\\') {
            if (b->buf[b->offset + 1] == '\r') {
                if (b->buf[b->offset + 2] == '\n') {
                    // DOS: [\][\r][\n]
                    b->offset += 3;
                } else {
                    // OSX: [\][\r]
                    b->offset += 2;
                }

                b->prevc = '\n';
                continue;
            }

            // UNX: [\][\n]
            if (b->buf[b->offset + 1] == '\n') {
                b->offset += 2;
                b->prevc = '\n';
                continue;
            }
        }

        if (b->buf[b->offset] == '\r') {
            if (b->buf[b->offset + 1] == '\n') {
                // DOS: [\r][\n]
                b->offset += 2;
            } else {
                // OSX: [\r]
                b->offset += 1;
            }
            b->prevc = '\n';
            return '\n';
        }

        if (b->offset == b->size) {
            b->eofs++;
            return HC_FEOF; // '\0';
        }

        int next = b->buf[b->offset++];
        b->column++;
        b->prevc = next;

        if (next == '\0') {
            b->eofs++;
            return HC_FEOF; // '\0';
        }

        return next;
    }

    return HC_FEOF;
}

static int nextis_internal(Cbuffer *b, char *what) {
    // buffer: "adcde"
    // what  : "abcdef"
    // is false positive may be here. because we check first 5 chars, min...
    size_t wlen = strlen(what);
    if (wlen > b->size) {
        return 0; // avoid false positive
    }

    size_t m = min(b->size, wlen);
    for (size_t i = 0; i < m; i++) {
        int exp = what[i];
        int act = nextc(b);
        if (exp != act) {
            return 0;
        }
    }
    return 1;
}

int nextis(Cbuffer *b, char *what) {
    size_t offset_save = b->offset;
    size_t line_save = b->line;
    size_t column_save = b->column;
    int prevc_save = b->prevc;
    int eofs_save = b->eofs;

    int result = nextis_internal(b, what);

    b->offset = offset_save;
    b->line = line_save;
    b->column = column_save;
    b->prevc = prevc_save;
    b->eofs = eofs_save;

    return result;
}

