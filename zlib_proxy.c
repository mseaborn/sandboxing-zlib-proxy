
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include "zlib.h"
#include "zlib_untrusted.h"


typeof(inflateInit_) wrap_inflateInit_;
typeof(inflateInit2_) wrap_inflateInit2_;
typeof(inflate) wrap_inflate;
typeof(inflateReset) wrap_inflateReset;
typeof(inflateEnd) wrap_inflateEnd;


/* Type for sandboxed pointers, which are 32-bit. */
typedef uint32_t sb_ptr_t;

extern char __sfi_data_segment[];
extern uint64_t __sfi_data_segment_size;
static const sb_ptr_t data_segment_dest = 0x10000;
static const sb_ptr_t scratch_area = 0x20000000;

uint64_t __sfi_memory_base __attribute__((visibility("hidden")));


/* This is the layout of z_stream as used by sandboxed code.
   Pointers are changed to sb_ptr_t.
   uLong (unsigned long) is changed to uint32_t. */
struct z_stream_sb {
    sb_ptr_t next_in;  /* next input byte */
    uInt     avail_in;  /* number of bytes available at next_in */
    uint32_t    total_in;  /* total nb of input bytes read so far */

    sb_ptr_t next_out; /* next output byte should be put there */
    uInt     avail_out; /* remaining free space at next_out */
    uint32_t    total_out; /* total nb of bytes output so far */

    sb_ptr_t msg;      /* last error message, NULL if no error */
    sb_ptr_t state; /* not visible by applications */

    sb_ptr_t zalloc;  /* used to allocate the internal state */
    sb_ptr_t zfree;   /* used to free the internal state */
    sb_ptr_t opaque;  /* private data object passed to zalloc and zfree */

    int     data_type;  /* best guess about the data type: binary or text */
    uint32_t   adler;      /* adler32 value of the uncompressed data */
    uint32_t   reserved;   /* reserved for future use */
};


static void copy_in(sb_ptr_t dest, void *src, size_t size) {
  memcpy((char *) __sfi_memory_base + dest, src, size);
}

static void copy_out(void *dest, sb_ptr_t src, size_t size) {
  memcpy(dest, (char *) __sfi_memory_base + src, size);
}

static void init_sandbox() {
  assert(__sfi_memory_base == 0);

  size_t sandbox_size = (size_t) 1 << 32;
  void *alloc = mmap(NULL, sandbox_size, PROT_READ | PROT_WRITE,
                     MAP_ANONYMOUS | MAP_PRIVATE | MAP_NORESERVE, -1, 0);
  assert(alloc != MAP_FAILED);
  __sfi_memory_base = (uintptr_t) alloc;

  copy_in(data_segment_dest, __sfi_data_segment, __sfi_data_segment_size);
}


int inflateInit_(z_stream *stream, const char *version, int stream_size) {
  fprintf(stderr, "inflateInit_()\n");

  fprintf(stderr, "not implemented\n");
  abort();
}

int inflateInit2_(z_stream *stream, int window_bits,
                  const char *version, int stream_size) {
  fprintf(stderr, "inflateInit2_()\n");

  init_sandbox();

  z_stream *state = z_inflate_create();
  stream->state = (void *) state;
  return z_inflate_init2(state, window_bits);
}

int inflate(z_stream *stream, int flush) {
  fprintf(stderr, "inflate()\n");

  struct z_stream_sb *state =
      (void *) (__sfi_memory_base + (uintptr_t) stream->state);

  uint8_t *next_in = stream->next_in;
  size_t avail_in = stream->avail_in;
  uint8_t *next_out = stream->next_out;
  size_t avail_out = stream->avail_out;

  sb_ptr_t scratch = scratch_area;

  sb_ptr_t input_buf = scratch;
  scratch += avail_in;
  copy_in(input_buf, next_in, avail_in);
  state->next_in = input_buf;
  state->avail_in = avail_in;

  sb_ptr_t output_buf = scratch;
  state->next_out = output_buf;
  state->avail_out = avail_out;

  int result = wrap_inflate((void *) stream->state, flush);

  size_t bytes_read = state->next_in - input_buf;
  assert(bytes_read <= avail_in);
  assert(bytes_read == avail_in - state->avail_in);
  stream->next_in = next_in + bytes_read;
  stream->avail_in = avail_in - bytes_read;

  size_t bytes_written = state->next_out - output_buf;
  assert(bytes_written <= avail_out);
  assert(bytes_written == avail_out - state->avail_out);
  copy_out(next_out, output_buf, bytes_written);
  stream->next_out = next_out + bytes_written;
  stream->avail_out = avail_out - bytes_written;

  /* TODO: Add test for the following. */
  assert(!state->msg);
  stream->total_in = state->total_in;
  stream->total_out = state->total_out;
  stream->data_type = state->data_type;
  stream->adler = state->adler;

  return result;
}

int inflateReset(z_stream *stream) {
  fprintf(stderr, "inflateReset()\n");

  z_stream *state = (void *) stream->state;
  return wrap_inflateReset(state);
}

int inflateEnd(z_stream *stream) {
  fprintf(stderr, "inflateEnd()\n");

  z_stream *state = (void *) stream->state;
  return wrap_inflateEnd(state);
}
