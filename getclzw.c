#include <stdint.h>
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

