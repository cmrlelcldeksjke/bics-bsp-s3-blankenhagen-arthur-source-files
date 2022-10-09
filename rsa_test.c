#include <stdio.h>
#include <gmp.h>

mpz_t p, q, n, totient;
mpz_t e, d;

void
print_is_prime(mpz_t n)
{
    char *s;

    switch(mpz_probab_prime_p(n, 50))
    {
    case 0:
        s = "not ";
        break;
    case 1:
        s = "maybe ";
        break;
    default:
        s = "";
        break;
    }
    gmp_printf("%Zd is %sa prime\n", n, s);
}

void
init_rsa()
{
    mpz_t tmp;

    mpz_init_set_ui(p, 23);
    mpz_init_set_ui(q, 31);

    mpz_init(n);
    mpz_mul(n, p, q);
    
    /* totient = 22 * 30 = 660 */
    mpz_init(totient);
    mpz_init(tmp);
    mpz_sub_ui(totient, p, 1);
    mpz_sub_ui(tmp, q, 1);
    mpz_mul(totient, totient, tmp);

    mpz_init_set_ui(e, 7);

    mpz_init(d);
    mpz_invert(d, e, totient);
}

void
test_rsa()
{
    mpz_t tmp;
    mpz_t msg;

    mpz_init(tmp);
    mpz_init_set_ui(msg, 5);

    gmp_printf("p = %Zd\n", p);
    print_is_prime(p);
    gmp_printf("q = %Zd\n", q);
    print_is_prime(q);
    gmp_printf("n = %Zd\n", n);
    gmp_printf("totient = %Zd\n", totient);
    gmp_printf("e = %Zd\n", e);
    mpz_gcd(tmp, e, totient);
    gmp_printf("gcd(e, totient) = %Zd\n", tmp);
    gmp_printf("d = %Zd\n", d);

    gmp_printf("m = %Zd\n", msg);
    mpz_powm(tmp, msg, e, n);
    gmp_printf("m^e = %Zd\n", tmp);
    mpz_powm(tmp, tmp, d, n);
    gmp_printf("m^e^d = %Zd\n", tmp);
}

int
main()
{
    init_rsa();
    test_rsa();

    return 0;
}
