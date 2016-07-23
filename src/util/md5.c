#include <stdio.h>
#include <stdlib.h>
#include <openssl/md5.h>
#include <assert.h>

#include "util/error.h"
#include "./md5.h"

#ifndef min
#define min(x, y)  ((x) < (y) ? (x) : (y))
#endif

int osux_md5_hash_file(FILE *f, char **md5_hash_str)
{
    if (f == NULL || md5_hash_str == NULL) {
        osux_error("invalid argument");
        return -1;
    }

    MD5_CTX mdContext;
    int bytes = 0;
    unsigned char data[1024];
    unsigned char hash[MD5_DIGEST_LENGTH];

    assert( NULL != f );
    rewind(f);
    MD5_Init(&mdContext);
    while ((bytes = fread(data, 1, 1024, f)) != 0)
        MD5_Update(&mdContext, data, bytes);
    MD5_Final(hash, &mdContext);
    
    *md5_hash_str = calloc(MD5_DIGEST_LENGTH*2+1, sizeof*md5_hash_str);
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i)
        sprintf(&(*md5_hash_str)[i*2], "%02x", (unsigned int)hash[i]);
    (*md5_hash_str)[MD5_DIGEST_LENGTH * 2] = '\0';
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
