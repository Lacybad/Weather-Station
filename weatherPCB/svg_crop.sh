#!/bin/bash

for i in *.svg; do
    echo -e "Cropping $i"
    inkscape --file="$i" --export-area-drawing --export-plain-svg="$i"
done
