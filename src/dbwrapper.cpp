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
#include "paths.h"
#include "constants.h"
//----------------------------------------------------------------------------
#include "sqlite3mc_amalgamation.h"
//----------------------------------------------------------------------------
/*
    SQLITE_OPEN_READWRITE
    The database is opened for reading and writing if possible, or reading
    only if the file is write protected by the operating system.  In either
    case the database must already exist, otherwise an error is returned.
*/
wxSharedPtr<wxSQLite3Database> static_db_ptr()
{
    static wxSharedPtr<wxSQLite3Database> db(new wxSQLite3Database);

    return db;
}

wxSharedPtr<wxSQLite3Database> mmDBWrapper::Open(const wxString &dbpath, const wxString &password, const bool debug)
{
    wxSharedPtr<wxSQLite3Database> db = static_db_ptr();

    int err = SQLITE_OK;
    wxString errStr=wxEmptyString;
    wxSQLite3CipherSQLCipher cipher;
    cipher.InitializeVersionDefault(4);
    cipher.SetLegacy(true);

     // open and disable flag SQLITE_CorruptRdOnly = 0x200000000
    const int flags = debug ? (WXSQLITE_OPEN_READWRITE | WXSQLITE_OPEN_CREATE) & ~0x200000000 : WXSQLITE_OPEN_READWRITE | WXSQLITE_OPEN_CREATE;

    try
    {
        db->Open(dbpath, cipher, password, flags);

        // Ensure that an existing mmex database is not encrypted.
        if ((db->IsOpen()) && (db->TableExists("INFOTABLE_V1")))
        {
            db->ExecuteQuery("select * from INFOTABLE_V1;");
        }
    }
    catch (const wxSQLite3Exception&)
    {
        
        // Check if database is encrypted with legacy AES128 cipher used prior to 1.8.1
        wxSQLite3CipherAes128 cipherAes128;
        cipherAes128.InitializeFromGlobalDefault();
        try
        {
            db->Open(dbpath, cipherAes128, password, flags);

            // Ensure that an existing mmex database is not encrypted.
            if ((db->IsOpen()) && (db->TableExists("INFOTABLE_V1")))
            {
                db->ExecuteQuery("select * from INFOTABLE_V1;");
            }

            wxMessageDialog msgDlg(nullptr, _("The default cipher algorithm has changed from AES-128 to AES-256 for compatibility with the MMEX mobile apps.")
                + "\n\n" + _("Rekeying with the new cipher will prevent opening this database in older versions of MMEX.")
                + "\n\n" + _("Do you want to update the database?"), _("Opening MMEX Database – Warning"), wxYES_NO | wxICON_WARNING);
            if (msgDlg.ShowModal() == wxID_YES)
            {
                if (db->ExecuteQuery("PRAGMA page_size;").GetInt(0) < 4096)
                {
                    db->ReKey("");
                    db->ExecuteUpdate("PRAGMA page_size = 4096;");
                    db->ExecuteUpdate("VACUUM;");
                }
                // ReKey with new cipher.
                db->ReKey(cipher, password);
            }
        }
        catch (const wxSQLite3Exception& e)
        {
            err = e.GetErrorCode();
            errStr << e.GetMessage();
        }
    }

    if (err==SQLITE_OK)
    {
        //timeout 2 sec
        db->SetBusyTimeout(2000);

        return (db);
    }
    db->Close();
    db.reset();

    wxString s = _("When database file opening:");
    s << "\n" << wxString::Format("\n%s\n\n", dbpath);
    if (err == SQLITE_CANTOPEN)
    {
        s << _("Unable to open file") << "\n" << _("You must specify path to another database file") << "\n";
    }
    else if (err == SQLITE_NOTADB)
    {
        s << _("An incorrect password given for an encrypted file\nor\nattempt to open a File that is not a database file \n");
    }
    else
    {
        s << wxString::Format(_("Error: %s"), wxString() << err << "\n" << errStr << "\n");
    }

    wxMessageDialog msgDlg(nullptr, s, _("Opening MMEX Database – Error"), wxOK | wxICON_ERROR);
    msgDlg.ShowModal();

    return db; // return a nullptr database pointer
}

//----------------------------------------------------------------------------

