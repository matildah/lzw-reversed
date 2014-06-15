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
    unsigned int numsymbols;
    unsigned int symbolwidth; /* no, you can't derive this from numsymbols */
    unsigned int lastsymbol;

    /* status flags */
    int overfill;
    int firstrun;
    int eof_reached;

    /* accumulator state */
    uint32_t accumulator;
    unsigned int bits_in_accumulator;
};

struct lzwctx *
initLZW(char *path)
{
    struct lzwctx *lzw;
    int i;

    lzw = malloc(sizeof(struct lzwctx));
    assert(NULL != lzw);

    lzw->fp = fopen(path, "r");
    assert(NULL != lzw->fp);

    lzw->dict = malloc(MAXSYMBOLS * 2 * 8);
    assert(NULL != lzw->dict);

    lzw->obuf = malloc(OBUF_LEN);
    assert(NULL != lzw->obuf);

    lzw->obuf_len = 0;
    lzw->obuf_idx = 0;

    lzw->numsymbols = 256;
    lzw->symbolwidth = 9;
    lzw->lastsymbol = 0;

    lzw->overfill = 0;
    lzw->firstrun = 1;
    lzw->eof_reached = 0;

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

void
obuf_push(uint8_t in, struct lzwctx *ctx)
{
    assert(ctx->obuf_len != OBUF_LEN - 1);
    *(ctx->obuf + ctx->obuf_len) = in;
    ctx->obuf_len++;
}

unsigned int
pullsymbol(struct lzwctx *ctx)
{
    unsigned int inbyte;
    uint32_t acctemp;

    if (ctx->bits_in_accumulator < ctx->symbolwidth) { 
        /* not enough bits in the accumulator, gotta reload! */
        while (ctx->bits_in_accumulator < ctx->symbolwidth) {
            
            inbyte = fgetc(ctx->fp);
            if (EOF == inbyte) {
                ctx->eof_reached = 1;
                return 0;
            }

            /* bits come in through the right */
            ctx->accumulator = ctx->accumulator << 8 | inbyte; 
            ctx->bits_in_accumulator += 8;
        }
    }

    /* now that we have enough bits in our accumulator, we take a symbol out */
    acctemp = ctx->accumulator;

    ctx->accumulator &= ((1 << ctx->bits_in_accumulator) - 1);
    ctx->bits_in_accumulator -= ctx->symbolwidth;

    return acctemp >> ctx->bits_in_accumulator;
}
        
int main()
{
    struct lzwctx * foo;
    foo = initLZW("bar");
}

