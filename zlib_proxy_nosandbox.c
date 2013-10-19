
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

  uint8_t *input_buf;  /* Buffer allocated by proxy */
  uint8_t *expect_next_in;  /* Data provided by proxy's client */
  size_t expect_avail_in;
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

  if (state->input_buf != NULL) {
    assert(next_in == state->expect_next_in);
    assert(avail_in == state->expect_avail_in);
  } else {
    state->input_buf = malloc(avail_in);
    assert(state->input_buf);
    memcpy(state->input_buf, next_in, avail_in);
    state->stream.next_in = state->input_buf;
    state->stream.avail_in = avail_in;
    state->expect_next_in = next_in;
    state->expect_avail_in = avail_in;
  }
  uint8_t *next_in_before = state->stream.next_in;

  uint8_t *output_buf = malloc(avail_out);
  assert(output_buf);
  state->stream.next_out = output_buf;
  state->stream.avail_out = avail_out;

  int result = wrap_inflate(&state->stream, flush);

  size_t bytes_read = state->stream.next_in - next_in_before;
  assert(bytes_read <= avail_in);
  assert(bytes_read == avail_in - state->stream.avail_in);
  stream->next_in = next_in + bytes_read;
  stream->avail_in = avail_in - bytes_read;
  state->expect_next_in += bytes_read;
  state->expect_avail_in -= bytes_read;
  if (state->expect_avail_in == 0) {
    free(state->input_buf);
    state->input_buf = NULL;
  }

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
  free(state->input_buf);
  return wrap_inflateEnd(&state->stream);
}
