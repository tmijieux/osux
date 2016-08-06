#ifndef XZ_DECOMP_H
#define XZ_DECOMP_H

#include <stdio.h>
#include <stdint.h>

void lzma_decompress(uint8_t *in_buf, size_t in_isze,
                     uint8_t **out_buf, size_t *out_len);

#endif //XZ_DECOMP_H
