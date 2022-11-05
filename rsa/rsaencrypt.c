#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gmp.h>

#include "common.h"

int
main(int argc, char *argv[])
{
    char *msg;
    size_t msgsize, maxmsgsize;
    mpz_t mpbuf;
    char *keydir;

    mpz_t n, e; /* public */

    if (--argc != 1)
        errx(1, "usage: %s <keydir>", argv[0]);
    keydir = argv[1];

    mpz_inits(mpbuf, n, e, NULL);

    xchdir(keydir);
    import(n, "n");
    import(e, "e");
    
    maxmsgsize = mpz_nbytes(n) - 1;
    msg = calloc(maxmsgsize, sizeof(char));
    if (fgets(msg, maxmsgsize, stdin) != NULL)
    {
        msgsize = strlen(msg) + 1;
        /* check if the message was longer than what we read
           For that we check if there is still input */
        if (getchar() != EOF)
            errx(1, "message too long");

        mpz_import(mpbuf, msgsize, WORDORDER, sizeof(char), ENDIANESS, 0, msg);
        mpz_powm(mpbuf, mpbuf, e, n);
        mpz_out_str(stdout, BASE, mpbuf);
    }

    free(msg);
    mpz_clears(mpbuf, n, e, NULL);

    return 0;
}
