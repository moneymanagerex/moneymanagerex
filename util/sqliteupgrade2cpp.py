#!/usr/bin/env python
# vi:tabstop=4:expandtab:shiftwidth=4:softtabstop=4:autoindent:smarttab:fileencoding=utf-8
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

def split_string(string, charleng):
    return [string[i:i+charleng] for i in range(0, len(string), charleng)]

StrHeader = '''/** @file
 * @brief     Compilation of SQL scripts to upgrade MMEX db version
 * @warning   Auto generated with %s script. DO NOT EDIT!
 * @copyright © 2016-2018 Gabriele-V
 * @date      %s
 */
'''% (os.path.basename(__file__), str(datetime.datetime.now()))

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
    FileContent = getFileContent(sqlfile).replace('\n','\n\t\t')
    LatestVersion = getVersion(sqlfile)
    StrUpgradeQuery += '''\t// Upgrade to version %i'''%(LatestVersion)
    for string in split_string(FileContent, 10240):
        StrUpgradeQuery += '''\n\tR"(%s)"'''% (string)
    StrUpgradeQuery += ''',\n\n'''

StrUpgradeQuery += '''};\n'''

StrLatestVersion = '''
const int dbLatestVersion = %i;
'''% (LatestVersion)

strEnd = '''
#endif // DB_UPGRADE_H_
'''

fp = open('DB_Upgrade.h', 'w')
fp.write(StrHeader + StrLatestVersion + StrUpgradeQuery + strEnd)
fp.close