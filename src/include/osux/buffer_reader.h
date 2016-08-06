#ifndef OSUX_BUFFER_READER_H
#define OSUX_BUFFER_READER_H

#include <stdlib.h>
#include <stdint.h>

typedef struct osux_buffer_reader_ {
    void *data;
    size_t size;
    uint64_t r; // read pointer
} osux_buffer_reader;

int osux_buffer_reader_init(osux_buffer_reader *br, void *data, size_t size);

ssize_t obr_read(osux_buffer_reader *br, void *data, size_t size);
int obr_read_uleb128(osux_buffer_reader *br, uint64_t *value);
int obr_read_string(osux_buffer_reader *br, char **value);
int obr_read_lzma(osux_buffer_reader *br, char **value, size_t size);

int osux_buffer_reader_free(osux_buffer_reader *br);

#endif // OSUX_BUFFER_READER_H
