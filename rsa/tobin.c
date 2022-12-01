/* convert input ASCII base10 integer to big-endian 4-byte sequence */

#include <err.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <arpa/inet.h>

int
main()
{
    char s[10]; /* max value in 4 bytes is 4294967295 (2^(32)-1) */
    int i;
    int c;
    uint32_t n; /* 4 bytes = 32 bits */

    while ((c = getchar()) != EOF && i < 10)
        s[i++] = c;

    n = atoi(s);
    n = htonl(n);
    fwrite(&n, sizeof(n), 1, stdout);
    return 0;
}
