#!/bin/bash

mergeLib="digikey-kicad-library.lib"

if [ -f $mergeLib ]; then
    echo "Removing old file"
    rm $mergeLib
fi

echo "merging files"

cat *.lib > $mergeLib
