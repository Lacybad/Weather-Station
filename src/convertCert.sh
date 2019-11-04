#!/bin/bash

#find first file ending with .crt in folder
cert="$(find . -type f -name "*.crt" -print | head -n 1)"

if [ -e "$cert" ]; then
    printf "%s\n%s %s\n%s\n" "//Cert from https://api.darksky.net" \
                          "//Get root cert, download to cert.crt," \
                          "run bash script convertCert.sh" \
                          "#define USE_CERT" > cert.h
    printf "static const char digicert[] PROGMEM = R\"EOF(\n" >> cert.h
    #remove return <0d> or \r
    tr -d '\r' < $cert >> cert.h
    printf ")EOF\";" >> cert.h
    echo "printed file"
else
    echo "no file"
fi
