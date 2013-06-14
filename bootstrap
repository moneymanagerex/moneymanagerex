#!/bin/sh

cd build/bakefiles
bakefile_gen

cd ../../auxd
bakefilize -c -v

cd ..
aclocal -I m4 -I /usr/local/share/aclocal
autoconf
