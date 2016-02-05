#ifndef MD5_H
#define MD5_H

#include <stdio.h>
#include <stdlib.h>
#include <openssl/md5.h>

int osux_md5_hash_file(FILE *f, char **md5_hash_str);
int osux_md5_hash_buf(
    size_t size, const unsigned char *buf, unsigned char *md5_hash);

#endif //MD5_H
