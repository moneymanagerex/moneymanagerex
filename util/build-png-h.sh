#!/bin/sh
#!
#! Converts all input SVG files to PNG files at various resolutions and then
#! creates an icons.h C-Header file from the PNGs
#!
#! Usage: /path/to/build-png-h *.svg

rm -f icons.h
for file in "$@"
do
    filename="$(basename ${file} .svg)"
    for size in 16 24 32 48
    do  
        rsvg-convert -h "${size}" -f png "${file}" > "${filename}_${size}".png
        xxd -i "${filename}_${size}".png | sed "1s/^/static const /" |  sed "s/_\([0-9][0-9]\)/\1/" | awk "NR>1{print buf}{buf = \$0}" >> icons.h
        rm "${filename}_${size}".png
    done
done
