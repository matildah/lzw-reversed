/* decompresses the LZW compression used by this awful software.

   copyright (c) 2014 Matilda Helou <heinousbutch@gmail.com>. 
   licensed under the terms of the ISC license 
   */

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

struct lzwctx {
    /* buffers */
    uint8_t *dict;
    /* dict is arranged as such:
       [uint32_t byte] [uint32_t pointer] */

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

    lzw = malloc(sizeof(struct lzwctx));
    assert(NULL != lzw);

    lzw->dict = malloc(32768); /* yes, i am writing this on a 64 bit machine. #yolo */
    assert(NULL != lzw->dict);

    lzw->obuf = malloc(8192);
    assert(NULL != lzw->obuf);

    lzw->numsymbols = 256;
    lzw->symbolwidth = 9;

    lzw->overfill = 0;
    lzw->firstrun = 1;

    lzw->accumulator = 0;
    lzw->bits_in_accumulator = 0;
    
    return lzw;
}

int main()
{
    uint8_t *buf_32, *buf_16;

    uint8_t *buf_32768_ptr = malloc(32768);
    uint8_t *v4 = malloc(16384);

    buf_32 = buf_32768_ptr;
    buf_16 = v4;
    int v2 = 0;
    do
    {
        *(uint32_t *)(v4 + 4 * v2) = buf_32768_ptr;
        if ( 256 > v2 )
            *(uint16_t *)buf_32768_ptr = v2;
        *(uint32_t *)(*(uint32_t *)(v4 + 4 * v2++) + 4) = 0;
        buf_32768_ptr = (char *)buf_32768_ptr + 8;
    }
    while ( 4096 > v2 );
}

