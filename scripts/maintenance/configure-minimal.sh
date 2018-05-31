#!/bin/sh
# Configure fim with minimal features allowing it to run in the framebuffer
./configure --disable-sdl --disable-fimrc --disable-history --disable-readline \
	--disable-xfig --disable-xcftopnm --disable-xfig --disable-convert \
       --disable-stdin-image-reading --disable-ps --disable-djvu \
       --disable-raw-bits-rendering "$@"   || exit
make || exit
make tests || exit
scripts/maintenance/live-test.sh
