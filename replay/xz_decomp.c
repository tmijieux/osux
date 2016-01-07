/*
 * xz_decomp.c
 * A simple example of pipe-only xz decompressor implementation.
 * version: 2010-07-12 - by Daniel Mealha Cabrita
 * Not copyrighted -- provided to the public domain.
 *
 * Compiling:
 * Link with liblzma. GCC example:
 * $ gcc -llzma xz_pipe_decomp.c -o xz_pipe_decomp
 *
 * Usage example:
 * $ cat some_file.xz | ./xz_pipe_decomp > some_file
 */

#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <lzma.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "xz_decomp.h"


/* read/write buffer sizes */
#define IN_BUF_MAX	512
#define OUT_BUF_MAX	512

/* error codes */
#define RET_OK			0
#define RET_ERROR_INIT		1
#define RET_ERROR_INPUT		2
#define RET_ERROR_OUTPUT	3
#define RET_ERROR_DECOMPRESSION	4

/* note: in_file and out_file must be open already */
static int lzma_legacy_decompress(FILE *in_file, FILE *out_file)
{
    lzma_stream strm = LZMA_STREAM_INIT; /* alloc and init lzma_stream struct */
    const uint64_t memory_limit = UINT64_MAX; /* no memory limit */
    uint8_t in_buf [IN_BUF_MAX];
    uint8_t out_buf [OUT_BUF_MAX];
    size_t in_len;	/* length of useful data in in_buf */
    size_t out_len;	/* length of useful data in out_buf */
    bool in_finished = false;
    bool out_finished = false;
    lzma_action action;
    lzma_ret ret_xz;
    int ret;

    ret = RET_OK;

    /* initialize lzma decoder */
    ret_xz = lzma_alone_decoder(&strm, memory_limit);
    if (ret_xz != LZMA_OK) {
	fprintf (stderr, "lzma_stream_decoder error: %d\n", (int) ret_xz);
	return RET_ERROR_INIT;
    }

    while ((! in_finished) && (! out_finished)) {
	/* read incoming data */
	in_len = fread (in_buf, 1, IN_BUF_MAX, in_file);

	if (feof (in_file)) {
	    in_finished = true;
	}
	if (ferror (in_file)) {
	    in_finished = true;
	    ret = RET_ERROR_INPUT;
	}
	strm.next_in = in_buf;
	strm.avail_in = in_len;

	/* if no more data from in_buf, flushes the
	   internal xz buffers and closes the decompressed data
	   with LZMA_FINISH */
	action = in_finished ? LZMA_FINISH : LZMA_RUN;

	/* loop until there's no pending decompressed output */
	do {
	    /* out_buf is clean at this point */
	    strm.next_out = out_buf;
	    strm.avail_out = OUT_BUF_MAX;

	    /* decompress data */
	    ret_xz = lzma_code (&strm, action);

	    if ((ret_xz != LZMA_OK) && (ret_xz != LZMA_STREAM_END)) {
		fprintf (stderr, "lzma_code error: %d\n", (int) ret_xz);
		out_finished = true;
		ret = RET_ERROR_DECOMPRESSION;
	    } else {
		/* write decompressed data */
		out_len = OUT_BUF_MAX - strm.avail_out;
		fwrite (out_buf, 1, out_len, out_file);
		if (ferror (out_file)) {
		    out_finished = true;
		    ret = RET_ERROR_OUTPUT;
		}
	    }
	} while (strm.avail_out == 0);
    }

    lzma_end (&strm);
    return ret;
}

void lzma_decompress(FILE *f, uint8_t **buf)
{
    struct stat st;
    FILE *out =  tmpfile();
    
    lzma_legacy_decompress(f, out);
    fflush(out);
    fstat(fileno(out), &st);
    *buf = malloc(st.st_size+1);
    rewind(out);
    fread(*buf, 1, st.st_size, out);
    (*buf)[st.st_size] = 0;
    fclose(out);
}
