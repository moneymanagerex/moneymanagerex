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

#include "budget_table.h"

/************************************************************************************
 TBudgetTableEntry Methods
 ***********************************************************************************/
/// Constructor used when loading assets from the database
TBudgetEntry::TBudgetEntry(wxSQLite3ResultSet& q1)
: TEntryBase()
{
    id_             = q1.GetInt("BudgetEntryID");
    id_budget_year_ = q1.GetInt("BudgetYearID");
    id_category_    = q1.GetInt("CategID");
    id_subcategory_ = q1.GetInt("SubCategID");
    period_         = q1.GetString("Period");
    amount_         = q1.GetDouble("Amount");
}

/// Copy constructor using a pointer
TBudgetEntry::TBudgetEntry(TBudgetEntry* pEntry)
: TEntryBase()
{
    id_budget_year_ = pEntry->id_budget_year_;
    id_category_    = pEntry->id_category_;
    id_subcategory_ = pEntry->id_subcategory_;
    period_         = pEntry->period_;
    amount_         = pEntry->amount_;
}

void TBudgetEntry::SetDatabaseValues(wxSQLite3Statement& st, int& db_index)
{
    st.Bind(++db_index, id_budget_year_);
    st.Bind(++db_index, id_category_);
    st.Bind(++db_index, id_subcategory_);
    st.Bind(++db_index, period_);
    st.Bind(++db_index, amount_);
}

/// Constructor for creating a new asset entry.
TBudgetEntry::TBudgetEntry()
: TEntryBase()
, id_budget_year_(-1)
, id_category_(-1)
, id_subcategory_(-1)
, amount_(0.0)
{}

int TBudgetEntry::Add(wxSQLite3Database* db)
{
    try
    {
        const char SQL_STATEMENT[] =
        "INSERT INTO BUDGETTABLE_V1 "
        "(BudgetYearID, CategID, SubCategID, Period, Amount) "
        "values (?, ?, ?, ?, ?)";

        wxSQLite3Statement st = db->PrepareStatement(SQL_STATEMENT);
        int db_index = 0;
        SetDatabaseValues(st, db_index);
        FinaliseAdd(db, st);
    }
    catch(const wxSQLite3Exception& e)
    {
        wxLogError("TBudgetTableEntry:Add: %s", e.GetMessage().c_str());
    }

    return id_;
}

void TBudgetEntry::Delete(wxSQLite3Database* db)
{
    DeleteEntry(db, "delete from BUDGETTABLE_V1 where BUDGETENTRYID = ?");
}

void TBudgetEntry::Update(wxSQLite3Database* db)
{
    try
    {
        const char UPDATE_BUDGETTABLE_V1[] =
        "update BUDGETTABLE_V1 set"
        " BudgetYearID = ?, CategID = ?, SubCategID = ?, "
        " Period = ?, Amount = ? "
        "where BUDGETENTRYID = ?";

        wxSQLite3Statement st = db->PrepareStatement(UPDATE_BUDGETTABLE_V1);
        int db_index = 0;
        SetDatabaseValues(st, db_index);
        st.Bind(++db_index, id_);

        FinaliseStatement(st);
    }
    catch(const wxSQLite3Exception& e)
    {
        wxLogError("TBudgetTableEntry:Update: %s", e.GetMessage().c_str());
    }
}

/************************************************************************************
 TBudgetTableList Methods
 ***********************************************************************************/
/// Constructor
TBudgetList::TBudgetList(wxSQLite3Database* db, bool load_entries)
: TListBase(db)
{
    LoadEntries(load_entries);
}

TBudgetList::~TBudgetList()
{
    DestroyEntryList();
}

void TBudgetList::LoadEntries(bool load_entries)
{
    try
    {
        if (!ListDatabase()->TableExists("BUDGETTABLE_V1"))
        {
            const char CREATE_TABLE_BUDGETTABLE_V1[] =
            "CREATE TABLE BUDGETTABLE_V1(BUDGETENTRYID integer primary key, "
            "BUDGETYEARID integer, CATEGID integer, SUBCATEGID integer, "
            "PERIOD TEXT NOT NULL, AMOUNT numeric NOT NULL)";

            ListDatabase()->ExecuteUpdate(CREATE_TABLE_BUDGETTABLE_V1);
        }

        if (load_entries)
        {
            LoadEntriesUsing("select * from BUDGETTABLE_V1");
        }
    }
    catch (const wxSQLite3Exception& e)
    {
        wxLogError("TBudgetTableList::LoadEntries %s", e.GetMessage().c_str());
    }
}

void TBudgetList::LoadEntriesUsing(const wxString& sql_statement)
{
    DestroyEntryList();
    wxSQLite3ResultSet q1 = ListDatabase()->ExecuteQuery(sql_statement);
    while (q1.NextRow())
    {
        entrylist_.push_back(new TBudgetEntry(q1));
    }
    q1.Finalize();
}

void TBudgetList::DestroyEntryList()
{
    for (size_t i = 0; i < entrylist_.size(); ++i)
    {
        delete entrylist_[i];
    }
    entrylist_.clear();
}

int TBudgetList::AddEntry(TBudgetEntry* pBudgetEntry)
{
    pBudgetEntry->Add(ListDatabase());
    entrylist_.push_back(pBudgetEntry);

    return pBudgetEntry->id_;
}

void TBudgetList::DeleteEntry(int budget_entry_id)
{
    TBudgetEntry* pEntry = GetEntryPtr(budget_entry_id);
    if (pEntry)
    {
        pEntry->Delete(ListDatabase());
        entrylist_.erase(entrylist_.begin() + current_index_);
        delete pEntry;
    }
}

TBudgetEntry* TBudgetList::GetEntryPtr(int budget_entry_id)
{
    TBudgetEntry* pEntry = 0;
    size_t index = 0;

    while (index < entrylist_.size())
    {
        if (entrylist_[index]->id_ == budget_entry_id)
        {
            pEntry = entrylist_[index];
            current_index_ = index;
            break;
        }
        ++ index;
    }

    return pEntry;
}

TBudgetEntry* TBudgetList::GetIndexedEntryPtr(unsigned int list_index)
{
    TBudgetEntry* pEntry = 0;
    if (list_index < entrylist_.size())
    {
        pEntry = entrylist_[list_index];
    }

    return pEntry;
}

int TBudgetList::CurrentListSize()
{
    return entrylist_.size();
}
