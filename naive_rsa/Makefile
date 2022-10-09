CC = cc
CFLAGS = -std=c99 -pedantic -Wall -Wno-deprecated-declarations
LDFLAGS = -lgmp

all: rsagen

%: %.c
	${CC} ${CFLAGS} ${LDFLAGS} $< -o $@

.PHONY: all
