#include <assert.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <openssl/sha.h>

#include <gmp.h>

#include "common.h"

/* cached result of SHA256("") */
uchar EMPTY_STR_HASH[HASHLEN] =
{
    0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14,
    0x9a, 0xfb, 0xf4, 0xc8, 0x99, 0x6f, 0xb9, 0x24,
    0x27, 0xae, 0x41, 0xe4, 0x64, 0x9b, 0x93, 0x4c,
    0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52, 0xb8, 0x55,
};


void
xchdir(char *path)
{
    if (chdir(path) != 0)
        err(1, "chdir: while changing directory to %s", path);
}

void
xor(uchar *dest, size_t size, uchar *op1, uchar *op2)
{
    for (size_t i = 0; i < size; i++)
        dest[i] = op1[i] ^ op2[i];
}

size_t
mpz_nbytes(mpz_t n)
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

/* I2OSP converts a nonnegative integer to an octet string of a
   specified length. (RFC 8017 section 4.1) */
void
i2osp(uchar *res, size_t len, uint n)
{
    for (int i = len-1; i >= 0; i--)
    {
        res[i] = n % 256;
        n /= 256;
    }
}

void
mgf1(uchar *mask, size_t masklen, uchar *seed, size_t seedlen)
{
    uchar si[4]; /* octet string of i */
    uchar hash[HASHLEN];
    uchar *tohash;
    size_t tohashlen, curmasklen;

    /* can't do this comparison because of overflow */
    /*if (masklen > (2 << 32)*HASHLEN)
        errx(1, "mgf1: mask too long");*/
    
    curmasklen = masklen;
    tohashlen = seedlen + nelem(si);

    tohash = calloc(tohashlen, sizeof(uchar));
    for (size_t i = 0; i <= masklen/HASHLEN; i++)
    {
        i2osp(si, nelem(si), i);

        /* hash(seed + si) */
        memcpy(tohash, seed, seedlen);
        memcpy(tohash+seedlen, si, nelem(si));
        SHA256(tohash, tohashlen, hash);

        /* mask += hash */
        memcpy(mask + i*HASHLEN, hash, min(curmasklen, HASHLEN));
        if (curmasklen - HASHLEN < 0)
        {
            assert(i == masklen/HASHLEN);
            curmasklen = 0;
        }
        else
            curmasklen -= HASHLEN;
    }

    free(tohash);
}
