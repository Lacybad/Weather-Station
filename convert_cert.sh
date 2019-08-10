#!/bin/bash

certFile="CACert.ino"
certDERu="${1//./_}"
certTMP="cacert.h"

if [[ $# -ne 1 ]]; then
    echo "Download cert from website, name as <file>.der, run with program"
    echo "Needed because openssl can't get root cert from COMPUTER"
    exit 1
fi

#convert
#openssl x509 -outform der -in $1 -out $certDER
xxd -i $1 > $certTMP

mv $certTMP $certFile

#text replace
sed -i -e "s/unsigned char $certDERu\[\]/const unsigned char caCert\[\] PROGMEM/g" $certFile
sed -i -e "s/unsigned int ${certDERu}_len/const unsigned int caCertLen/g" $certFile

#remove old files
#rm cert*.pem $certDER 

exit 0
