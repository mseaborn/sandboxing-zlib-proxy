#!/bin/bash

set -eux

mkdir -p out

zlib=~/sw/zlib-1.2.3.3.dfsg
cflags="-Wall -Werror -g"

files="inflate inftrees inffast adler32 crc32 zutil"
for file in $files; do
  gcc $cflags -c -fPIC $zlib/$file.c -o out/$file.o
done
ld -r $(for f in $files; do echo out/$f.o; done) -o out/zlib.o

# Non-proxying subset of zlib that works as an LD_PRELOAD library.
gcc -shared out/zlib.o -o out/zlib_min.so


# Rename symbols so that we can wrap the library.
args=""
for sym in inflateInit_ inflateInit2_ inflate inflateReset inflateEnd; do
  args="$args -G wrap_$sym --redefine-sym $sym=wrap_$sym"
done
objcopy $args out/zlib.o out/zlib_hidden.o

gcc $cflags -shared -fPIC zlib_proxy.c out/zlib_hidden.o \
    -o out/zlib_proxy.so


# Test decompression.
testfile=/usr/share/common-licenses/GPL-3
hash=$(cat $testfile | sha1sum -)
gzip -c $testfile | LD_PRELOAD=out/zlib_proxy.so ../zlib/decompress > out/result
hash2=$(cat out/result | sha1sum -)
if [ "$hash" != "$hash2" ]; then
  echo mismatch: $hash $hash2
  exit 1
fi
echo PASS
