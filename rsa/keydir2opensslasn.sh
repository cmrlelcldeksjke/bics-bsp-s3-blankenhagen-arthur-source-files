#!/bin/sh
# TODO check if we are in a keydir

if [[ -z $1 ]]; then
    echo "usage: $0 <keydir>"
    exit 1
fi

cwd=$PWD
cd "$1"

n=0x$(cat n)
e=0x$(cat e)
d=0x$(cat d)
p=0x$(cat p)
q=0x$(cat q)
exp1=0x$(cat exp1)
exp2=0x$(cat exp2)
coeff=0x$(cat coeff)

s="asn1=SEQUENCE:private_key
[private_key]
version=INTEGER:0

n=INTEGER:$n

e=INTEGER:$e

d=INTEGER:$d

p=INTEGER:$p

q=INTEGER:$q

exp1=INTEGER:$exp1

exp2=INTEGER:$exp2

coeff=INTEGER:$coeff"

cd "$cwd"
tmp=$(mktemp opensslasn.XXXXXXXXXX)
echo "$s" > "$tmp"
openssl asn1parse -genconf "$tmp" -noout -out key.der
rm "$tmp"
openssl pkey -in key.der -inform DER -out key.pem
