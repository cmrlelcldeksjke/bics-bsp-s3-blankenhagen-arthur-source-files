#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gmp.h>

#include "common.h"

/* k is the length in bytes of n
   oaep_unpad returns the unpadded message, and if msglen is not NULL
   it will contain the message length */
uchar *
oaep_unpad(size_t *msglen, uchar *padded, size_t k)
{
    uchar *db, *dbmask, *maskeddb;
    uchar *seed, *seedmask, *maskedseed;
    uchar hash[HASHLEN];
    uchar *msg;
    size_t dblen;
    uchar y, onesep;
    size_t i;

    dblen = k - HASHLEN - 1;

    db = calloc(dblen, sizeof(uchar));
    dbmask = calloc(dblen, sizeof(uchar));
    maskeddb = calloc(dblen, sizeof(uchar));
    seed = calloc(HASHLEN, sizeof(uchar));
    seedmask = calloc(HASHLEN, sizeof(uchar));
    maskedseed = calloc(HASHLEN, sizeof(uchar));
    msg = calloc(k, sizeof(uchar));

    /* padded = y + maskedseed + maskeddb */
    i = 0;
    y = padded[i++];
    for (size_t j = 0; i < HASHLEN+1; i++, j++)
        maskedseed[j] = padded[i];
    for (size_t j = 0; i < k; i++, j++)
        maskeddb[j] = padded[i];

    mgf1(seedmask, HASHLEN, maskeddb, dblen);
    xor(seed, HASHLEN, maskedseed, seedmask);

    mgf1(dbmask, dblen, seed, HASHLEN);
    xor(db, dblen, maskeddb, dbmask);

    /* DB = hash("") + PS + 1 + M
       PS consists of only 0s */
    for (i = 0; i < HASHLEN; i++)
        hash[i] = db[i];
    for (; db[i] == 0 && i < k-1; i++);
    onesep = db[i++];

    if (msglen != NULL)
        *msglen = 0;
    for (size_t j = 0; i < dblen; i++, j++)
    {
        msg[j] = db[i];
        if (msglen != NULL)
            (*msglen)++;
    }

    free(db);
    free(dbmask);
    free(maskeddb);
    free(seed);
    free(seedmask);
    free(maskedseed);

    /* error detection
       Done at the end to prevent from timing attacks and
       to be sure we freed everything */
    if (onesep != 1 || y != 0 || memcmp(hash, EMPTY_STR_HASH, HASHLEN) != 0)
    {
        free(msg);
        warnx("unpadding error");
        return NULL;
    }

    return msg;
}

int
main(int argc, char *argv[])
{
    char *buf;
    uchar *padded, *msg;
    size_t k, _bufsize, paddedlen, msglen;
    ssize_t nread;
    mpz_t mpbuf;
    char *keydir;

    mpz_t n; /* public */
    mpz_t d; /* private */

    if (--argc != 1)
        errx(1, "usage: %s <keydir>", argv[0]);
    keydir = argv[1];

    xchdir(keydir);

    mpz_inits(mpbuf, n, d, NULL);
    if (!import(n, "n"))
    {
        mpz_clears(mpbuf, n, d, NULL);
        return 1;
    }
    if (!import(d, "d"))
    {
        mpz_clears(mpbuf, n, d, NULL);
        return 1;
    }
    
    k = mpz_nbytes(n);
    padded = NULL;
    /* _bufsize is unused and only needed by getline */
    if ((nread = getline(&buf, &_bufsize, stdin)) > 0)
    {
        mpz_set_str(mpbuf, buf, BASE);
        mpz_powm(mpbuf, mpbuf, d, n);

        /* mpz_export skips the leading 0s, so we need to add them ourselves */
        paddedlen = mpz_nbytes(mpbuf);
        if (paddedlen > k)
        {
            free(buf);
            mpz_clears(mpbuf, n, d, NULL);
            errx(1, "invalid padding");
        }
        padded = calloc(paddedlen, sizeof(char));
        mpz_export(padded+(k-paddedlen), NULL, WORDORDER, sizeof(char), ENDIANESS, 0, mpbuf);

        msg = oaep_unpad(&msglen, padded, k);
        if (msg == NULL)
        {
            free(buf);
            free(padded);
            mpz_clears(mpbuf, n, d, NULL);
            return 1;
        }
        msg = reallocarray(msg, msglen+1, sizeof(char));
        msg[msglen] = '\0';
        printf("%s", msg);

        free(padded);
        free(msg);
    }

    free(buf);
    mpz_clears(mpbuf, n, d, NULL);

    return 0;
}
