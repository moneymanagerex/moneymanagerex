/*******************************************************
 Copyright (C) 2013 Stefano Giorgio

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
#include "memory"
#include <vector>
#include "../defs.h"
#include "../util.h"

#include "cajun/json/elements.h"

/************************************************************************************
 Class TEntryBase
 This is the base class for all entries of the database.
 ***********************************************************************************/
class TEntryBase
{
protected:
    int id_;

    /// This will set the record id and finalise the sql statement.
    void FinaliseAdd(wxSQLite3Database* db, wxSQLite3Statement& st);

    /// This will finalise the sql statement.
    void FinaliseStatement(wxSQLite3Statement& st);
    // Delete the entry from the database
    void DeleteEntry(wxSQLite3Database* db, const wxString& sql_statement);

public:
    virtual bool operator < (const TEntryBase& entry) const;

public:
    TEntryBase();

    int GetId() const;
};

/************************************************************************************
 Class TListBase
 This is the base class for main Lists of the database.
 ***********************************************************************************/
class TListBase
{
private:
    wxSQLite3Database* db_;

protected:
    int current_index_;

public:
    TListBase(wxSQLite3Database* db);
    wxSQLite3Database* ListDatabase();
    int GetCurrentIndex();
};
