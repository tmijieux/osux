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
#include <gio/gio.h>

#include "osux/read.h"
#include "osux/xz_decomp.h"

/* read/write buffer sizes */
#define IN_BUF_MAX	512
#define OUT_BUF_MAX	512

/* error codes */
#define RET_OK			0
#define RET_ERROR_INIT		1
#define RET_ERROR_INPUT		2
#define RET_ERROR_OUTPUT	3
#define RET_ERROR_DECOMPRESSION	4


static void lzma_error(lzma_ret error_code)
{
    switch (error_code) {
    case LZMA_OK:
    case LZMA_STREAM_END:
        break;
    case LZMA_DATA_ERROR:
    case LZMA_BUF_ERROR:
        fprintf(stderr, "error: replay data may be corrupted\n");
        break;
    default:
        fprintf(stderr,
                "error: an unexpected error has occured during replay"
                " data decompression:\nlzma code error %d\n\n", error_code);
                break;
    }

}

/* note: in_file and out_file must be open already */
static int lzma_legacy_decompress(GInputStream *in_file, GOutputStream *out_file)
{
    lzma_stream strm = LZMA_STREAM_INIT; /* alloc and init lzma_stream struct */
    const uint64_t memory_limit = UINT64_MAX; /* no memory limit */
    uint8_t in_buf [IN_BUF_MAX];
    uint8_t out_buf [OUT_BUF_MAX];
    ssize_t in_len;	/* length of useful data in in_buf */
    ssize_t out_len;	/* length of useful data in out_buf */
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

    while ((!in_finished) && (!out_finished)) {
	/* read incoming data */
	in_len = g_input_stream_read(in_file, in_buf, IN_BUF_MAX, NULL, NULL);

	if (in_len == 0)
	    in_finished = true; //end of file

	if (in_len < 0) {
	    in_finished = true;
	    ret = RET_ERROR_INPUT;
            in_len = 0;
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
	    ret_xz = lzma_code(&strm, action);

	    if ((ret_xz != LZMA_OK) && (ret_xz != LZMA_STREAM_END)) {
                lzma_error(ret_xz);
		out_finished = true;
		ret = RET_ERROR_DECOMPRESSION;
	    } else {
		/* write decompressed data */
		out_len = OUT_BUF_MAX - strm.avail_out;
                ssize_t err;
		err = g_output_stream_write(out_file, out_buf, out_len, NULL, NULL);
		if (err < 0) {
		    out_finished = true;
		    ret = RET_ERROR_OUTPUT;
		}
	    }
	} while (strm.avail_out == 0);
    }

    lzma_end (&strm);
    return ret;
}

void lzma_decompress(uint8_t *in_buf, size_t in_size,
                     uint8_t **out_buf, size_t *out_len)
{
    GInputStream *is = g_memory_input_stream_new_from_data(
        in_buf, in_size, NULL);
    GOutputStream *os = g_memory_output_stream_new(NULL, 0, g_realloc, NULL);
    if (lzma_legacy_decompress(is, os) != 0) {
        *out_len = 0;
        *out_buf = NULL;
    } else {
        *out_buf = g_memory_output_stream_get_data((GMemoryOutputStream*) os);
        *out_len = g_memory_output_stream_get_size((GMemoryOutputStream*) os);
    }

    g_object_unref(is);
    g_object_unref(os);
}
