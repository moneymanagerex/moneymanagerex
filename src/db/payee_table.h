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
#include "entry_base.h"

/************************************************************************************
 Class TPayeeEntry
 ***********************************************************************************/
class TPayeeEntry : public TEntryBase
{
private:
    friend class TPayeeList;    // Allows the class, access to private members.

    int Add(wxSQLite3Database* db);
    void Delete(wxSQLite3Database* db);
    void SetDatabaseValues(wxSQLite3Statement& st, int& db_index);

public:
    wxString name_;
    int cat_id_;
    int subcat_id_;

    /// Constructor used when loading payees from the database
    TPayeeEntry(wxSQLite3ResultSet& q1);

    /// Constructor for creating a new payee entry
    TPayeeEntry(const wxString& name);

    void Update(wxSQLite3Database* db);
};

/************************************************************************************
 Class TPayeeList
 ***********************************************************************************/
class TPayeeList : public TListBase
{
private:
    void LoadEntries(bool load_entries = true);
    // delete all the objects in the list and clear the list.
    void DestroyEntryList();

public:
    std::vector<TPayeeEntry*> entrylist_;

    TPayeeList(wxSQLite3Database* db);
    ~TPayeeList();
    // Allows specialised loads by providing the required SQL statement
    void LoadEntriesUsing(const wxString& sql_statement);

    int AddEntry(const wxString& name, wxString category = wxEmptyString, wxString subcategory = wxEmptyString);
    void UpdateEntry(int payee_id, const wxString& new_payee_name, int cat_id = -1, int subcat_id = -1);
    void UpdateEntry(const wxString& payee_name, int cat_id, int subcat_id);

    /// Note: At this level, no checking is done for usage in other tables.
    void DeleteEntry(int payee_id);
    void DeleteEntry(const wxString& payee_name);

    TPayeeEntry* GetEntryPtr(const wxString& name);
    TPayeeEntry* GetEntryPtr(int payee_id);
    int GetPayeeId(const wxString& name);
    wxString GetPayeeName(int payee_id);
    bool PayeeExists(const wxString& name);
};

