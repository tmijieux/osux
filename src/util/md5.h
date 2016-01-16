#ifndef MD5_H
#define MD5_H

#include <stdio.h>
#include <stdlib.h>

unsigned char *osux_md5_hash_file(FILE *f);
unsigned char *osux_md5_hash_buf(size_t size, const unsigned char *buf);

#endif //MD5_H
