
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <zlib.h>


/* Use 32k to match gzip. */
#define BUFFER_SIZE (32 * 1024)

int main(int argc, char **argv) {
  FILE *fp = stdin;
  z_stream stream;
  int err;

  if (argc >= 2) {
    fp = fopen(argv[1], "r");
    assert(fp);
  }

  stream.zalloc = NULL;
  stream.zfree = NULL;
  stream.opaque = NULL;

  stream.next_in = NULL;
  stream.avail_in = 0;

  /* 15 is the window size.  Add 32 to support both zlib and gzip formats. */
  err = inflateInit2(&stream, 15 + 32);
  assert(err == Z_OK);

  do {
    uint8_t input_buf[BUFFER_SIZE];
    int got = fread(input_buf, 1, sizeof(input_buf), fp);
    assert(got != 0);
    assert(!ferror(fp));
    stream.next_in = input_buf;
    stream.avail_in = got;

    do {
      uint8_t out_buf[BUFFER_SIZE];
      stream.next_out = out_buf;
      stream.avail_out = sizeof(out_buf);
      err = inflate(&stream, Z_NO_FLUSH);
      assert(err != Z_STREAM_ERROR);
      int size = stream.next_out - out_buf;
      fwrite(out_buf, 1, size, stdout);
    } while (stream.avail_in != 0);
  } while (err != Z_STREAM_END);

  err = inflateEnd(&stream);
  assert(err == Z_OK);

  return 0;
}
