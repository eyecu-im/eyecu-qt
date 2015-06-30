#!/bin/sh
# build.sh

PREFIX="$(pwd)/debian/tmp/usr"
mkdir -p debian/tmpbuild
cd debian/tmpbuild
export FFMPEGDIR="/usr"
export HOME="/home/yagiza"
qmake "INSTALL_PREFIX=${PREFIX}" ../../../../../eyecu.pro -r
make
make install

exit 0
