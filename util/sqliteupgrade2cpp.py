#!/usr/bin/env python
# vi:tabstop=4:expandtab:shiftwidth=4:softtabstop=4:autoindent:smarttab:fileencoding=utf-8
'''
Usage: python sqliteupgrade2cpp.py path_to_database_folder
'''

import os
import sys
import re
import glob
import subprocess
from io import open

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
        with open(FileName, "r", encoding='utf-8') as fp:
            content = fp.read()
        return content

def split_string(string, charleng):
    return [string[i:i+charleng] for i in range(0, len(string), charleng)]

def gitLastModified(*fnames):
    """return date string for the last commit modifing files given"""
    cmd = ['git', 'log', '-n1', '--pretty=format:%ci', '--']
    func = 'commonpath' if 'commonpath' in dir(os.path) else 'commonprefix'
    cpath = os.path.dirname(getattr(os.path, func)(fnames)) or None
    cmd.extend([os.path.relpath(f, cpath) if cpath else f for f in fnames])
    return subprocess.check_output(cmd, universal_newlines=True, cwd=cpath)

StrUpgradeQuery = '''
const std::vector<wxString> dbUpgradeQuery =
{
'''

LatestVersion = 0
sqlfiles = sorted(glob.glob(os.path.join(sys.argv[1], 'database_version_*.sql')), key=numericalSort)
if not sqlfiles:
    print(__doc__)
    sys.exit(1)

for sqlfile in sqlfiles:
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

StrHeader = u'''/** @file
 * @brief     Compilation of SQL scripts to upgrade MMEX db version
 * @warning   Auto generated with %s script. DO NOT EDIT!
 * @copyright © 2016-2018 Gabriele-V
 * @date      %s
 */

#ifndef DB_UPGRADE_H_
#define DB_UPGRADE_H_

#include <vector>
#include <wx/string.h>
'''% (os.path.basename(__file__), gitLastModified(__file__, *sqlfiles) or 'unknown')

with open('DB_Upgrade.h', 'w', encoding='utf-8') as fp:
    fp.write(StrHeader + StrLatestVersion + StrUpgradeQuery + strEnd)
