
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include <zlib.h>


/* Use 32k to match gzip. */
#define BUFFER_SIZE (32 * 1024)

int main(int argc, char **argv) {
  assert(argc == 2);
  FILE *fp = fopen(argv[1], "r");
  assert(fp);

  z_stream stream;
  stream.zalloc = NULL;
  stream.zfree = NULL;
  stream.opaque = NULL;

  stream.next_in = NULL;
  stream.avail_in = 0;

  /* 15 is the window size.  Add 32 to support both zlib and gzip formats. */
  int err = inflateInit2(&stream, 15 + 32);
  assert(err == Z_OK);

  struct timespec start_time;
  int rc = clock_gettime(CLOCK_MONOTONIC, &start_time);
  assert(rc == 0);

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
      /* Discard the output. */
    } while (stream.avail_in != 0);
  } while (err != Z_STREAM_END);

  struct timespec end_time;
  rc = clock_gettime(CLOCK_MONOTONIC, &end_time);
  assert(rc == 0);
  double taken =
      (end_time.tv_sec - start_time.tv_sec) +
      (end_time.tv_nsec - start_time.tv_nsec) * 1e-9;

  err = inflateEnd(&stream);
  assert(err == Z_OK);

  printf("took %.6f\n", taken);

  return 0;
}
