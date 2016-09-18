#ifndef OSUX_XZ_DECOMP_H
#define OSUX_XZ_DECOMP_H

#include <stdio.h>
#include <stdint.h>
#include <glib.h>

G_BEGIN_DECLS

void lzma_decompress(uint8_t *in_buf, size_t in_isze,
                     uint8_t **out_buf, size_t *out_len);
G_END_DECLS

#endif //XZ_DECOMP_H
