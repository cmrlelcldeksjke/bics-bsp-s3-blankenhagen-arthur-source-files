#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <gmp.h>

#include "common.h"

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
    if (mpz_fits_ulong_p(n))
    {
        bufsize = mpz_get_ui(n);
        /* if n fits an unsigned long but is bigger than BUFSIZ, use BUFSIZ */
        if (bufsize > BUFSIZ)
            bufsize = BUFSIZ;
        /* otherwise keep n-1 as our bufsize */
        else
            bufsize--;
    }
    /* if n can't fit an unsigned long it certainly is bigger than BUFSIZ */
    else
        bufsize = BUFSIZ;

    return bufsize;
}

int
main()
{
    char *buf;
    size_t bufsize;
    size_t nread;
    mpz_t mpbuf;

    mpz_t n, e; /* public */

    if (chdir(KEYDIR) != 0)
        err(1, "chdir: while changing directory to %s", KEYDIR);
    import(n, "n");
    import(e, "e");
    
    bufsize = getbufsize(n);
    buf = calloc(bufsize, sizeof(char));
    while ((nread = fread(buf, sizeof(char), bufsize, stdin)) > 0)
    {
        mpz_import(mpbuf, nread, WORDORDER, sizeof(char), ENDIANESS, 0, buf);
        mpz_powm(mpbuf, mpbuf, e, n);
        mpz_out_str(stdout, BASE, mpbuf);
    }

    return 0;
}
