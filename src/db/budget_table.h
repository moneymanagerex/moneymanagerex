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

const wxString BUDGET_TYPE_DEF[] =
{
    _("Expense"),
    _("Income")
};

const wxString BUDGET_PERIOD_DEF[] =
{
    wxTRANSLATE("None"),
    wxTRANSLATE("Weekly"),
    wxTRANSLATE("Bi-Weekly"),
    wxTRANSLATE("Monthly"),
    wxTRANSLATE("Bi-Monthly"),
    wxTRANSLATE("Quarterly"),
    wxTRANSLATE("Half-Yearly"),
    wxTRANSLATE("Yearly"),
    wxTRANSLATE("Daily")
};

/************************************************************************************
 Class TBudgetEntry
 ***********************************************************************************/
class TBudgetEntry : public TEntryBase
{
private:
    friend class TBudgetList;    // Allows TBudgetTableList access to private members

    int Add(wxSQLite3Database* db);
    void Delete(wxSQLite3Database* db);
    void SetDatabaseValues(wxSQLite3Statement& st, int& db_index);

public:
    enum BUDGET_TYPE
    {
        TYPE_EXPENSE,
        TYPE_INCOME
    };

    enum BUDGET_PERIOD
    {
        PERIOD_NONE,
        PERIOD_WEEKLY,
        PERIOD_BI_WEEKLY,
        PERIOD_MONTHLY,
        PERIOD_BI_MONTHLY,
        PERIOD_QUARTERLY,
        PERIOD_HALF_YEARLY,
        PERIOD_YEARLY,
        PERIOD_DAILY
    };

    int id_budget_year_;
    int id_category_;
    int id_subcategory_;
    wxString period_;
    double amount_;

    // Constructor used when loading assets from the database.
    TBudgetEntry(wxSQLite3ResultSet& q1);
    // Copy constructor using a pointer
    TBudgetEntry(TBudgetEntry* pEntry);
    // Constructor for creating a new asset entry.
    TBudgetEntry();

    void Update(wxSQLite3Database* db);
};

/************************************************************************************
 Class TBudgetTableList
 ***********************************************************************************/
class TBudgetList : public TListBase
{
private:
    void LoadEntries(bool load_entries = true);
    // delete all the objects in the list and clear the list.
    void DestroyEntryList();

public:
    std::vector<TBudgetEntry*> entrylist_;

    TBudgetList(wxSQLite3Database* db, bool load_entries = true);
    ~TBudgetList();

    // Allows specialised loads by providing the required SQL statement
    void LoadEntriesUsing(const wxString& sql_statement);

    int AddEntry(TBudgetEntry* budget_entry);
    void DeleteEntry(int budget_entry_id);

    TBudgetEntry* GetEntryPtr(int budget_entry_id);
    TBudgetEntry* GetIndexedEntryPtr(unsigned int list_index);

    int CurrentListSize();
};
