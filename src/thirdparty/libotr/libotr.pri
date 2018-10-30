HEADERS = \
    src/b64.h \
    src/context.h \
    src/dh.h mem.h \
    src/message.h \
    src/privkey.h \
    src/proto.h \
    src/version.h \
    src/userstate.h \
    src/tlv.h \
    src/serial.h \
    src/auth.h \
    src/sm.h \
    src/privkey-t.h \
    src/context_priv.h \
    src/instag.h \
    toolkit/aes.h \
    toolkit/ctrmode.h \
    toolkit/parse.h \
    toolkit/sesskeys.h \
    toolkit/readotr.h \
    toolkit/sha1hmac.h

SOURCES = \
    src/privkey.c \
    src/context.c \
    src/proto.c \
    src/b64.c \
    src/dh.c \
    src/mem.c \
    src/message.c \
    src/userstate.c \
    src/tlv.c \
    src/auth.c \
    src/sm.c \
    src/context_priv.c \
    src/instag.c \
    toolkit/parse.c \
    toolkit/sha1hmac.c \
    toolkit/otr_parse.c \
    toolkit/readotr.c\
    toolkit/otr_sesskeys.c \
    toolkit/sesskeys.c \
    toolkit/otr_mackey.c \
    toolkit/aes.c \
    toolkit/ctrmode.c \
    toolkit/otr_readforge.c \
    toolkit/otr_modify.c \
    toolkit/otr_remac.c

