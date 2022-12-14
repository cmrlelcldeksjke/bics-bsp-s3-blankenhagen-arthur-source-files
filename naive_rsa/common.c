#include <err.h>
#include <stdio.h>
#include <unistd.h>

#include <gmp.h>

#include "common.h"

size_t mpz_nbytes(mpz_t n)
{
    size_t nbits;
    nbits = mpz_sizeinbase(n, 2);
    if (nbits % 8 != 0)
        return nbits/8 + 1;
    else
        return nbits/8;
}

void
import(mpz_t rop, char *path)
{
    FILE *f;

    f = fopen(path, "r");
    if (f == NULL)
        err(1, "import: while opening %s", path);
    if (mpz_inp_str(rop, f, BASE) == 0)
    {
        fclose(f);
        err(1, "mpz_inp_str: while reading %s", path);
    }
    fclose(f);
}

size_t
getbufsize(mpz_t n)
{
    size_t bufsize;

    bufsize = mpz_nbytes(n) - 1;
    /* set a limit to the bufsize, to avoid having it being too big.
       All we need is a number < n, thus one < n-1 works too */
    if (BUFSIZ < bufsize)
        bufsize = BUFSIZ;

    return bufsize;
}

void xchdir(char *path)
{
    if (chdir(path) != 0)
        err(1, "chdir: while changing directory to %s", path);
}
