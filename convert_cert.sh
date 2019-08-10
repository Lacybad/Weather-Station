#!/bin/bash

certDERu="${1//./_}"
certFile="CACert.h"

if [[ $# -ne 1 ]]; then
    echo "Download cert from website, name as <file>.der, run with program"
    echo "Needed because openssl can't get root cert from COMPUTER"
    exit 1
fi

#convert
#openssl x509 -outform der -in $1 -out $certDER
echo -e "Converted to .h"
xxd -i $1 > $certFile

#text replace
echo -e "Replaced text"
sed -i -e "s/unsigned char $certDERu\[\]/const unsigned char caCert\[\] PROGMEM/g" $certFile
sed -i -e "s/unsigned int ${certDERu}_len/const unsigned int caCertLen/g" $certFile

exit 0
