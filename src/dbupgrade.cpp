/*******************************************************
Copyright (C) 2016 Gabriele-V

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
********************************************************/


#include "dbupgrade.h"

#include <wx/filename.h>
#include <wx/msgdlg.h>
#include <wx/tokenzr.h>

int dbUpgrade::GetCurrentVersion(wxSQLite3Database * db)
{
    try
    {
        wxSQLite3Statement stmt = db->PrepareStatement("PRAGMA user_version");
        wxSQLite3ResultSet rs = stmt.ExecuteQuery();
        return rs.GetInt(0);
    }
    catch (const wxSQLite3Exception& /*e*/)
    {
        return -1;
    }
}

std::vector<wxString> dbUpgrade::SplitQueries(const wxString & statement)
{
    std::vector<wxString> queries;
    wxStringTokenizer tokenizer(statement, ";");
    while (tokenizer.HasMoreTokens())
    {
        wxString token = tokenizer.GetNextToken();
        if (token.Length() > 10) // Remove empty queries
            queries.push_back(token);
    }
    return queries;
}

bool dbUpgrade::UpgradeToVersion(wxSQLite3Database * db, int version)
{
    wxString UpgradeQueries = dbUpgradeQuery[version];

    db->Savepoint("MMEX_Upgrade");
    for (const wxString& query : SplitQueries(UpgradeQueries))
    {
        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(query);
            stmt.ExecuteUpdate();
        }
        catch (const wxSQLite3Exception& e)
        {
            wxMessageBox(wxString::Format(_("MMEX database upgrade to version %i failed!"), version) + "\n\n"
                + _("Please restore DB from autocreated pre-upgrade backup and retry or contact MMEX support") + "\n\n"
                + e.GetMessage(), _("MMEX database upgrade"), wxOK | wxICON_ERROR);
            db->Rollback("MMEX_Upgrade");
            return false;
        }
    }
    if (!InitializeVersion(db, version))
        return false;
    
    db->ReleaseSavepoint("MMEX_Upgrade");
    return true;
}

bool dbUpgrade::InitializeVersion(wxSQLite3Database* db, int version)
{
    try
    {
        wxSQLite3Statement stmt = db->PrepareStatement(wxString::Format("PRAGMA user_version = %i", version));
        stmt.ExecuteUpdate();
        return true;
    }
    catch (const wxSQLite3Exception& /*e*/)
    {
        return false;
    }
}

bool dbUpgrade::CheckUpgradeDB(wxSQLite3Database * db)
{
    int ver = GetCurrentVersion(db);

    return (ver != dbLatestVersion) ? true : false;
}

bool dbUpgrade::UpgradeDB(wxSQLite3Database * db, const wxString& DbFileName)
{
    int ver = GetCurrentVersion(db);

    if (ver == -1 || ver > dbLatestVersion)
    {
        wxMessageBox(_("MMEX database error!") + "\n\n"
            + wxString::Format(_("MMEX database version %i doesn't work with this MMEX version.\nPlease upgrade MMEX to newer version."), ver)
            , _("MMEX database upgrade"), wxOK | wxICON_ERROR);
        return false;
    }

    for (ver; ver < dbLatestVersion; ver++)
    {
        BackupDB(DbFileName, dbUpgrade::BACKUPTYPE::VERSION_UPGRADE, 999, ver);
        if (!UpgradeToVersion(db, ver + 1))
            return false;
    }

    wxMessageBox(wxString::Format(_("MMEX database succesfully upgraded to version %i"), ver) + "\n\n"
        + _("We suggest a database optimization under Tools -> Database -> Optimize"), _("MMEX database upgrade"), wxOK | wxICON_INFORMATION);

    return true;
}

void dbUpgrade::BackupDB(const wxString& FileName, int BackupType, int FilesToKeep, int UpgradeVersion)
{
    wxFileName fn(FileName);
    if (!fn.IsOk()) return;

    wxString BackupName = "";

    // define string in backup filename
    switch (BackupType)
    {
    case BACKUPTYPE::START :
        BackupName = "_start_";
        break;
    case BACKUPTYPE::CLOSE :
        BackupName = "_update_";
        break;
    case BACKUPTYPE::VERSION_UPGRADE :
        BackupName = wxString::Format("_upgrade_v%i_", UpgradeVersion);
        break;
    default:
        break;
    }

    wxString backupFileName = FileName + BackupName + wxDateTime().Today().FormatISODate() + "." + fn.GetExt();
    wxFileName fnBak(backupFileName);

    // process backup
    switch (BackupType)
    {
    case BACKUPTYPE::START:
        if (!fnBak.FileExists())
        {
            wxCopyFile(FileName, backupFileName, true);
        }
        break;
    case BACKUPTYPE::CLOSE:
        wxCopyFile(FileName, backupFileName, true);
        break;
    case BACKUPTYPE::VERSION_UPGRADE:
        if (!fnBak.FileExists())
        {
            wxCopyFile(FileName, backupFileName, true);
        }
        break;
    default:
        break;
    }

    // Cleanup old backups
    if (BackupType != BACKUPTYPE::VERSION_UPGRADE)
    {
        wxArrayString backupFileArray;
        wxString fileSearch = FileName + BackupName + "*." + fn.GetExt();
        wxString backupFile = wxFindFirstFile(fileSearch);
        while (!backupFile.empty())
        {
            backupFileArray.Add(backupFile);
            backupFile = wxFindNextFile();
        }

        if (backupFileArray.Count() > (size_t)FilesToKeep)
        {
            backupFileArray.Sort(true);
            // ensure file is not read only before deleting file.
            wxFileName fnLastFile(backupFileArray.Last());
            if (fnLastFile.IsFileWritable()) wxRemoveFile(backupFileArray.Last());
        }
    }
}