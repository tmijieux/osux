#include <glib.h>
#include <string.h>

#include "osux/util.h"
#include "osux/md5.h"

#define DEFAULT_LINE_CAPACITY 512

char *osux_getline(GIOChannel *chan)
{
    char *str = NULL;
    size_t length, end;
    GIOStatus status = g_io_channel_read_line(chan, &str, &length, &end, NULL);
    if (status != G_IO_STATUS_NORMAL)
        return NULL;
    str[end] = 0;
    return str;
}

#define CHECK_ENCODING(bom, data, enc, chan)                            \
    do {                                                                \
        GIOStatus status;                                               \
        GError *error = NULL;                                           \
        if (!memcmp((bom), data, sizeof((bom)))) {                      \
            status = g_io_channel_seek_position(                        \
                chan, sizeof((bom)), G_SEEK_SET, NULL);                 \
            if (status == G_IO_STATUS_ERROR) {                          \
                fprintf(stderr, "seek error: %s\n", error->message);    \
                g_error_free(error);                                    \
                return NULL;                                            \
            }                                                           \
            status = g_io_channel_set_encoding(chan, enc, NULL);        \
            if (status == G_IO_STATUS_ERROR) {                          \
                fprintf(stderr, "set encoding error: %s\n", error->message); \
                g_error_free(error);                                    \
                return NULL;                                            \
            }                                                           \
            return chan;                                                \
        }                                                               \
    } while(0)

GIOChannel *osux_open_text_file_reading(char const *file_path)
{
    static char utf8[] = { 0xEF, 0xBB, 0xBF };
    static char utf16le[] = { 0xFF, 0xFE };
    static char utf16be[] = { 0xFE, 0xFF };
    static char utf32le[] = { 0xFF, 0xFE, 0x00, 0x00 };
    static char utf32be[] = { 0x00, 0x00, 0xFE, 0xFF };

    GIOStatus status;
    GError *error = NULL;
    GIOChannel *chan = g_io_channel_new_file(file_path, "r", &error);

    if (chan == NULL) {
        fprintf(stderr, "%s: %s\n", file_path, error->message);
        g_error_free(error);
        return NULL;
    }

    // set file to binary mode (no encoding):
    status = g_io_channel_set_encoding(chan, NULL, &error);
    if (status == G_IO_STATUS_ERROR) {
        fprintf(stderr, "set binary error: %s\n", error->message);
        g_error_free(error);
        return NULL;
    }

    char data[4]; gsize length;
    status = g_io_channel_read_chars(chan, data, 4, &length, &error);
    if (status == G_IO_STATUS_ERROR) {
        fprintf(stderr, "read bom error: %s\n", error->message);
        g_error_free(error);
        return NULL;
    }
    CHECK_ENCODING(utf32be, data, "UTF-32BE", chan); // order is important
    CHECK_ENCODING(utf32le, data, "UTF-32LE", chan);
    CHECK_ENCODING(utf16le, data, "UTF-16LE", chan);
    CHECK_ENCODING(utf16be, data, "UTF-16BE", chan);
    CHECK_ENCODING(utf8, data, "UTF-8", chan);

    //set file to utf8 if no bom was found
    status = g_io_channel_seek_position(chan, 0, G_SEEK_SET, NULL);
    if (status == G_IO_STATUS_ERROR) {
        fprintf(stderr, "seek error: %s\n", error->message);
        g_error_free(error);
        return NULL;
    }
    status = g_io_channel_set_encoding(chan, "UTF-8", NULL);
    if (status == G_IO_STATUS_ERROR) {
        fprintf(stderr, "set encoding error (UTF-8): %s\n", error->message);
        g_error_free(error);
        return NULL;
    }
    return chan;
}

char *bytearray2hexstr(uint8_t const *bytes, size_t size)
{
    char *str = g_malloc(2*size+1);

    for (unsigned i = 0; i < size; ++i) {
        char buffer[4];
        sprintf(buffer, "%02x", bytes[i]);
        memcpy(str+2*i, buffer, 2);
    }
    str[2*size] = '\0';
    return str;
}

char *osux_get_file_hashstr(char const *file_path)
{
    osux_md5 md5;
    char *hash;

    osux_md5_init(&md5);
    osux_md5_update_file(&md5, file_path);
    osux_md5_finalize(&md5);

    hash = bytearray2hexstr(osux_md5_get_digest(&md5),
                            osux_md5_digest_length(&md5));
    osux_md5_free(&md5);
    return hash;
}

unsigned strsplit_size(char **split)
{
    if (split == NULL)
        return 0;
    int i = 0;
    while (split[i] != NULL)
        i++;
    return i;
}
