#include <glib.h>

#include "osux/error.h"
#include "osux/buffer_reader.h"
#include "osux/xz_decomp.h"

ssize_t obr_read(osux_buffer_reader *br, void *data, size_t size)
{
    if (br->r + size > br->size)
        return -OSUX_ERR_BUFFER_READER_RANGE;
    memcpy(data, br->data+br->r, size);
    br->r += size;
    return size;
}

int obr_read_uleb128(osux_buffer_reader *br, uint64_t *value)
{
    *value = 0;
    unsigned shift = 0;
    uint8_t p;
    do {
        CHECK_ERROR(obr_read(br, &p, sizeof p), );
        *value += (uint64_t) (p & 0x7f) << shift;
        shift += 7;
    } while (p >= 0x80);
    return 0;
}

int obr_read_string(osux_buffer_reader *br, char **p_str)
{
    int err;
    if (p_str == NULL)
        return -OSUX_ERR_INVAL;

    uint8_t head;
    CHECK_ERROR(obr_read(br, &head, sizeof head), );
    if (head == 0x0B) {
        uint64_t uleb_size;
        CHECK_ERROR(obr_read_uleb128(br, &uleb_size), );
        *p_str = g_malloc(uleb_size+1);
        CHECK_ERROR(obr_read(br, *p_str, uleb_size), );
        (*p_str)[uleb_size] = 0;
    }
    return 0;
}

int obr_read_lzma(osux_buffer_reader *br, char **value, size_t size)
{
    size_t out_len = 0;
    lzma_decompress(br->data+br->r, size, (uint8_t**) value, &out_len);
    br->r += out_len;
    return 0;
}

int osux_buffer_reader_init(osux_buffer_reader *br, void *data, size_t size)
{
    memset(br, 0, sizeof*br);
    br->data = data;
    br->size = size;
    br->r = 0;
    return 0;
}

int osux_buffer_reader_free(osux_buffer_reader *br)
{
    (void) br;
}
