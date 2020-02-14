#!/usr/bin/env python
# vi:tabstop=4:expandtab:shiftwidth=4:softtabstop=4:autoindent:smarttab
'''
Usage: python sqliteupgrade2cpp.py path_to_database_folder
'''

import datetime
import os
import sys
import re
import glob

numbers = re.compile(r'(\d+)')
def numericalSort(value):
    parts = numbers.split(value)
    parts[1::2] = map(int, parts[1::2])
    return parts

def getVersion(path):
    Version = numbers.search(os.path.basename(path)).group(1)
    return int(Version) if Version else 0

def getFileContent(FileName):
    if os.path.exists(FileName):
        fp = open(FileName, "r")
        content = fp.read()
        fp.close()
        return content

StrHeader = '''//=============================================================================
/**
 *      Copyright (c) 2016 - %s Gabriele-V
 *
 *      @author [%s]
 *
 *      @brief
 *
 *      Revision History:
 *          AUTO GENERATED at %s.
 *          DO NOT EDIT!
 */
//=============================================================================
'''% (datetime.date.today().year, os.path.basename(__file__), str(datetime.datetime.now()))

StrHeader += '''
#ifndef DB_UPGRADE_H_
#define DB_UPGRADE_H_

#include <vector>
#include <wx/string.h>
'''

StrUpgradeQuery = '''
const std::vector<wxString> dbUpgradeQuery =
{
'''

LatestVersion = 0
folder = sys.argv[1]
for sqlfile in sorted(glob.glob(os.path.join(folder, 'database_version_*.sql')), key=numericalSort):
    FileContent = getFileContent(sqlfile).replace('\n','\n        ')
    LatestVersion = getVersion(sqlfile)
    StrUpgradeQuery += '''    // Upgrade to version %i
    R"(
        %s
    )",

'''% (LatestVersion, FileContent)

StrUpgradeQuery += '''};
'''

StrLatestVersion = '''
const int dbLatestVersion = %i;
'''% (LatestVersion)

strEnd = '''
#endif // DB_UPGRADE_H_
'''

fp = open('DB_Upgrade.h', 'w')
fp.write(StrHeader + StrLatestVersion + StrUpgradeQuery + strEnd)
fp.close