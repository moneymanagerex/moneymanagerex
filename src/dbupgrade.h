/*******************************************************
Copyright (C) 2016 Gabriele-V
Copyright (C) 2017 Stefano Giorgio [stef145g]

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
********************************************************/
#pragma once

#include "Table.h"
#include "DB_Upgrade.h"

class dbUpgrade
{
    static int FixVersionStatus(wxSQLite3Database* db, int version);
    static int GetCurrentVersion(wxSQLite3Database* db);
    static std::vector<wxString> SplitQueries(const wxString& statement);
    static bool UpgradeToVersion(wxSQLite3Database* db, int version);
    static void UpgradeFailedMessage(const wxString& error, const wxString& step, int version);
public:
    static bool InitializeVersion(wxSQLite3Database* db, int version = dbLatestVersion);
    static bool CheckUpgradeDB(wxSQLite3Database* db);
    static bool UpgradeDB(wxSQLite3Database* db, const wxString& DbFileName);
    static void BackupDB(const wxString& Filename, int BackupType, int FilesToKeep, int UpgradeVersion = 0);
    enum BACKUPTYPE { START = 0, CLOSE, VERSION_UPGRADE };
    static void SqlFileDebug(wxSQLite3Database* db);
};
