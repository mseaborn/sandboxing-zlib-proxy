
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "zlib.h"


typeof(inflateInit_) wrap_inflateInit_;
typeof(inflateInit2_) wrap_inflateInit2_;
typeof(inflate) wrap_inflate;
typeof(inflateReset) wrap_inflateReset;
typeof(inflateEnd) wrap_inflateEnd;


int inflateInit_(z_stream *stream, const char *version, int stream_size) {
  fprintf(stderr, "inflateInit_()\n");
  memset(stream, 0, sizeof(*stream));
  return wrap_inflateInit_(stream, version, stream_size);
}

int inflateInit2_(z_stream *stream, int window_bits,
                  const char *version, int stream_size) {
  fprintf(stderr, "inflateInit2_()\n");
  return wrap_inflateInit2_(stream, window_bits, version, stream_size);
}

int inflate(z_stream *stream, int flush) {
  fprintf(stderr, "inflate()\n");
  uint8_t *next_out = stream->next_out;
  size_t avail_out = stream->avail_out;

  uint8_t *buf = malloc(avail_out);
  assert(buf);
  stream->next_out = buf;

  int result = wrap_inflate(stream, flush);
  size_t written = stream->next_out - buf;
  assert(written <= avail_out);
  assert(written == avail_out - stream->avail_out);
  memcpy(next_out, buf, written);
  free(buf);
  stream->next_out = next_out + written;
  stream->avail_out = avail_out - written;
  return result;
}

int inflateReset(z_stream *stream) {
  fprintf(stderr, "inflateReset()\n");
  return wrap_inflateReset(stream);
}

int inflateEnd(z_stream *stream) {
  fprintf(stderr, "inflateEnd()\n");
  return wrap_inflateEnd(stream);
}
