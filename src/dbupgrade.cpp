/*******************************************************
Copyright (C) 2016 Gabriele-V
Copyright (C) 2017 Stefano Giorgio [stef145g]

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

#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>
#include <wx/textdlg.h>
#include <wx/textfile.h>
#include <wx/tokenzr.h>

int dbUpgrade::FixVersionStatus(wxSQLite3Database* db, int version)
{
    if (version == 7)
    {
        wxSQLite3Table account_table = db->GetTable("select * from ACCOUNTLIST_V1");
        // database version 7 has 20 columns
        // database version 6 has 13 columns
        if ((account_table.GetColumnCount() < 20) && (account_table.GetColumnCount() > 0))
        {
            version = 6;
        }
    }

    return version;
}

int dbUpgrade::GetCurrentVersion(wxSQLite3Database * db)
{
    try
    {
        int ver = FixVersionStatus(db, db->ExecuteScalar("PRAGMA user_version;"));
        return ver;
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
            db->ExecuteUpdate(query);
        }
        catch (const wxSQLite3Exception& e)
        {
            UpgradeFailedMessage(e.GetMessage(), _("upgrade"), version);
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
        db->ExecuteUpdate(wxString::Format("PRAGMA user_version = %i;", version));
        db->ExecuteUpdate("PRAGMA application_id = 0x4d4d4558;");
        return true;
    }
    catch (const wxSQLite3Exception& e)
    {
        UpgradeFailedMessage(e.GetMessage(), _("initialization"), version);
        db->Rollback("MMEX_Upgrade");
        return false;
    }
}

bool dbUpgrade::CheckUpgradeDB(wxSQLite3Database* db)
{
    int ver = GetCurrentVersion(db);

    return (ver != dbLatestVersion) ? true : false;
}

bool dbUpgrade::UpgradeDB(wxSQLite3Database* db, const wxString& DbFileName)
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

    try
    {
        db->Vacuum();
    }
    catch (const wxSQLite3Exception& e)
    {
        wxMessageBox(_("MMEX database vacuum failed!") + "\n\n"
            + _("MMEX still should work, but try to re-optimize it from Tools -> Database -> Optimize") + "\n\n"
            + e.GetMessage(), _("MMEX database upgrade"), wxOK | wxICON_WARNING);
    }

    wxMessageBox(wxString::Format(_("MMEX database successfully upgraded to version %i"), ver), _("MMEX database upgrade"), wxOK | wxICON_INFORMATION);

    return true;
}

void dbUpgrade::UpgradeFailedMessage(const wxString& error, const wxString& step, int version)
{
    wxMessageBox(wxString::Format(_("MMEX database %s to version %i failed!"), step, version) + "\n\n"
        + _("Please restore DB from autocreated pre-upgrade backup and retry or contact MMEX support") + "\n\n"
        + error, _("MMEX database upgrade"), wxOK | wxICON_ERROR);
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

void dbUpgrade::SqlFileDebug(wxSQLite3Database* db)
{
    wxFileDialog fileDlgLoad(nullptr,_("Load debug file"),"","",_("MMEX debug files (*.mmdbg)")+"|*.mmdbg", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (fileDlgLoad.ShowModal() != wxID_OK)
        return;

    wxTextFile txtFile;
    txtFile.Open(fileDlgLoad.GetPath());
    bool readonly;

    wxString txtLine = txtFile.GetFirstLine();
    if (txtLine == "-- MMEX Debug SQL - Read --") readonly=true;
    else if (txtLine == "-- MMEX Debug SQL - Update --") readonly=false;
    else
    {
        wxMessageBox(_("Invalid debug file content, please contact MMEX support!"), _("MMEX debug error"), wxOK | wxICON_ERROR);
        return;
    }

    if (!txtFile.Eof())
    {
        txtLine = txtFile.GetNextLine();
        if (txtLine.StartsWith("-- MMEX db version required ",&txtLine))
        {
            int ver = GetCurrentVersion(db);
            long reqver;
            if (!txtLine.ToCLong(&reqver))
            {
                wxMessageBox(_("Invalid debug file content, please contact MMEX support!"), _("MMEX debug error"), wxOK | wxICON_ERROR);
                return;
            }
            if (ver != reqver)
            {
                wxString msg = wxString::Format(_("This SQL debug script requires %li database version, but current database is version %i."), reqver, ver);
                wxMessageBox(msg, _("MMEX debug error"), wxOK | wxICON_ERROR);
                return;
            }
            if (!txtFile.Eof())
                txtLine = txtFile.GetNextLine();
        }
    } 

    wxString txtMsg;
    for (; !txtFile.Eof() && txtLine.StartsWith("-- ",&txtLine); txtLine = txtFile.GetNextLine())
    {
            txtMsg << txtLine << "\n";

    }
    if (!txtMsg.IsEmpty())
        if (wxMessageBox(_("Debug script description:") + "\n\n" + txtMsg + "\n" + _("Please confirm running this SQL script."), _("MMEX debug"), wxOK | wxCANCEL) != wxOK)
            return;

    if (txtFile.Eof()) return; // only comments in file

    if (readonly)
    {
        wxString txtLog = wxEmptyString;

        txtLog << wxString::Format("Current db file version: %i", dbUpgrade::GetCurrentVersion(db)) + wxTextFile::GetEOL();
        txtLog << mmex::getProgramDescription() + wxTextFile::GetEOL();

        for (; !txtFile.Eof(); txtLine = txtFile.GetNextLine())
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
                wxMessageBox(_("Invalid debug file content, please contact MMEX support!"), _("MMEX debug error"), wxOK | wxICON_ERROR);
                return;
            }
        }

        wxTextEntryDialog dlg(nullptr, _("Send this log to MMEX support team:\npress OK to save to file or Cancel to exit"),
            _("MMEX debug"), txtLog, wxOK | wxCANCEL | wxCENTRE | wxTE_MULTILINE);
        if (dlg.ShowModal() == wxID_OK)
        {
            wxFileDialog fileDlgSave(nullptr, _("Save debug file"), "", "", _("Text files (*.txt)")+"|*.txt", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
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
    else
    {
        db->Savepoint("MMEX_Debug");
        for (; !txtFile.Eof(); txtLine = txtFile.GetNextLine())
        {
            try
            {
                db->ExecuteUpdate(txtLine);
            }
            catch (const wxSQLite3Exception& e)
            {
                wxMessageBox(_("Query error, please contact MMEX support!") + "\n\n" + e.GetMessage(), _("MMEX debug error"), wxOK | wxICON_ERROR);
                db->Rollback("MMEX_Debug");
                return;
            }
        }
        db->ReleaseSavepoint("MMEX_Debug");
        wxMessageBox(_("DB maintenance completed, please close and re-open MMEX!"), _("MMEX debug"), wxOK | wxICON_INFORMATION);
    }
}
