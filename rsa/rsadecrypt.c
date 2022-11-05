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
    size_t bufsize;
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
    if ((nread = getdelim(&buf, &bufsize, DELIMITER, stdin)) > 0)
    {
        /* normally we output without ending \n, but just in case */
        if (buf[nread-1] == '\n')
            buf[nread-1] = '\0';
        mpz_set_str(mpbuf, buf, BASE);
        free(buf);

        mpz_powm(mpbuf, mpbuf, d, n);
        /* ASSUMPTION: the encrypted message is NUL-terminated */
        msg = mpz_export(NULL, NULL, WORDORDER, sizeof(char), ENDIANESS, 0, mpbuf);
        printf("%s", msg);
        free(msg);
    }

    mpz_clears(mpbuf, n, d, NULL);

    return 0;
}
