#!/bin/sh

#
# This file is part of the Code::Blocks IDE and licensed under the GNU General Public License, version 3
# http://www.gnu.org/licenses/gpl-3.0.html
#
# $Rev$
# $Id$
# $HeadURL$
#


# make sure [git-]svn answers in english
export LC_ALL="C"

REV_FILE=./revision.m4

# let's import OLD_REV (if there)
if [ -f ./.last_revision ]; then
	. ./.last_revision
else
	OLD_REV=0
fi

if svn --xml info >/dev/null 2>&1; then
	REV=`svn --xml info | tr -d '\r\n' | sed -e 's/.*<commit.*revision="\([0-9]*\)".*<\/commit>.*/\1/'`
	LCD=`svn --xml info | tr -d '\r\n' | sed -e 's/.*<commit.*<date>\([0-9\-]*\)\T\([0-9\:]*\)\..*<\/date>.*<\/commit>.*/\1 \2/'`
elif svn --info >/dev/null 2>&1; then
	REV=`svn info | grep "^Revision:" | cut -d" " -f2`
	LCD=`svn info | grep "^Last Changed Date:" | cut -d" " -f4,5`
elif git svn --version >/dev/null 2>&1; then
	REV=`git svn info | grep "^Revision:" | cut -d" " -f2`
	LCD=`git svn info | grep "^Last Changed Date:" | cut -d" " -f4,5`
elif git version >/dev/null 2>&1; then
	REV=`git log --max-count=1 | grep -o -e "@\([0-9]*\)" | tr -d '@ '`
	LCD=`git log --max-count=1 | grep -o -e "Date: \(.*\)" | cut -d " " -f 5-9`
else
	REV=0
	LCD=""
fi

if [ "x$REV" != "x$OLD_REV" -o ! -r $REV_FILE ]; then
	echo "m4_define([SVN_REV], $REV)" > $REV_FILE
	echo "m4_define([SVN_REVISION], 0.9.9.2svn$REV)" >> $REV_FILE
	echo "m4_define([SVN_DATE], $LCD)" >> $REV_FILE
fi

echo "OLD_REV=$REV" > ./.last_revision

exit 0
