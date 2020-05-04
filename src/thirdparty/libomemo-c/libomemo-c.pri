INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

SOURCES += \
    # protobuf_SRCS
	OMEMO.pb-c.c \
    LocalStorageProtocol.pb-c.c \
    WhisperTextProtocol.pb-c.c \
    FingerprintProtocol.pb-c.c \
    # signal_protocol_SRCS
    vpool.c \
    signal_protocol.c \
    curve.c \
    hkdf.c \
    ratchet.c \
    protocol.c \
    session_state.c \
    session_record.c \
    session_pre_key.c \
    session_builder.c \
    session_cipher.c \
    key_helper.c \
    sender_key.c \
    sender_key_state.c \
    sender_key_record.c \
    group_session_builder.c \
    group_cipher.c \
    fingerprint.c \
    device_consistency.c

HEADERS += \
	FingerprintProtocol.pb-c.h \
	LocalStorageProtocol.pb-c.h \
	OMEMO.pb-c.h \
	WhisperTextProtocol.pb-c.h \
	signal_utarray.h \
	utarray.h \
	uthash.h \
	utlist.h \
    vpool.h \
    signal_protocol.h \
    signal_protocol_types.h \
    signal_protocol_internal.h \
    curve.h \
    hkdf.h \
    ratchet.h \
    protocol.h \
    session_state.h \
    session_record.h \
    session_pre_key.h \
    session_builder.h \
    session_builder_internal.h \
    session_cipher.h \
    key_helper.h \
    sender_key.h \
    sender_key_state.h \
    sender_key_record.h \
    group_session_builder.h \
    group_cipher.h \
    fingerprint.h \
    device_consistency.h

include(curve25519/curve25519.pri)
include(protobuf-c/protobuf-c.pri)
