
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


struct state {
  z_stream stream;
};


static struct state *stream_init(z_stream *stream) {
  struct state *state = malloc(sizeof(struct state));
  assert(state);
  memset(state, 0, sizeof(*state));
  stream->state = (void *) state;
  return state;
}

int inflateInit_(z_stream *stream, const char *version, int stream_size) {
  fprintf(stderr, "inflateInit_()\n");

  struct state *state = stream_init(stream);
  return wrap_inflateInit_(&state->stream, version, stream_size);
}

int inflateInit2_(z_stream *stream, int window_bits,
                  const char *version, int stream_size) {
  fprintf(stderr, "inflateInit2_()\n");

  struct state *state = stream_init(stream);
  return wrap_inflateInit2_(&state->stream, window_bits, version, stream_size);
}

int inflate(z_stream *stream, int flush) {
  fprintf(stderr, "inflate()\n");

  struct state *state = (void *) stream->state;

  uint8_t *next_in = stream->next_in;
  size_t avail_in = stream->avail_in;
  uint8_t *next_out = stream->next_out;
  size_t avail_out = stream->avail_out;

  uint8_t *input_buf = malloc(avail_in);
  assert(input_buf);
  memcpy(input_buf, next_in, avail_in);
  state->stream.next_in = input_buf;
  state->stream.avail_in = avail_in;

  uint8_t *output_buf = malloc(avail_out);
  assert(output_buf);
  state->stream.next_out = output_buf;
  state->stream.avail_out = avail_out;

  int result = wrap_inflate(&state->stream, flush);

  size_t bytes_read = state->stream.next_in - input_buf;
  assert(bytes_read <= avail_in);
  assert(bytes_read == avail_in - state->stream.avail_in);
  free(input_buf);
  stream->next_in = next_in + bytes_read;
  stream->avail_in = avail_in - bytes_read;

  size_t bytes_written = state->stream.next_out - output_buf;
  assert(bytes_written <= avail_out);
  assert(bytes_written == avail_out - state->stream.avail_out);
  memcpy(next_out, output_buf, bytes_written);
  free(output_buf);
  stream->next_out = next_out + bytes_written;
  stream->avail_out = avail_out - bytes_written;

  /* TODO: Add test for the following. */
  stream->msg = state->stream.msg;
  stream->total_in = state->stream.total_in;
  stream->total_out = state->stream.total_out;
  stream->data_type = state->stream.data_type;
  stream->adler = state->stream.adler;

  return result;
}

int inflateReset(z_stream *stream) {
  fprintf(stderr, "inflateReset()\n");

  struct state *state = (void *) stream->state;
  return wrap_inflateReset(&state->stream);
}

int inflateEnd(z_stream *stream) {
  fprintf(stderr, "inflateEnd()\n");

  struct state *state = (void *) stream->state;
  return wrap_inflateEnd(&state->stream);
}
