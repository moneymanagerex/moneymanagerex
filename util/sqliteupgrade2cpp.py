#!/usr/bin/env python
# vi:tabstop=4:expandtab:shiftwidth=4:softtabstop=4:autoindent:smarttab
'''
Usage: python sqliteupgrade2cpp.py path_to_database_folder
'''

import datetime
import fnmatch
import os
import sys

def getVersion(FileName):
    FileName = FileName[17:]
    Version = FileName.partition(".")[0]
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
#ifndef MM_EX_DBUPGRADE_QUERY_H_
#define MM_EX_DBUPGRADE_QUERY_H_

#include <vector>
#include <wx/string.h>
'''

StrUpgradeQuery = '''
const std::vector<wxString> dbUpgradeQuery =
{
'''

LatestVersion = 0
folder = sys.argv[1]
for root, dirs, files in os.walk(folder):
        for name in sorted(files, key=getVersion):
            if fnmatch.fnmatch(name, 'database_version_*.sql'):
                FileContent = getFileContent(os.path.join(folder, name)).replace('\n','\n        ')
                LatestVersion = getVersion(name)
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
#endif // MM_EX_DBUPGRADE_QUERY_H_
'''

fp = open('DB_Upgrade.h', 'w')
fp.write(StrHeader + StrLatestVersion + StrUpgradeQuery + strEnd)
fp.close