#!/bin/sh
# make_deb.sh

# Version
BASE=-71
NAME="eyecu"
[ -d .svn ] && svn_version="$(sed -n -e '/^dir$/{n;p;q;}' .svn/entries 2>/dev/null)"||svn_version=""
# LC_ALL=C svn info 2> /dev/null | grep Revision | cut -d' ' -f2
#VER="$(echo `grep CLIENT_VERSION ../../definitions/version.h|awk -F'"' '{print $2}'`|tr ' ' '.')"
VER="$(grep 'CLIENT_VERSION ' ../../definitions/version.h|awk -F'"' '{print $2}')"
#NAME="$(grep 'CLIENT_NAME ' ../../definitions/version.h|awk -F'"' '{print $2}')"

PATCH=$((${svn_version}-${BASE}))
VERSION="${VER}.${PATCH}"

echo NAME=${NAME}
echo VER=${VER}
echo VERSION=${VERSION}

echo "${NAME} (${VERSION}-1) unstable; urgency=low" > ./debian/changelog
echo "" >> ./debian/changelog
echo "  * Initial debianization" >> ./debian/changelog
echo "" >> ./debian/changelog
echo " -- Konstantin Kozlov <yagiza@rwsoftware.ru>  $(date -R)" >> ./debian/changelog

echo "All Done"
exit 0
