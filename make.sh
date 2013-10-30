#!/bin/bash

set -eux

mkdir -p out

zlib=zlib_subset
tc_bin=~/devel/nacl-git3/native_client/toolchain/pnacl_linux_x86/host_x86_64/bin
cflags="-Wall -Werror -g -O2"

files="inflate inftrees inffast adler32 crc32 zutil"
for file in $files; do
  gcc $cflags -c -fPIC $zlib/$file.c -o out/$file.o
  pnacl-clang $cflags -c -fPIC $zlib/$file.c -o out/$file.p.o
done
ld -r $(for f in $files; do echo out/$f.o; done) -o out/zlib_nosandbox.o

pnacl-clang $cflags -c sfi_stack_ptr.c -o out/sfi_stack_ptr.p.o
pnacl-clang $cflags -c zlib_untrusted.c -o out/zlib_untrusted.p.o -I$zlib

pnacl-ld -r $(for f in $files; do echo out/$f.p.o; done) \
    out/sfi_stack_ptr.p.o \
    out/zlib_untrusted.p.o \
    -o out/zlib.p.o
pnacl-opt -pnacl-abi-simplify-postopt \
    -expand-allocas -allocate-data-segment -sandbox-memory-accesses \
    out/zlib.p.o -o out/zlib.p.o
$tc_bin/llc -mtriple=x86_64-linux-gnu -relocation-model=pic -filetype=obj -O2 \
    out/zlib.p.o -o out/zlib.o


# Rename symbols so that we can wrap the library.
args=""
for sym in inflateInit_ inflateInit2_ inflate inflateReset inflateEnd; do
  args="$args -G wrap_$sym --redefine-sym $sym=wrap_$sym"
done
objcopy $args -G z_inflate_create -G z_inflate_init -G z_inflate_init2 \
    -G __sfi_data_segment -G __sfi_data_segment_size \
    out/zlib.o out/zlib_hidden.o

gcc $cflags -shared -fPIC -Wl,-z,defs zlib_proxy.c out/zlib_hidden.o \
    -o out/zlib_proxy.so


# Build non-sandboxing version.
objcopy $args out/zlib_nosandbox.o out/zlib_hidden_nosandbox.o
gcc $cflags -shared -fPIC -Wl,-z,defs \
    zlib_proxy_nosandbox.c out/zlib_hidden_nosandbox.o \
    -o out/zlib_proxy_nosandbox.so


# Test decompression.
gcc $cflags decompress.c -lz -o out/decompress
gcc $cflags decompress_timer.c -lz -lrt -o out/decompress_timer

function decompress_sandbox {
  LD_PRELOAD=out/zlib_proxy.so out/decompress
}
function decompress_nosandbox {
  LD_PRELOAD=out/zlib_proxy_nosandbox.so out/decompress
}

testfile=/usr/share/common-licenses/GPL-3
hash=$(cat $testfile | sha1sum -)
gzip -c $testfile > out/input.gz

function test_decompress {
  "$@" < out/input.gz > out/result
  hash2=$(cat out/result | sha1sum -)
  if [ "$hash" != "$hash2" ]; then
    echo mismatch: $hash $hash2
    exit 1
  fi
}
test_decompress decompress_nosandbox
test_decompress decompress_sandbox
echo PASS
