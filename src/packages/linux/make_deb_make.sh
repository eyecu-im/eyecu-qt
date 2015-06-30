#!/bin/sh
# make_deb.sh

./debian/checkver.sh
cp ./debian/build.make.sh ./debian/build.sh
dpkg-buildpackage -us -uc
rm ./debian/build.sh

echo "All Done"
exit 0
