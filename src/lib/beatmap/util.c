#include <glib.h>
#include <string.h>

#include "osux/util.h"
#include "osux/md5.h"

#define DEFAULT_LINE_CAPACITY 512

// read a line from file ;
// returned buffer must be freed by the user
char *osux_getline(FILE *file)
{
    if (feof(file))
        return NULL;
    
    size_t line_cap = DEFAULT_LINE_CAPACITY;
    char *line = g_malloc(line_cap);
    unsigned line_pos = 0;
    
    memset(line, 0, line_cap);
    
    for (;;) {
        int c = fgetc(file);
        switch ( c ) {
        case '\n':
            line[line_pos] = '\0';
            return line;
        case '\r':
            c = fgetc(file);
            if (c != '\n')
                ungetc(c, file);
            line[line_pos] = 0;
            return line;
        case EOF:
            return line;
        default:
            HANDLE_ARRAY_SIZE(line, line_pos, line_cap);
            line[line_pos] = c;
            ++ line_pos;
            break;
        }
    }
}

char *bytearray2hexstr(uint8_t const *bytes, size_t size)
{
    unsigned i;
    char *str = g_malloc(2*size+1);
    
    for (i = 0; i < size; ++i) {
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
    
    hash = bytearray2hexstr(osux_md5_get_digest(&md5), osux_md5_digest_length(&md5));
    osux_md5_free(&md5);
    return hash;
}


int strsplit_size(char **split)
{
    if (split == NULL)
        return 0;
    int i = 0;
    while (split[i] != NULL)
        i++;
    return i;
}
