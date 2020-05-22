HEADERS += \
    random/jitterentropy-base-user.h \
    random/jitterentropy.h \
    cipher/asm-common-aarch64.h \
    cipher/asm-common-amd64.h \
    cipher/asm-poly1305-amd64.h \
    cipher/bithelp.h \
    cipher/bufhelp.h \
    cipher/camellia.h \
    cipher/cipher-internal.h \
    cipher/cipher-selftest.h \
    cipher/ecc-common.h \
    cipher/gost.h \
    cipher/hash-common.h \
    cipher/kdf-internal.h \
    cipher/keccak_permute_32.h \
    cipher/keccak_permute_64.h \
    cipher/mac-internal.h \
    cipher/poly1305-internal.h \
    cipher/pubkey-internal.h \
    cipher/rijndael-internal.h \
    cipher/rijndael-tables.h \
    cipher/sha1.h \
    compat/libcompat.h \
    mpi/ec-internal.h \
    mpi/generic/mpi-asm-defs.h \
    mpi/longlong.h \
    mpi/mpi-inline.h \
    mpi/mpi-internal.h \
    random/rand-internal.h \
    random/random.h \
    src/cipher-proto.h \
    src/cipher.h \
    src/context.h \
    src/ec-context.h \
    src/g10lib.h \
    src/gcrypt-int.h \
    src/hmac256.h \
    src/hwf-common.h \
    src/mpi.h \
    src/secmem.h \
    src/stdmem.h \
    src/types.h \
    src/visibility.h \
    SMP/config.h \
    SMP/gcrypt.h \
    SMP/mpi/asm-syntax.h \
    SMP/mpi/mod-source-info.h \
    SMP/mpi/mpi-asm-defs.h \
    SMP/mpi/sysdep.h \
    SMP/version.h \
    SMP/compat.h

SOURCES += \
    random/jitterentropy-base.c \
    random/rndjent.c \
    cipher/blake2.c \
    cipher/cipher-eax.c \
    cipher/cipher-xts.c \
    cipher/crc-armv8-ce.c \
    cipher/rijndael-armv8-ce.c \
    cipher/sha1-intel-shaext.c \
    cipher/sha256-intel-shaext.c \
    cipher/sm3.c \
    cipher/arcfour.c \
    cipher/blowfish.c \
    cipher/camellia-glue.c \
    cipher/camellia.c \
    cipher/cast5.c \
    cipher/chacha20.c \
    cipher/cipher-aeswrap.c \
    cipher/cipher-cbc.c \
    cipher/cipher-ccm.c \
    cipher/cipher-cfb.c \
    cipher/cipher-cmac.c \
    cipher/cipher-ctr.c \
    cipher/cipher-gcm-intel-pclmul.c \
    cipher/cipher-gcm.c \
    cipher/cipher-ocb.c \
    cipher/cipher-ofb.c \
    cipher/cipher-poly1305.c \
    cipher/cipher-selftest.c \
    cipher/cipher.c \
    cipher/crc-intel-pclmul.c \
    cipher/crc.c \
    cipher/des.c \
    cipher/dsa-common.c \
    cipher/dsa.c \
    cipher/ecc-curves.c \
    cipher/ecc-ecdsa.c \
    cipher/ecc-eddsa.c \
    cipher/ecc-gost.c \
    cipher/ecc-misc.c \
    cipher/ecc.c \
    cipher/elgamal.c \
    cipher/gost28147.c \
    cipher/gostr3411-94.c \
    cipher/hash-common.c \
    cipher/hmac-tests.c \
    cipher/idea.c \
    cipher/kdf.c \
    cipher/keccak.c \
    cipher/mac-cmac.c \
    cipher/mac-gmac.c \
    cipher/mac-hmac.c \
    cipher/mac-poly1305.c \
    cipher/mac.c \
    cipher/md.c \
    cipher/md4.c \
    cipher/md5.c \
    cipher/poly1305.c \
    cipher/primegen.c \
    cipher/pubkey-util.c \
    cipher/pubkey.c \
    cipher/rfc2268.c \
    cipher/rijndael-aesni.c \
    cipher/rijndael-padlock.c \
    cipher/rijndael.c \
    cipher/rmd160.c \
    cipher/rsa-common.c \
    cipher/rsa.c \
    cipher/salsa20.c \
    cipher/scrypt.c \
    cipher/seed.c \
    cipher/serpent.c \
    cipher/sha1.c \
    cipher/sha256.c \
    cipher/sha512.c \
    cipher/stribog.c \
    cipher/tiger.c \
    cipher/twofish.c \
    cipher/whirlpool.c \
    compat/compat.c \
    mpi/ec-ed25519.c \
    mpi/ec.c \
    mpi/generic/mpih-add1.c \
    mpi/generic/mpih-lshift.c \
    mpi/generic/mpih-mul1.c \
    mpi/generic/mpih-mul2.c \
    mpi/generic/mpih-mul3.c \
    mpi/generic/mpih-rshift.c \
    mpi/generic/mpih-sub1.c \
    mpi/mpi-add.c \
    mpi/mpi-bit.c \
    mpi/mpi-cmp.c \
    mpi/mpi-div.c \
    mpi/mpi-gcd.c \
    mpi/mpi-inline.c \
    mpi/mpi-inv.c \
    mpi/mpi-mod.c \
    mpi/mpi-mpow.c \
    mpi/mpi-mul.c \
    mpi/mpi-pow.c \
    mpi/mpi-scan.c \
    mpi/mpicoder.c \
    mpi/mpih-div.c \
    mpi/mpih-mul.c \
    mpi/mpiutil.c \
    random/random-csprng.c \
    random/random-drbg.c \
    random/random-system.c \
    random/random.c \
    random/rndhw.c \
    src/context.c \
    src/fips.c \
    src/global.c \
    src/hmac256.c \
    src/hwfeatures.c \
    src/misc.c \
    src/missing-string.c \
    src/secmem.c \
    src/sexp.c \
    src/stdmem.c \
    src/visibility.c

win32 {
    HEADERS += \

    SOURCES += \
        cipher/rijndael-ssse3-amd64.c \
        random/rndw32.c 
}

linux {
    SOURCES += \
        random/rndlinux.c
}

unix {
    SOURCES += \
        random/rndunix.c \
        random/random-fips.c
}

# equals(QT_ARCH, "i386") | equals(QT_ARCH, "x86_64") {
equals(QMAKE_HOST.arch, "i386") | equals(QMAKE_HOST.arch, x86) {
    SOURCES += src/hwf-x86.c
}
