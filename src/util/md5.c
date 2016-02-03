#include <stdio.h>
#include <stdlib.h>
#include <openssl/md5.h>
#include <assert.h>
#include "util/error.h"

#ifndef min
#define min(x, y)  ((x) < (y) ? (x) : (y))
#endif

int osux_md5_hash_file(FILE *f, unsigned char *md5_hash)
{
    if (f == NULL || md5_hash == NULL) {
        osux_error("invalid argument");
        return -1;
    }

    MD5_CTX mdContext;
    int bytes = 0;
    unsigned char data[1024];

    assert( NULL != f );
    rewind(f);
    MD5_Init(&mdContext);
    while ((bytes = fread(data, 1, 1024, f)) != 0)
        MD5_Update(&mdContext, data, bytes);
    MD5_Final(md5_hash, &mdContext);
    rewind(f);
    return 0;
}

int osux_md5_hash_buf(
    size_t size, const unsigned char *buf, unsigned char *md5_hash)
{
    if (buf == NULL || md5_hash == NULL) {
        osux_error("invalid argument");
        return -1;
    } else if (0 == size) {
        return 0;
    }
    
    MD5_CTX mdContext;
    int bytes = 0;
    const unsigned char *data = buf;
    
    MD5_Init(&mdContext);
    while ( data < buf+size ) {
        bytes = min(1024, (buf+size)-data);
        MD5_Update(&mdContext, data, bytes);
        data += bytes;
    }
    MD5_Final(md5_hash, &mdContext);
    return 0;
}

char *osux_md5_string(unsigned char *md5, char *buf)
{
    for(int i = 0; i < MD5_DIGEST_LENGTH; ++i)
        sprintf(&buf[i*2], "%02x", (unsigned int)md5[i]);
    buf[MD5_DIGEST_LENGTH * 2] = '\0';
    return buf;
}

void osux_md5_print(FILE *outfile, unsigned char *md5)
{
    char buf[MD5_DIGEST_LENGTH * 2 + 1];
    fprintf(outfile, "%s", osux_md5_string(md5, buf));
}
