/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel

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

#include "dbwrapper.h"
#include "util.h"
#include <wx/msgdlg.h>
#include <wx/wxsqlite3.h>
#include <sqlite3.h> // for SQLITE_OK etc.

wxSharedPtr<wxSQLite3Database> static_db_ptr()
{
    static wxSharedPtr<wxSQLite3Database> db(new wxSQLite3Database);

    return db;
}

wxSharedPtr<wxSQLite3Database> mmDBWrapper::Open(const wxString &dbpath, const wxString &password)
{
    wxSharedPtr<wxSQLite3Database> db = static_db_ptr();

    int err = SQLITE_OK;
    wxString errStr = wxEmptyString;
    try
    {
        db->Open(dbpath, password);
        // Ensure that an opened mmex database is decrypted
        db->TableExists("INFOTABLE");
    }
    catch (const wxSQLite3Exception& e)
    {
        err = e.GetErrorCode();
        errStr << e.GetMessage();
    }

    if (err==SQLITE_OK)
    {
        //timeout 2 sec
        db->SetBusyTimeout(2000);

        return (db);
    }

    wxString s = _("Error opening database file:");
    s << wxString::Format("\n%s\n\n", dbpath);
    if (err == SQLITE_CANTOPEN)
    {
        s << _("Can't open file") << "\n"
          << _("You must specify path to another database file");
    }
    else if (err == SQLITE_NOTADB)
    {
        // wrong file extension maybe? try emb<->mmb
        wxASSERT(db->IsOpen());
        db->Close();
        err = SQLITE_OK;
        errStr = wxEmptyString;
        try
        {
            db->Open(dbpath, password.IsEmpty() ? readPasswordFromUser(false) : wxGetEmptyString());
            db->TableExists("INFOTABLE");
        }
        catch (const wxSQLite3Exception& e)
        {
            err = e.GetErrorCode();
            errStr << e.GetMessage();
        }

        if (err==SQLITE_OK)
        {
            //timeout 2 sec
            db->SetBusyTimeout(2000);

            return (db);
        }
        s << _("Possible reasons:")
          << "\n- " << _("An incorrect password given for an encrypted file")
          << "\n- " << _("Attempt to open a file that is not a database file")
          << "\n- " << _("Corrupted database file");
    }
    else
    {
        s << errStr;
    }

    wxMessageDialog msgDlg(nullptr, s, _("Opening MMEX Database - Error"), wxOK | wxICON_ERROR);
    msgDlg.ShowModal();

    if (db->IsOpen()) db->Close();
    db.reset();
    return db; // return a NULL database pointer
}

//----------------------------------------------------------------------------
