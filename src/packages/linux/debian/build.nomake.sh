#!/bin/sh
# build.sh

echo "pwd=$(pwd)"

mkdir -p debian/tmp/usr/bin
cp /usr/local/bin/eyecu debian/tmp/usr/bin/eyecu
mkdir debian/tmp/usr/include
cp -r /usr/local/include/eyecu debian/tmp/usr/include
mkdir debian/tmp/usr/lib
cp -r /usr/local/lib/eyecu debian/tmp/usr/lib
cp /usr/local/lib/libeyecuutils.so* debian/tmp/usr/lib
mkdir -p debian/tmp/usr/share/doc
cp -r /usr/local/share/doc/eyecu debian/tmp/usr/share/doc
mkdir debian/tmp/usr/share/applications
cp /usr/local/share/applications/eyecu.desktop debian/tmp/usr/share/applications
mkdir debian/tmp/usr/share/pixmaps
cp /usr/local/share/pixmaps/eyecu.svg debian/tmp/usr/share/pixmaps
cp -r /usr/local/share/eyecu debian/tmp/usr/share

exit 0
