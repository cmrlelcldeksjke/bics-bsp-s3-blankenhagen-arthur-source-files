#!/bin/sh
tmp=$(mktemp opensslcrypt.XXXXXXXXXX)
openssl pkeyutl -inkey key.der -encrypt -pkeyopt rsa_padding_mode:oaep -pkeyopt rsa_oaep_md:SHA256 > "$tmp"
wc -c < "$tmp" | ./tobin | cat - "$tmp" | ./rsadecrypt keys
rm "$tmp"
