#ifndef XZ_DECOMP_H
#define XZ_DECOMP_H

#include <stdio.h>
#include <stdint.h>

void lzma_decompress(FILE *f, uint8_t **buf);

#endif //XZ_DECOMP_H
