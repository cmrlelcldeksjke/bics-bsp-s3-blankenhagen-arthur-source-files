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
mpz_t p, q, totient, d; /* private */

void
genprime(mpz_t rop, gmp_randstate_t state, mp_bitcnt_t bitsize)
{
        do {
            mpz_urandomb(rop, state, bitsize);
        } while (!mpz_probab_prime_p(rop, REPS));
}

void
init_rsa(gmp_randstate_t state, mp_bitcnt_t bitsize)
{
    mpz_t tmp;

    mpz_init(p);
    mpz_init(q);
    genprime(p, state, bitsize);
    genprime(q, state, bitsize);

    mpz_init(n);
    mpz_mul(n, p, q);
    
    /* totient = (p-1) * (q-1) */
    mpz_init(totient);
    mpz_init(tmp);
    mpz_sub_ui(totient, p, 1);
    mpz_sub_ui(tmp, q, 1);
    mpz_mul(totient, totient, tmp);

    mpz_init_set_ui(e, E);

    mpz_init(d);
    mpz_invert(d, e, totient);

    mpz_clear(tmp);
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

    gmp_randinit_default(state);
    getrandom(&seed, sizeof(seed), 0);
    gmp_randseed_ui(state, seed);

    init_rsa(state, BITSIZE);

    xmkdir(keydir);
    xchdir(keydir);
    export("p", p, true);
    export("q", q, true);
    export("totient", totient, true);
    export("d", d, true);
    mpz_clear(p);
    mpz_clear(q);
    mpz_clear(d);
    mpz_clear(totient);
    export("n", n, false);
    export("e", e, false);
    mpz_clear(n);
    mpz_clear(e);

    return 0;
}
