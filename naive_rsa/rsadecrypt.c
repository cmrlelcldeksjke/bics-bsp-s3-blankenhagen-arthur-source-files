#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#include <gmp.h>

#include "common.h"

int
main(int argc, char *argv[])
{
    char *buf, *msg;
    size_t buflen, msglen;
    ssize_t nread;
    mpz_t mpbuf;
    char *keydir;

    mpz_t n; /* public */
    mpz_t d; /* private */

    if (--argc != 1)
        errx(1, "usage: %s <keydir>", argv[0]);
    keydir = argv[1];

    mpz_inits(mpbuf, n, d, NULL);

    xchdir(keydir);
    import(n, "n");
    import(d, "d");
    
    buf = NULL;
    while ((nread = getdelim(&buf, &buflen, DELIMITER, stdin)) > 0)
    {
        if (buf[nread-1] == ':')
            buf[nread-1] = '\0';
        mpz_set_str(mpbuf, buf, BASE);
        mpz_powm(mpbuf, mpbuf, d, n);
        msg = mpz_export(NULL, &msglen, WORDORDER, sizeof(char), ENDIANESS, 0, mpbuf);
        fwrite(msg, sizeof(char), msglen, stdout);
        free(msg);
    }

    free(buf);
    mpz_clears(mpbuf, n, d, NULL);

    return 0;
}
