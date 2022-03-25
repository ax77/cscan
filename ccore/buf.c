#include "buf.h"
#include "xmem.h"

#define BUFFER_PADDING (32)

CharBuf* charbuf_new(char *from)
{
    assert(from);

    CharBuf *r = cc_malloc(sizeof(CharBuf));
    size_t buflen = strlen(from);

    // +32 : some little padding, when we check the buffer like this: buffer[index + 2].
    size_t alloclen = (buflen + BUFFER_PADDING) * sizeof(char);

    r->buf = (char*) cc_malloc(alloclen);
    for (size_t j = 0; j < alloclen; j++) {
        r->buf[j] = '\0';
    }

    // Ignore the BOM, if any.
    size_t offset = 0;
    if (buflen > 3) {
        if (from[0] == 0xef && from[1] == 0xbb && from[2] == 0xbf) {
            offset = 3;
        }
    }
    for (size_t i = offset, j = 0; i < buflen && from[i]; i++, j++) {
        r->buf[j] = from[i];
    }

    r->size = buflen;
    r->offset = 0;

    r->line = 1;
    r->column = 0;

    r->prevc = 0;
    r->eofs = -1;

    return r;
}

int charbuf_nextc(CharBuf *b)
{

    for (; b->offset < b->size;) {

        if (b->eofs >= (BUFFER_PADDING - 8)) {
            cc_fatal("Infinite loop handling...");
        }

        if (b->prevc == '\n') {
            b->line++;
            b->column = 0;
        }

        // we'are able to ignore the IOOB, because our buffer has a nice padding \0
        int c1 = b->buf[b->offset + 0];
        int c2 = b->buf[b->offset + 1];
        int c3 = b->buf[b->offset + 2];

        // let's handle the line-joining
        //
        if (c1 == '\\') {
            if (c2 == '\r') {
                if (c3 == '\n') {
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
            if (c2 == '\n') {
                b->offset += 2;
                b->prevc = '\n';
                continue;
            }
        }

        if (c1 == '\r') {
            if (c2 == '\n') {
                // DOS: [\r][\n]
                b->offset += 2;
            } else {
                // OSX: [\r]
                b->offset += 1;
            }
            b->prevc = '\n';
            return '\n';
        }

        // get the char at the current offset
        //
        int next = b->buf[b->offset++];
        b->prevc = next;

        if (next == '\t') {
            b->column += 4;
        } else {
            b->column += 1;
        }

        if (next == '\0') {
            b->eofs++;
            return HC_FEOF; // '\0';
        }

        return next;
    }

    return HC_FEOF;
}

int* charbuf_next4(CharBuf *buf)
{
    assert(buf);

    size_t size = buf->size;
    size_t offset = buf->offset;
    size_t line = buf->line;
    size_t column = buf->column;
    int prevc = buf->prevc;
    int eofs = buf->eofs;

    static int lookup[4] = { -1, -1, -1, -1 };
    lookup[0] = charbuf_nextc(buf);
    lookup[1] = charbuf_nextc(buf);
    lookup[2] = charbuf_nextc(buf);
    lookup[3] = charbuf_nextc(buf);

    buf->size = size;
    buf->offset = offset;
    buf->line = line;
    buf->column = column;
    buf->prevc = prevc;
    buf->eofs = eofs;

    return lookup;
}

int charbuf_peekc(CharBuf *buf)
{
    assert(buf);

    size_t size = buf->size;
    size_t offset = buf->offset;
    size_t line = buf->line;
    size_t column = buf->column;
    int prevc = buf->prevc;
    int eofs = buf->eofs;

    int c = charbuf_nextc(buf);

    buf->size = size;
    buf->offset = offset;
    buf->line = line;
    buf->column = column;
    buf->prevc = prevc;
    buf->eofs = eofs;

    return c;
}

