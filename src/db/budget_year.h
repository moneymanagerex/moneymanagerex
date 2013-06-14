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
 Class TBudgetYearEntry
 ***********************************************************************************/
class TBudgetYearEntry : public TEntryBase
{
private:
    friend class TBudgetYearList;    // Allows TAssetList access to private members
    
    int Add(wxSQLite3Database* db);
    void Delete(wxSQLite3Database* db);
    void SetDatabaseValues(wxSQLite3Statement& st, int& db_index);

public:
    wxString name_;

    /// Constructor used when loading assets from the database.
    TBudgetYearEntry(wxSQLite3ResultSet& q1);
    /// Constructor for creating a new asset entry.
    TBudgetYearEntry();

    void Update(wxSQLite3Database* db);
};

/************************************************************************************
 Class TBudgetYearList
 ***********************************************************************************/
class TBudgetYearList : public TListBase
{
private:
    void LoadEntries(bool load_entries = true);
    // delete all the objects in the list and clear the list.
    void DestroyEntryList();

public:
    std::vector<TBudgetYearEntry*> entrylist_;

    TBudgetYearList(wxSQLite3Database* db, bool load_entries = true);
    ~TBudgetYearList();

    /// Allows specialised loads by providing the required SQL statement
    void LoadEntriesUsing(const wxString& sql_statement);

    int AddEntry(TBudgetYearEntry* pBudgetYearEntry);
    int AddEntry(wxString budget_year_name);
    void DeleteEntry(int budget_year_id);
    
    TBudgetYearEntry* GetEntryPtr(int budget_year_id);
    TBudgetYearEntry* GetIndexedEntryPtr(unsigned int list_index);

    int CurrentListSize();
};
