#!/bin/sh -e

XGETTEXT_ARGS="-k_ -kN_ -kwxGetTranslation -kwxTRANSLATE -kwxPLURAL:1,2 -kwxPLURAL_U8:1,2 \
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
POT=mmex.pot

echo "Extracting strings into po/$POT..."
cd src
find . \( -name \*.cpp -o -name \*.h \) ! -name 'DB_Upgrade.h' ! -name 'DB_Table_Currencyformats_V1.h' | xgettext -f - $XGETTEXT_ARGS -o "../po/$POT"

echo "Merging into *.po..."
for p in ../po/*.po ; do
    echo "Merging $p ..."
    msgmerge $MSGMERGE_ARGS "$p" "../po/$POT"
done

cd ..
echo "Finished"
