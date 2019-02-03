#!/bin/sh -e

XGETTEXT_ARGS="-k_ -kwxGetTranslation -kwxTRANSLATE -kwxPLURAL:1,2 \
               --language=C++ \
               --sort-by-file \
               --add-comments=TRANSLATORS \
               --from-code=UTF-8 \
               --add-location \
               --package-name=MMEX \
               --no-wrap \
               --msgid-bugs-address=developer@moneymanagerex.org"
MSGMERGE_ARGS="--quiet \
               --update \
               --sort-by-file \
               --add-location \
               --no-wrap \
               --backup=none"
SED_ARGS="--null-data \
          --regexp-extended \
          --in-place"
POT=mmex.pot

echo "Extracting strings into po/$POT..."
find src \( -name \*.cpp -o -name \*.h \) -printf '%P\n' | xgettext -D src -f - $XGETTEXT_ARGS -o po/$POT

echo "Merging into *.po..."
for p in po/*.po ; do
    echo "Merging $p ..."
    msgmerge $MSGMERGE_ARGS $p po/$POT
    sed $SED_ARGS 's/(msgid|msgstr) ""\n"/\1 "/;s/\\n"\n"\\n"\n/\\n\\n"\n/;s/(msgid ""\nmsgstr ")/\1"\n"/' $p
done
echo "Finished"
