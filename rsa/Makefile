CC = cc
CFLAGS = -std=c99 -pedantic -Wall -Wno-deprecated-declarations -g
LDFLAGS = -lgmp -lcrypto

all: rsagen rsaencrypt rsadecrypt

rsagen: common.c rsagen.c
	${CC} ${CFLAGS} ${LDFLAGS} common.c rsagen.c -o $@

rsaencrypt: common.c rsaencrypt.c
	${CC} ${CFLAGS} ${LDFLAGS} common.c rsaencrypt.c -o $@

rsadecrypt: common.c rsadecrypt.c
	${CC} ${CFLAGS} ${LDFLAGS} common.c rsadecrypt.c -o $@

%: %.c
	${CC} ${CFLAGS} ${LDFLAGS} $< -o $@

clean:
	rm rsagen rsaencrypt rsadecrypt

.PHONY: all clean
