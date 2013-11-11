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
#include "sqlite3.h"
//----------------------------------------------------------------------------
/*
    SQLITE_OPEN_READWRITE
    The database is opened for reading and writing if possible, or reading
    only if the file is write protected by the operating system.  In either
    case the database must already exist, otherwise an error is returned.
*/
wxSharedPtr<wxSQLite3Database> mmDBWrapper::Open(const wxString &dbpath, const wxString &password)
{
    wxSharedPtr<wxSQLite3Database> db = static_db_ptr();

    int err = SQLITE_OK;
    wxString errStr=wxEmptyString;
    try
    {
        db->Open(dbpath, password);
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
    db->Close();
    db.reset();

    wxString s = _("When database file opening:");
    s << "\n" << wxString::Format("\n%s\n\n", dbpath);
    if (err == SQLITE_CANTOPEN)
    {
        s << _("Can't open file") <<"\n" << _("You must specify path to another database file") << "\n";
    }
    else if (err == SQLITE_NOTADB)
    {
        s << _("An incorrect password given for an encrypted file")
        << "\n\n" << _("or") << "\n\n"
        << _("Attempt to open a File that is not a database file") << "\n";
    }
    else
    {
        s << "\n" << _("Error") << err << "\n";
    }

    wxSafeShowMessage(_("Database::open: %s"), s);

    s << errStr << "\n\n" << _("Continue ?");

    wxMessageDialog msgDlg(NULL, s, _("Error"), wxYES_NO|wxICON_ERROR);
    if (msgDlg.ShowModal() == wxID_NO)
    {
        exit(err);
    }
    return db; // return a NULL database pointer
}

//----------------------------------------------------------------------------

