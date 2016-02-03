#ifndef MD5_H
#define MD5_H

#include <stdio.h>
#include <stdlib.h>
#include <openssl/md5.h>

int osux_md5_hash_file(FILE *f, unsigned char *md5_hash);
int osux_md5_hash_buf(size_t size,
                      const unsigned char *buf, unsigned char *md5_hash);
void osux_md5_print(FILE *outifle, unsigned char *md5);
char *osux_md5_string(unsigned char *md5, char *buf);

#endif //MD5_H
