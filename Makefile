CC = cc
CFLAGS = -std=c99 -pedantic -Wall -Wno-deprecated-declarations
LDFLAGS = -lgmp

all: rsa_test

%: %.c
	${CC} ${CFLAGS} ${LDFLAGS} $< -o $@

.PHONY: all
