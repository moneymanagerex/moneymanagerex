#!/bin/zsh
#!
#! Converts all input SVG files to PNG files at various resolutions
#! Creates an icons.h C-Header file from the PNGs
#! Generates README.md file so that images can be viewed at various resolutions
#!
#! Usage: /path/to/build-png-h *.svg

rm -f icons.h
echo "# Icons" > README.md
echo "enum | x16 | x24 | x32 | x48" >> README.md
echo ":-- | --- | --- | --- | ---" >> README.md
for file in "$@"
do
    filename="${file%.*}"
    echo -n "${filename}" >> README.md
    for size in 16 24 32 48
    do  
        printf " | <img src=\"%s\" width=\"%s\"> " "${file}" "${size}" >> README.md
        rsvg-convert -h "${size}" -f png "${file}" > "${filename}_${size}".png
        xxd -i "${filename}_${size}".png | sed "1s/^/static const /" |  sed "s/_\([0-9][0-9]\)/\1/" | awk "NR>1{print buf}{buf = \$0}" >> icons.h
        rm "${filename}_${size}".png
    done
    echo "" >> README.md
done
