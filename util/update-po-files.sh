#!/bin/sh -e

XGETTEXT_ARGS="-k_ -kwxGetTranslation -kwxTRANSLATE -kwxPLURAL:1,2 \
               --language=C++ \
               --sort-by-file \
               --add-comments=TRANSLATORS \
               --from-code=UTF-8 \
               --add-location \
               --package-name=MMEX \
               --width=78 \
               --msgid-bugs-address=developer@moneymanagerex.org"
MSGMERGE_ARGS="--quiet \
               --update \
               --sort-by-file \
               --add-location \
               --width=78 \
               --backup=none"
POT=MoneyManagerEx.pot

echo "Extracting strings into po/$POT..."
find src -name \*.cpp -o -name \*.h | xgettext -f - $XGETTEXT_ARGS -o po/$POT

echo "Merging into *.po..."
for p in po/*.po ; do
    echo "Merging $p ..."
    msgmerge $MSGMERGE_ARGS $p po/$POT
done
echo "Finished"
