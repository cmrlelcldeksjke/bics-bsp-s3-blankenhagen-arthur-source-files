#!/bin/sh
./rsaencrypt keys | tail -c +5 | openssl pkeyutl -inkey key.der -decrypt -pkeyopt rsa_padding_mode:oaep -pkeyopt rsa_oaep_md:SHA256
