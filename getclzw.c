/* decompresses the LZW compression used by this awful software.

   copyright (c) 2014 Matilda Helou <heinousbutch@gmail.com>. 
   licensed under the terms of the ISC license 
   */

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#define MAXSYMBOLS 4096
#define OBUF_LEN 8192

struct lzwctx {
    FILE *fp;

    /* buffers */
    uint64_t *dict;
    /* dict is arranged as such:
       [uint64_t databyte] [uint64_t pointer] */

    uint8_t *obuf;
    size_t obuf_len; /* how many bytes we've put in */
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
initLZW(char *path)
{
    struct lzwctx *lzw;
    int i;

    lzw = malloc(sizeof(struct lzwctx));
    assert(NULL != lzw);

    lzw->dict = malloc(MAXSYMBOLS * 2 * 8);
    assert(NULL != lzw->dict);

    lzw->obuf = malloc(OBUF_LEN);
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
    
    lzw->fp = fopen(path, "r");
    assert(NULL != lzw->fp);

    return lzw;
}

void
obuf_push(uint8_t in, struct lzwctx *ctx)
{
    assert(ctx->obuf_len != OBUF_LEN - 1);
    *(ctx->obuf + ctx->obuf_len) = in;
    ctx->obuf_len++;
}

        
int main()
{
    struct lzwctx * foo;
    foo = initLZW();
}

