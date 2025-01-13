/*******************************************************
Copyright (C) 2016 Gabriele-V
Copyright (C) 2021 Nikolay Akimov

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
#include "constants.h"
#include "util.h"

#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>
#include <wx/textdlg.h>
#include <wx/textfile.h>
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
        wxString token = tokenizer.GetNextToken().Trim().Trim(false);
        if (token != "" && !(token.StartsWith("--") && !token.Contains("\n"))) // Remove queries with comments only
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

    for (; ver < dbLatestVersion; ver++)
    {
        BackupDB(DbFileName, dbUpgrade::BACKUPTYPE::VERSION_UPGRADE, 999, ver);
        if (!UpgradeToVersion(db, ver + 1))
            return false;
    }

    wxMessageBox(wxString::Format(_("MMEX database succesfully upgraded to version %i"), ver) + "\n\n"
        + _u("It is recommended to optimize the database with Tools → Database → Optimize.")
            , _("MMEX database upgrade"), wxOK | wxICON_INFORMATION);

    return true;
}

void dbUpgrade::BackupDB(const wxString& FileName, int BackupType, int FilesToKeep, int UpgradeVersion)
{
    wxFileName fn(FileName);
    if (!fn.IsOk()) return;

    const wxString BackupName[3] = { "_start_", "_update_", wxString::Format("_upgrade_v%i_", UpgradeVersion) };
    const auto backupFileName = wxString::Format("%s%s%s.bak", FileName, BackupName[BackupType], wxDateTime().Today().FormatISODate());
    wxFileName fnBak(backupFileName);

    // process backup
    switch (BackupType)
    {
    case BACKUPTYPE::START:
        if (!fnBak.FileExists()) {
            wxCopyFile(FileName, backupFileName, true);
        }
        break;
    case BACKUPTYPE::CLOSE:
        wxCopyFile(FileName, backupFileName, true);
        break;
    case BACKUPTYPE::VERSION_UPGRADE:
        if (!fnBak.FileExists()) {
            wxCopyFile(FileName, backupFileName, true);
        }
        break;
    default:
        break;
    }

    // Cleanup old backups
    if (BackupType != BACKUPTYPE::VERSION_UPGRADE)
    {
        wxSortedArrayString backupFileArray;
        const auto fileSearch = wxString::Format(R"(%s%s????-??-??.bak)", FileName, BackupName[BackupType]);
        wxString backupFile = wxFindFirstFile(fileSearch);
        while (!backupFile.empty())
        {
            backupFileArray.Add(backupFile);
            backupFile = wxFindNextFile();
        }

        while (backupFileArray.GetCount() > static_cast<size_t>(FilesToKeep))
        {
            wxFileName fnLastFile(backupFileArray.Item(0));
            wxLogDebug("%s", backupFileArray.Item(0));
            // ensure file is not read only before deleting file.
            if (fnLastFile.IsFileWritable())
                wxRemoveFile(backupFileArray.Item(0));

            backupFileArray.erase(backupFileArray.begin());
        }
    }
}

void dbUpgrade::SqlFileDebug(wxSQLite3Database * db)
{
    wxFileDialog fileDlgLoad(nullptr,_("Load debug file"),"","","MMDBG Files(*.mmdbg)|*.mmdbg", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (fileDlgLoad.ShowModal() != wxID_OK)
        return;

    wxString filePath = fileDlgLoad.GetPath();
    if (wxFileName(filePath).GetExt().MakeLower() != "mmdbg")
    {
        wxMessageBox(_("Wrong file type!"), _("MMEX debug error"), wxOK | wxICON_ERROR);
        return;
    }

    wxTextFile txtFile;
    txtFile.Open(filePath);

    if (txtFile.GetFirstLine().Contains("-- MMEX Debug SQL - Read --"))
    {
        wxString txtLine, txtLog = "";

        txtLog << wxString::Format("MMEX Version: %s", mmex::version::string) + wxTextFile::GetEOL();
        txtLog << wxString::Format("DB Version: %i", dbUpgrade::GetCurrentVersion(db)) + wxTextFile::GetEOL();
        txtLog << wxString::Format("Operating System: %s", wxGetOsDescription()) + wxTextFile::GetEOL() + wxTextFile::GetEOL();

        for (txtLine = txtFile.GetNextLine(); !txtFile.Eof(); txtLine = txtFile.GetNextLine())
        {
            txtLog << wxTextFile::GetEOL() << "=== Query ===" << wxTextFile::GetEOL() << txtLine << wxTextFile::GetEOL();

            wxSQLite3Statement stmt = db->PrepareStatement(txtLine);
            if (stmt.IsReadOnly())
            {
                try
                {
                    wxSQLite3ResultSet rs = stmt.ExecuteQuery();
                    int columnCount = rs.GetColumnCount();
                    txtLog << "=== Result ===" << wxTextFile::GetEOL();
                    while (rs.NextRow())
                    {
                        wxString strRow = "";
                        for (int i = 0; i < columnCount; ++i)
                            strRow << "'" + rs.GetAsString(i) + "'   ";
                        txtLog << strRow + wxTextFile::GetEOL();
                    }
                }
                catch (const wxSQLite3Exception& e)
                {
                    wxMessageBox(_("Query error, please contact MMEX support!") + "\n\n" + e.GetMessage(), _("MMEX debug error"), wxOK | wxICON_ERROR);
                    return;
                }
                
            }
            else
            {
                wxMessageBox(_("Invalid file content, please contact MMEX support!"), _("MMEX debug error"), wxOK | wxICON_ERROR);
                return;
            }
        }

        txtLog << wxTextFile::GetEOL() << wxTextFile::GetEOL() << wxTextFile::GetEOL() << getProgramDescription();

        wxTextEntryDialog dlg(nullptr, _("Send this log to MMEX support team:\npress OK to save to file or Cancel to exit"),
            _("MMEX debug"), txtLog, wxOK | wxCANCEL | wxCENTRE | wxTE_MULTILINE);
        if (dlg.ShowModal() == wxID_OK)
        {
            wxFileDialog fileDlgSave(nullptr, _("Save debug file"), "", "", "*.txt", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
            if (fileDlgSave.ShowModal() == wxID_OK)
            {
                wxFile file(fileDlgSave.GetPath(), wxFile::write);
                if (file.IsOpened())
                {
                    file.Write(txtLog);
                    file.Close();
                }
            }
        }
    }
    else if (txtFile.GetFirstLine().Contains("-- MMEX Debug SQL - Update --"))
    {
        db->Savepoint("MMEX_Debug");
        wxString txtLine, sql;
        for (txtLine = txtFile.GetNextLine(); !txtFile.Eof(); txtLine = txtFile.GetNextLine())
        {
            txtLine.Trim();
            if (!txtLine.empty() && !txtLine.StartsWith("--"))
                sql += sql.empty() ? txtLine : "\n" + txtLine;

            if (sql.EndsWith(";"))
            {
                wxLogDebug("%s", sql);
                try
                {
                    wxSQLite3Statement stmt = db->PrepareStatement(sql);
                    sql = "";
                    stmt.ExecuteUpdate();
                }
                catch (const wxSQLite3Exception& e)
                {
                    wxMessageBox(_("Query error, please contact MMEX support!") + "\n\n" + e.GetMessage(), _("MMEX debug error"), wxOK | wxICON_ERROR);
                    db->Rollback("MMEX_Debug");
                    return;
                }
            }
        }
        db->ReleaseSavepoint("MMEX_Debug");
        wxMessageBox(_("DB maintenance completed, please close and re-open MMEX!"), _("MMEX debug"), wxOK | wxICON_INFORMATION);
    }
    else
    {
        wxMessageBox(_("Invalid file content, please contact MMEX support!"), _("MMEX debug error"), wxOK | wxICON_ERROR);
        return;
    }
}
