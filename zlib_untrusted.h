#ifndef ZLIB_UNTRUSTED_H_
#define ZLIB_UNTRUSTED_H_

z_stream *z_inflate_create(void);
int z_inflate_init(z_stream *stream);
int z_inflate_init2(z_stream *stream, int window_bits);

#endif
