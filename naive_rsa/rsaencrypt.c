#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <gmp.h>

#include "common.h"

int
main(int argc, char *argv[])
{
    char *buf;
    size_t bufsize;
    size_t nread;
    mpz_t mpbuf;
    bool isfirstit = true;
    char *keydir;

    mpz_t n, e; /* public */

    if (--argc != 1)
        errx(1, "usage: %s <keydir>", argv[0]);
    keydir = argv[1];

    mpz_inits(mpbuf, n, e, NULL);

    xchdir(keydir);
    import(n, "n");
    import(e, "e");
    
    bufsize = getbufsize(n);
    buf = calloc(bufsize, sizeof(char));
    while ((nread = fread(buf, sizeof(char), bufsize, stdin)) > 0)
    {
        mpz_import(mpbuf, nread, WORDORDER, sizeof(char), ENDIANESS, 0, buf);
        mpz_powm(mpbuf, mpbuf, e, n);
        if (!isfirstit)
            putchar(DELIMITER);
        mpz_out_str(stdout, BASE, mpbuf);
        isfirstit = false;
    }

    free(buf);
    mpz_clears(mpbuf, n, e, NULL);

    return 0;
}
