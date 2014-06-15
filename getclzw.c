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
    size_t obuf_len; /* index for writing */
    size_t obuf_idx; /* index for reading */

    /* decoder state */
    unsigned int numsymbols;
    unsigned int symbolwidth; /* no, you can't derive this from numsymbols */
    unsigned int lastsymbol;
    uint8_t firstbyte_lastsymbol;

    /* status flags */
    int overfill;
    int firstrun;
    int nomoresymbols; /* the semantics of this are slightly different than those of 
                          eof_reached in the original program, we can have reached the
                          EOF of the file but if we still have bits in the accumulator, 
                          nomoresymbols will still be zero */

    /* accumulator state */
    uint32_t accumulator;
    unsigned int bits_in_accumulator;
};

struct lzwctx *
initLZW(char *path)
{
    struct lzwctx *ctx;
    int i;

    ctx = malloc(sizeof(struct lzwctx));
    assert(NULL != ctx);

    ctx->fp = fopen(path, "r");
    assert(NULL != ctx->fp);

    ctx->dict = malloc(MAXSYMBOLS * 2 * 8);
    assert(NULL != ctx->dict);

    ctx->obuf = malloc(OBUF_LEN);
    assert(NULL != ctx->obuf);

    ctx->obuf_len = 0;
    ctx->obuf_idx = 0;

    ctx->numsymbols = 256;
    ctx->symbolwidth = 9;
    ctx->lastsymbol = 0;
    ctx->firstbyte_lastsymbol = 0;

    ctx->overfill = 0;
    ctx->firstrun = 1;
    ctx->nomoresymbols = 0;

    ctx->accumulator = 0;
    ctx->bits_in_accumulator = 0;

    for (i = 0; i < MAXSYMBOLS; i++) {
        if (i < 256) {
            *(ctx->dict + i * 2) = i;               /* byte */
        } else {
            *(ctx->dict + i * 2) = 0;               /* byte */
        }

        *(ctx->dict + i * 2 + 1) = 0;           /* pointer */
    }
    

    return ctx;
}

void
obuf_push(uint8_t in, struct lzwctx *ctx)
{
    assert(ctx->obuf_len != OBUF_LEN - 1);
    *(ctx->obuf + ctx->obuf_len) = in;
    ctx->obuf_len++;
}

uint8_t
obuf_pull(struct lzwctx *ctx)
{
    uint8_t rval;
    assert(ctx->obuf_idx < ctx->obuf_len);

    rval = *(ctx->obuf + ctx->obuf_idx);
    ctx->obuf_idx++;

    if (ctx->obuf_idx == ctx->obuf_len) {
        ctx->obuf_idx = 0;
        ctx->obuf_len = 0;
    }
    return rval;
}

unsigned int
getsymbol(struct lzwctx *ctx)
{
    unsigned int inbyte;
    uint32_t accsave;

    if (ctx->bits_in_accumulator < ctx->symbolwidth) { 
        /* not enough bits in the accumulator, gotta reload! */
        while (ctx->bits_in_accumulator < ctx->symbolwidth) {
            
            inbyte = fgetc(ctx->fp);
            if (EOF == inbyte) {
                ctx->nomoresymbols = 1;
                return -1;
            }

            /* bits come in through the right */
            ctx->accumulator = (ctx->accumulator << 8) | inbyte; 
            ctx->bits_in_accumulator += 8;
        }
    }

    /* now that we have enough bits in our accumulator, we take a symbol out */
    accsave = ctx->accumulator;

    ctx->bits_in_accumulator -= ctx->symbolwidth;
    ctx->accumulator &= ((1 << ctx->bits_in_accumulator) - 1);

    return accsave >> ctx->bits_in_accumulator;
}

void
pullstring(uint64_t *start, uint8_t *firstbyte, struct lzwctx *ctx)
{
    if (0 != *(start + 1)) {
        pullstring((uint64_t *) *(start + 1), firstbyte, ctx);
        obuf_push((uint8_t) *(start), ctx);
    } else {
        obuf_push((uint8_t) *(start), ctx);
        *firstbyte = (uint8_t) *start;
    }
}

uint8_t
tablelookup(unsigned int cursymbol, unsigned int prevsymbol, struct lzwctx *ctx)
{
    uint8_t rval;

    if (cursymbol == ctx->numsymbols) { /* known unknown symbol */
        pullstring(ctx->dict + 2 * prevsymbol, &rval, ctx);
        obuf_push(ctx->firstbyte_lastsymbol, ctx);
    } else if (cursymbol < ctx->numsymbols) { /* known symbol */
        pullstring(ctx->dict + 2 * cursymbol, &rval, ctx);
    } else { /* unknown unknown symbol, take a shit in the bed */
        assert(0);
    }
    ctx->firstbyte_lastsymbol = rval;
    
    return rval;
}




unsigned int
getclzw(struct lzwctx *ctx)
{
    unsigned int symbol, i;
    
    /* handle special cases -- either the first symbol read or an overfill */
    if (1 == ctx->firstrun) {
        ctx->firstrun = 0;
        symbol = getsymbol(ctx);
        if (1 == ctx->nomoresymbols) {
            return -1;
        }
        
        ctx->lastsymbol = symbol;
        tablelookup(symbol, symbol, ctx);
    } else if (1 == ctx->overfill) {
        ctx->overfill = 0;
        /* clear the dictionary */
        for (i = 0; i < MAXSYMBOLS; i++) {
            if (i < 256) {
                *(ctx->dict + i * 2) = i;               /* byte */
            } else {
                *(ctx->dict + i * 2) = 0;               /* byte */
            }

            *(ctx->dict + i * 2 + 1) = 0;           /* pointer */
        }

        ctx->numsymbols = 256;
        ctx->symbolwidth = 9;

        symbol = getsymbol(ctx);
        if (0 == ctx->nomoresymbols) {
            ctx->lastsymbol = symbol;
            tablelookup(symbol, symbol, ctx);
        }
    } /* (1 == ctx->overfill) */
    /* end of special cases */

    if (ctx->obuf_idx < ctx->obuf_len) {
        return obuf_pull(ctx);
    }
    
    if (1 == ctx->nomoresymbols) {
        return -1;
    }

    /* KEEP HONKING I'M RELOADING */
    symbol = getsymbol(ctx);

    if (0 == ctx->nomoresymbols) {
        tablelookup(symbol, ctx->lastsymbol, ctx);
        if (MAXSYMBOLS - 2 == ctx->numsymbols) { /* overflow! */
            ctx->overfill = 1;
        } else {
            *(ctx->dict + 2 * ctx->numsymbols) = ctx->firstbyte_lastsymbol;
            *(ctx->dict + 2 * ctx->numsymbols + 1) = (uint64_t) (ctx->dict + 2 * ctx->lastsymbol);
            ctx->numsymbols++;
            if (ctx->numsymbols + 1 == 1 << ctx->symbolwidth) {
                ctx->symbolwidth++;
            }
            ctx->lastsymbol = symbol;
        }
    } else {
        return -1;
    }


    if (ctx->obuf_idx < ctx->obuf_len) {
        return obuf_pull(ctx);
    } else {
        return -1;
    }

}



int main(int argc, char **argv)
{
    struct lzwctx * ctx;
    unsigned int foo;

    assert(2 == argc);

    ctx = initLZW(argv[1]);
    while ((foo = getclzw(ctx)) != -1) {
        putchar(foo);
    }

}

