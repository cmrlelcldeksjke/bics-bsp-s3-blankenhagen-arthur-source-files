/* rsagen uses the getrandom function from Linux
   This program may not work on other platforms */

#include <err.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/random.h>
#include <sys/stat.h>

#include <gmp.h>

#include "common.h"

#define E 65537
#define REPS 50 /* for mpz_probab_prime_p */

mpz_t n, e; /* public */
mpz_t d; /* private */
mpz_t p, q, expo1, expo2, coeff; /* private, for DER/PEM */

void
genprime(mpz_t rop, gmp_randstate_t state, mp_bitcnt_t bitsize)
{
        do {
            mpz_urandomb(rop, state, bitsize);
        } while (!mpz_probab_prime_p(rop, REPS));
}

void
genkeys(gmp_randstate_t state, mp_bitcnt_t bitsize)
{
    mpz_t totient, p1, q1;

    mpz_init(p);
    mpz_init(q);
    genprime(p, state, bitsize);
    genprime(q, state, bitsize);

    mpz_init(n);
    mpz_mul(n, p, q);
    
    mpz_init(p1);
    mpz_init(q1);
    mpz_sub_ui(p1, p, 1);
    mpz_sub_ui(q1, q, 1);

    mpz_init(totient);
    mpz_mul(totient, p1, q1);

    mpz_init_set_ui(e, E);

    mpz_init(d);
    mpz_invert(d, e, totient);

    mpz_init(expo1);
    mpz_mod(expo1, d, p1);

    mpz_init(expo2);
    mpz_mod(expo2, d, q1);

    mpz_init(coeff);
    mpz_invert(coeff, q, p);

    mpz_clears(totient, p1, q1, NULL);
}

/* only create if non-existing */
void
xmkdir(char *name)
{
    if (mkdir(name, 0755) != 0 && errno != EEXIST)
        err(1, "failed to create directory %s", name);
}

void
export(char *path, mpz_t num, bool isprivate)
{
    FILE *f;

    if (access(path, F_OK) == 0)
    {
        fprintf(stderr, "WARNING: file '%s' already exists. No write will be done on it\n", path);
        return;
    }

    f = fopen(path, "w");
    if (isprivate)
        chmod(path, 0600);
    mpz_out_str(f, BASE, num);
    fclose(f);
}

int
main(int argc, char *argv[])
{
    mp_bitcnt_t BITSIZE = 1024;
    gmp_randstate_t state;
    unsigned long seed;
    char *keydir;

    if (--argc != 1)
        errx(1, "usage: %s <keydir>", argv[0]);
    keydir = argv[1];

    xmkdir(keydir);
    xchdir(keydir);

    gmp_randinit_default(state);
    getrandom(&seed, sizeof(seed), 0);
    gmp_randseed_ui(state, seed);

    genkeys(state, BITSIZE);

    export("d", d, true);
    export("p", p, true);
    export("q", q, true);
    export("exp1", expo1, true);
    export("exp2", expo2, true);
    export("coeff", coeff, true);
    /* clear as soon as possible the private data */
    mpz_clears(d, p, q, expo1, expo2, coeff, NULL);
    export("n", n, false);
    export("e", e, false);
    mpz_clears(n, e, NULL);

    return 0;
}
