#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/random.h>

#include <gmp.h>
#include <openssl/sha.h>

#include "common.h"

/* k is the length in bytes of n
   oaep_pad returns a padded octet string of length k */
uchar *
oaep_pad(uchar *msg, size_t msglen, size_t k)
{
    uchar *db, *dbmask, *maskeddb;
    uchar *seed, *seedmask, *maskedseed;
    uchar *padded;
    size_t pslen, dblen;
    size_t i;

    pslen = k - msglen - 2*HASHLEN - 2;
    dblen = k - HASHLEN - 1;

    if (msglen > k - 2*HASHLEN - 2)
    {
          warnx("message too long");
          return NULL;
    }

    db = calloc(dblen, sizeof(uchar));
    dbmask = calloc(dblen, sizeof(uchar));
    maskeddb = calloc(dblen, sizeof(uchar));
    seed = calloc(HASHLEN, sizeof(uchar));
    seedmask = calloc(HASHLEN, sizeof(uchar));
    maskedseed = calloc(HASHLEN, sizeof(uchar));
    padded = calloc(k, sizeof(uchar));

    /* DB = hash("") + PS + 1 + M
       PS consists of only 0s */
    for (i = 0; i < HASHLEN; i++)
        db[i] = EMPTY_STR_HASH[i];
    /* we don't need to explicitly set PS because by default
     * our memory is set to 0 with calloc */
    i = HASHLEN+pslen;
    db[i++] = 1;
    for (size_t j = 0; i < dblen; i++, j++)
        db[i] = msg[j];

    getrandom(seed, HASHLEN, 0);

    mgf1(dbmask, dblen, seed, HASHLEN);
    xor(maskeddb, dblen, db, dbmask);

    mgf1(seedmask, HASHLEN, maskeddb, dblen);
    xor(maskedseed, HASHLEN, seed, seedmask);

    /* padded = 0 + maskedseed + maskeddb */
    i = 0;
    padded[i++] = 0;
    for (size_t j = 0; i < HASHLEN+1; i++, j++)
        padded[i] = maskedseed[j];
    for (size_t j = 0; i < k; i++, j++)
        padded[i] = maskeddb[j];

    free(db);
    free(dbmask);
    free(maskeddb);
    free(seed);
    free(seedmask);
    free(maskedseed);
    return padded;
}

int
main(int argc, char *argv[])
{
    uchar *msg, *padded;
    size_t k, msglen, maxmsglen;
    mpz_t mpbuf;
    char *keydir;

    mpz_t n, e; /* public */

    if (--argc != 1)
        errx(1, "usage: %s <keydir>", argv[0]);
    keydir = argv[1];

    xchdir(keydir);

    mpz_inits(mpbuf, n, e, NULL);
    if (!import(n, "n"))
    {
            mpz_clears(mpbuf, n, e, NULL);
            return 1;
    }
    if (!import(e, "e"))
    {
            mpz_clears(mpbuf, n, e, NULL);
            return 1;
    }
    
    k = mpz_nbytes(n);
    maxmsglen = k - 2*HASHLEN - 2;
    msg = calloc(maxmsglen, sizeof(uchar));
    msglen = fread(msg, sizeof(uchar), maxmsglen, stdin);
    if (msglen > 0)
    {
        /* check if the message was longer than what we read
           For that we check if there is still input */
        if (getchar() != EOF)
        {
            free(msg);
            mpz_clears(mpbuf, n, e, NULL);
            errx(1, "message too long");
        }

        padded = oaep_pad((uchar *) msg, msglen, k);
        if (padded == NULL)
        {
            free(msg);
            mpz_clears(mpbuf, n, e, NULL);
            return 1;
        }
        mpz_import(mpbuf, k, WORDORDER, sizeof(char), ENDIANESS, 0, padded);
        mpz_powm(mpbuf, mpbuf, e, n);
        mpz_out_raw(stdout, mpbuf);

        free(padded);
    }

    free(msg);
    mpz_clears(mpbuf, n, e, NULL);

    return 0;
}
