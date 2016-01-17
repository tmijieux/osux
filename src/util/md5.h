#ifndef MD5_H
#define MD5_H

#include <stdio.h>
#include <stdlib.h>

unsigned char *osux_md5_hash_file(FILE *f);
unsigned char *osux_md5_hash_buf(size_t size, const unsigned char *buf);
void osux_md5_print(FILE *outifle, unsigned char *md5);
char *osux_md5_string(unsigned char *md5);

#endif //MD5_H
