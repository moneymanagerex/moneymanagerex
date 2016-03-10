#!/bin/sh -e

[ -n "${WXRC}" ] || WXRC=wxrc

XGETTEXT_ARGS="-C -k_ -kwxGetTranslation -kwxTRANSLATE -kwxPLURAL:1,2 -F \
              --add-comments=TRANSLATORS \
              --from-code=UTF-8 \
              --no-location \
              --package-name=MMEX \
              --msgid-bugs-address=developer@moneymanagerex.org"

echo "Extracting strings into ./po/MoneyManagerEx.pot..."
find ./src -name "*.cpp" | xargs xgettext ${XGETTEXT_ARGS}             -o ./po/MoneyManagerEx.pot
find ./src -name "*.h"   | xargs xgettext ${XGETTEXT_ARGS}          -j -o ./po/MoneyManagerEx.pot

echo "Merging into *.po..."
for p in ./po/*.po ; do
    echo "Merging $p ..."
    msgmerge --quiet --update --sort-output --backup=none $p ./po/MoneyManagerEx.pot
done
echo "Finished"
