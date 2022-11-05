CC = cc
CFLAGS = -std=c99 -pedantic -Wall -Wno-deprecated-declarations
LDFLAGS = -lgmp

all: rsagen rsaencrypt rsadecrypt

rsagen: common.c rsagen.c
	${CC} ${CFLAGS} ${LDFLAGS} common.c rsagen.c -o $@

rsaencrypt: common.c rsaencrypt.c
	${CC} ${CFLAGS} ${LDFLAGS} common.c rsaencrypt.c -o $@

rsadecrypt: common.c rsadecrypt.c
	${CC} ${CFLAGS} ${LDFLAGS} common.c rsadecrypt.c -o $@

%: %.c
	${CC} ${CFLAGS} ${LDFLAGS} $< -o $@

.PHONY: all
