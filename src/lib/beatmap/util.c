#include <glib.h>
#include <string.h>

#include "osux/util.h"
#include "osux/error.h"
#include "osux/md5.h"

#define DEFAULT_LINE_CAPACITY 512

int osux_getline(GIOChannel *chan, char **return_value)
{
    int err = 0;
    char *str = NULL;
    size_t length, end;
    GError *error = NULL;
    *return_value = NULL;
    GIOStatus status = g_io_channel_read_line(chan, &str, &length, &end, &error);
    if (status == G_IO_STATUS_EOF)
        return 1;
    else if (status != G_IO_STATUS_NORMAL) {
        printf ("getline: %s\n", error?error->message:"no error");
        g_free(str);
        g_clear_error(&error);
        return -OSUX_ERR_FILE_BAD_ENCODING;
    }
    str[end] = 0;
    g_strchomp(str);
    *return_value = str;
    return err;
}

#define SET_ENCODING(chan, error, status, encoding)                     \
    do {                                                                \
        status = g_io_channel_set_encoding(chan, encoding, &error);     \
        if (status == G_IO_STATUS_ERROR) {                              \
            fprintf(stderr, "set encoding error (%s): %s\n",            \
                    encoding?encoding:"BINARY", error->message);        \
            g_error_free(error);                                        \
            return NULL;                                                \
        }                                                               \
    } while (0)

#define SEEK_POS(chan, error, status, pos)                              \
    do {                                                                \
        status = g_io_channel_seek_position(chan, pos, G_SEEK_SET, &error); \
        if (status == G_IO_STATUS_ERROR) {                              \
            fprintf(stderr, "seek error: %s\n", error->message);        \
            g_clear_error(&error);                                      \
            return NULL;                                                \
        }                                                               \
    } while (0)

#define SET_ENCODING_FROM_BOM(bom, data, enc, chan)             \
    do {                                                        \
        GIOStatus status;                                       \
        GError *error = NULL;                                   \
        if (!memcmp((bom), data, sizeof((bom)))) {              \
            SEEK_POS(chan, error, status, sizeof(bom));         \
            SET_ENCODING(chan, error, status, enc);             \
            /*printf("%s BOM detected\n", enc);*/               \
            return chan;                                        \
        }                                                       \
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
    SET_ENCODING(chan, error, status, NULL);

    char data[4]; gsize length; // BOMs are up to 4 bytes long
    status = g_io_channel_read_chars(chan, data, 4, &length, &error);
    if (status == G_IO_STATUS_ERROR) {
        fprintf(stderr, "%s: %s\n", file_path, error->message);
        g_error_free(error);
        return NULL;
    }
    // these macros return on successful encoding match
    SET_ENCODING_FROM_BOM(utf32be, data, "UTF-32BE", chan); // order is important
    SET_ENCODING_FROM_BOM(utf32le, data, "UTF-32LE", chan);
    SET_ENCODING_FROM_BOM(utf16le, data, "UTF-16LE", chan);
    SET_ENCODING_FROM_BOM(utf16be, data, "UTF-16BE", chan);
    SET_ENCODING_FROM_BOM(utf8, data, "UTF-8", chan);

    //set file to utf8 if no bom was found
    SEEK_POS(chan, error, status, 0);
    SET_ENCODING(chan, error, status, "UTF-8");

    char *str = NULL;
    status = g_io_channel_read_line(chan, &str, NULL, NULL, NULL);
    if (status == G_IO_STATUS_ERROR) {
        SEEK_POS(chan, error, status, 0);
        SET_ENCODING(chan, error, status, "ISO-8859-1");
    }
    g_free(str);
    SEEK_POS(chan, error, status, 0);
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
