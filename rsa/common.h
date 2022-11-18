#include <openssl/sha.h>

#define KEYDIR "keys"
/* base of string representation of our numbers
   16 would be better (less characters to store)
   but it's 10 for our testing purposes */
#define BASE 10
#define DELIMITER ':' /* a non-digit char to separate numbers */

/* for mpz_import */
#define WORDORDER 1
#define ENDIANESS 1

#define HASHLEN SHA256_DIGEST_LENGTH
#define nelem(arr) (sizeof(arr)/sizeof(arr[0]))
#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))

typedef unsigned char uchar;
typedef unsigned int uint;

extern unsigned char EMPTY_STR_HASH[HASHLEN];
void xchdir(char *path);
void xor(uchar *dest, size_t size, uchar *op1, uchar *op2);
size_t mpz_nbytes(mpz_t n);
void import(mpz_t rop, char *path);
void i2osp(uchar *res, size_t len, unsigned n);
void mgf1(uchar *mask, size_t masklen, uchar *seed, size_t seedlen);
