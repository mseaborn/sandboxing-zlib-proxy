
#include <stdlib.h>
#include <string.h>

#include "zlib.h"
#include "zlib_untrusted.h"


static uintptr_t next_alloc = 0x10000000;

void *malloc(size_t size) {
  uintptr_t addr = next_alloc;
  next_alloc += size;
  int align_mask = 7;
  next_alloc = (next_alloc + align_mask) & ~align_mask;
  return (void *) addr;
}

void free(void *block) {
}


z_stream *z_inflate_create(void) {
  z_stream *stream = malloc(sizeof(z_stream));
  memset(stream, 0, sizeof(*stream));
  return stream;
}

int z_inflate_init(z_stream *stream) {
  return inflateInit(stream);
}

int z_inflate_init2(z_stream *stream, int window_bits) {
  return inflateInit2(stream, window_bits);
}
