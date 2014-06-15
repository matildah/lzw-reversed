/* decompresses the LZW compression used by this awful software.

   copyright (c) 2014 Matilda Helou <heinousbutch@gmail.com>. 
   licensed under the terms of the ISC license 
   */

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#define MAXSYMBOLS 4096

struct lzwctx {
    /* buffers */
    uint64_t *dict;
    /* dict is arranged as such:
       [uint64_t databyte] [uint64_t pointer] */

    uint8_t *obuf;
    size_t obuf_len;
    size_t obuf_idx;

    /* decoder state */
    int numsymbols;
    int symbolwidth; /* no, you can't derive this from numsymbols */
    int lastsymbol;

    /* status flags */
    int overfill;
    int firstrun;

    /* accumulator state */
    uint32_t accumulator;
    int bits_in_accumulator;
};

struct lzwctx *
initLZW()
{
    struct lzwctx *lzw;
    int i;

    lzw = malloc(sizeof(struct lzwctx));
    assert(NULL != lzw);

    lzw->dict = malloc(MAXSYMBOLS * 2 * 8);
    assert(NULL != lzw->dict);

    lzw->obuf = malloc(8192);
    assert(NULL != lzw->obuf);

    lzw->numsymbols = 256;
    lzw->symbolwidth = 9;

    lzw->overfill = 0;
    lzw->firstrun = 1;

    lzw->accumulator = 0;
    lzw->bits_in_accumulator = 0;

    for (i = 0; i < MAXSYMBOLS; i++) {
        if (i < 256) {
            *(lzw->dict + i * 2) = i;               /* byte */
        } else {
            *(lzw->dict + i * 2) = 0;               /* byte */
        }

        *(lzw->dict + i * 2 + 1) = 0;           /* pointer */
    }

    return lzw;
}

int main()
{
    struct lzwctx * foo;
    foo = initLZW();
}

