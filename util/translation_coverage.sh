#!/bin/sh -e

XGETTEXT_ARGS="-k_ -kwxGetTranslation -kwxTRANSLATE -kwxPLURAL:1,2 \
               --language=C++ \
               --from-code=UTF-8"

POT=$(mktemp --suffix=.pot) || exit
trap "rm -f -- '$POT'" EXIT

# echo "Extracting strings into $POT..."
find src -name \*.cpp -o -name \*.h | xgettext -f - $XGETTEXT_ARGS -o "$POT"

for p in po/*.po ; do
    o=$(msgmerge --quiet -o - $p "$POT" | LANG=C msgfmt --statistics -o /dev/null - 2>&1)
    t=$(echo "$o" | sed -r 's/.*([^0-9]|^)([0-9]+) translated.*/\2/;t;c 0')
    f=$(echo "$o" | sed -r 's/.*([^0-9]|^)([0-9]+) fuzzy.*/\2/;t;c 0')
    u=$(echo "$o" | sed -r 's/.*([^0-9]|^)([0-9]+) untranslated.*/\2/;t;c 0')
    a=$((t+f+u))
    printf '%-5s: %3d%%' $(basename -s .po $p) $((t*100/a))
    if [ $f -ne 0 ]; then
        printf ' %*d fuzzy' ${#a} $f
    fi
    echo
done

rm -f -- "$POT"
trap - EXIT
exit
