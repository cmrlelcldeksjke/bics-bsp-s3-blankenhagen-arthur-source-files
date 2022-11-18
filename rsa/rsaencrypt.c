#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
        errx(1, "message too long");

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
        errx(1, "unpadding error");
    }

    return msg;
}

int
main(int argc, char *argv[])
{
    char *msg;
    size_t k, msglen, maxmsglen;
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
    
    k = mpz_nbytes(n);
    maxmsglen = k - 2*HASHLEN - 2;
    msg = calloc(maxmsglen, sizeof(char));
    if (fgets(msg, maxmsglen, stdin) != NULL)
    {
        msglen = strlen(msg);
        /* check if the message was longer than what we read
           For that we check if there is still input */
        if (getchar() != EOF)
            errx(1, "message too long");

        /*mpz_import(mpbuf, msglen, WORDORDER, sizeof(char), ENDIANESS, 0, msg);
        mpz_powm(mpbuf, mpbuf, e, n);
        mpz_out_str(stdout, BASE, mpbuf);*/

        uchar *padded, *unpadded;
        size_t unpaddedlen;
        padded = oaep_pad((uchar *) msg, msglen, k);
        unpadded = oaep_unpad(&unpaddedlen, padded, k);
        printf("%d %d\n", memcmp(msg, unpadded, msglen) == 0, msglen == unpaddedlen);
    }

    free(msg);
    mpz_clears(mpbuf, n, e, NULL);

    return 0;
}
