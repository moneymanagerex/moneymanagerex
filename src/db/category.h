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
 Class TCategoryEntry
 ***********************************************************************************/
class TCategoryEntry : public TEntryBase
{
private:
    friend class TCategoryList;    // Allows access

    int Add(wxSQLite3Database* db);
    void Delete(wxSQLite3Database* db);

public:
    wxString name_;

    /// Constructor used when loading categories from the database
    TCategoryEntry(wxSQLite3ResultSet& q1);

    /// Constructor for creating a new category entry
    TCategoryEntry(const wxString& name);

    void Update(wxSQLite3Database* db);
};

/************************************************************************************
 Class TCategoryList
 ***********************************************************************************/
class TCategoryList : public TListBase
{
private:
    std::vector<TCategoryEntry*> entrylist_;

    void LoadEntries(bool load_entries = true);
    // delete all the objects in the list and clear the list.
    void DestroyEntryList();

public:
    TCategoryList(wxSQLite3Database* db);
    ~TCategoryList();

    // Allows specialised loads by providing the required SQL statement
    void LoadEntriesUsing(const wxString& sql_statement);

    /// Saves a new category name to the database and memory list
    int AddEntry(const wxString& name);
    void UpdateEntry(int cat_id, const wxString& new_category);

    /// Note: At this level, no checking is done for usage in other tables.
    void DeleteEntry(int cat_id);

    TCategoryEntry* GetEntryPtr(const wxString& name);
    TCategoryEntry* GetEntryPtr(int cat_id);
    int GetCategoryId(const wxString& name);
    wxString GetCategoryName(int cat_id);
    bool CategoryExists(const wxString& name);
};

