#!/bin/sh -e

sed -i -z -r 's_(#, fuzzy\n)*#~ msgid "([^"]*)"\n#~ msgstr "(|\2)"\n\n__g' po/*.po