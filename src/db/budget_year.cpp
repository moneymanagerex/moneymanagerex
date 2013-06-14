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

#include "budget_year.h"

/************************************************************************************
 TBudgetYearEntry Methods
 ***********************************************************************************/
/// Constructor used when loading assets from the database
TBudgetYearEntry::TBudgetYearEntry(wxSQLite3ResultSet& q1)
: TEntryBase()
{
    id_   = q1.GetInt("BudgetYearID");
    name_ = q1.GetString("BudgetYearName");
}

void TBudgetYearEntry::SetDatabaseValues(wxSQLite3Statement& st, int& db_index)
{
    st.Bind(++db_index, name_);
}

/// Constructor for creating a new budget year entry.
TBudgetYearEntry::TBudgetYearEntry()
: TEntryBase()
{}

int TBudgetYearEntry::Add(wxSQLite3Database* db)
{
    try
    {
        const char INSERT_INTO_BUDGETYEAR_V1[] =
        "insert into BUDGETYEAR_V1 "
        "(BudgetYearName) "
        "values (?)";

        wxSQLite3Statement st = db->PrepareStatement(INSERT_INTO_BUDGETYEAR_V1);
        int db_index = 0;
        SetDatabaseValues(st, db_index);
        FinaliseAdd(db, st);
    }
    catch(const wxSQLite3Exception& e)
    {
        wxLogError("TBudgetYearEntry:Add: %s", e.GetMessage().c_str());
    }

    return id_;
}

void TBudgetYearEntry::Delete(wxSQLite3Database* db)
{
    DeleteEntry(db, "delete from BUDGETYEAR_V1 where BudgetYearID = ?");
}

void TBudgetYearEntry::Update(wxSQLite3Database* db)
{
    try
    {
        const char SQL_STATEMENT[] =
        "update BUDGETYEAR_V1 set"
        " BudgetYearName = ?";

        wxSQLite3Statement st = db->PrepareStatement(SQL_STATEMENT);
        int db_index = 0;
        SetDatabaseValues(st, db_index);
        st.Bind(++db_index, id_);

        FinaliseStatement(st);
    }
    catch(const wxSQLite3Exception& e)
    {
        wxLogError("TBudgetYearEntry:Update: %s", e.GetMessage().c_str());
    }
}

/************************************************************************************
 TBudgetYearList Methods
 ***********************************************************************************/
// Constructor
TBudgetYearList::TBudgetYearList(wxSQLite3Database* db, bool load_entries)
: TListBase(db)
{
    LoadEntries(load_entries);
}

TBudgetYearList::~TBudgetYearList()
{
    DestroyEntryList();
}

void TBudgetYearList::LoadEntries(bool load_entries)
{
    try
    {
        if (!ListDatabase()->TableExists("BUDGETYEAR_V1"))
        {
            const char CREATE_TABLE_BUDGETYEAR_V1[] =
            "CREATE TABLE BUDGETYEAR_V1(BUDGETYEARID integer primary key, "
            "BUDGETYEARNAME TEXT NOT NULL UNIQUE)";

            ListDatabase()->ExecuteUpdate(CREATE_TABLE_BUDGETYEAR_V1);
        }

        if (load_entries)
        {
            LoadEntriesUsing("select * from BUDGETYEAR_V1");
        }
    }
    catch (const wxSQLite3Exception& e)
    {
        wxLogError("TBudgetYearList::LoadEntries %s", e.GetMessage().c_str());
    }
}

void TBudgetYearList::LoadEntriesUsing(const wxString& sql_statement)
{
    DestroyEntryList();
    wxSQLite3ResultSet q1 = ListDatabase()->ExecuteQuery(sql_statement);
    while (q1.NextRow())
    {
        entrylist_.push_back(new TBudgetYearEntry(q1));
    }
    q1.Finalize();
}

void TBudgetYearList::DestroyEntryList()
{
    for (size_t i = 0; i < entrylist_.size(); ++i)
    {
        delete entrylist_[i];
    }
    entrylist_.clear();
}

int TBudgetYearList::AddEntry(TBudgetYearEntry* pBudgetYearEntry)
{
    entrylist_.push_back(pBudgetYearEntry);
    pBudgetYearEntry->Add(ListDatabase());

    return pBudgetYearEntry->id_;
}

int TBudgetYearList::AddEntry(wxString budget_year_name)
{
    TBudgetYearEntry* pEntry = new TBudgetYearEntry();
    pEntry->name_ = budget_year_name;

    return AddEntry(pEntry);
}

void TBudgetYearList::DeleteEntry(int budget_year_id)
{
    TBudgetYearEntry* pEntry = GetEntryPtr(budget_year_id);
    if (pEntry)
    {
        pEntry->Delete(ListDatabase());
        entrylist_.erase(entrylist_.begin() + current_index_);
    }
}

TBudgetYearEntry* TBudgetYearList::GetEntryPtr(int budget_year_id)
{
    TBudgetYearEntry* pEntry = 0;
    size_t index = 0;

    while (index < entrylist_.size())
    {
        if (entrylist_[index]->id_ == budget_year_id)
        {
            pEntry = entrylist_[index];
            current_index_ = index;
            break;
        }
        ++ index;
    }

    return pEntry;
}

TBudgetYearEntry* TBudgetYearList::GetIndexedEntryPtr(unsigned int list_index)
{
    TBudgetYearEntry* pEntry = 0;
    if (list_index < entrylist_.size())
    {
        pEntry = entrylist_[list_index];
    }

    return pEntry;
}

int TBudgetYearList::CurrentListSize()
{
    return entrylist_.size();
}
