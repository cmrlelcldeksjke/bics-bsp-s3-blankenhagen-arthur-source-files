#define KEYDIR "keys"
/* base of string representation of our numbers
   16 would be better (less characters to store)
   but it's 10 for our testing purposes */
#define BASE 10
#define DELIMITER ':' /* a non-digit char to separate numbers */

/* for mpz_import */
#define WORDORDER 1
#define ENDIANESS 1

size_t mpz_nbytes(mpz_t n);
void import(mpz_t rop, char *path);
size_t getbufsize(mpz_t n);
void xchdir(char *path);
