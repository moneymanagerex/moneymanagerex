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

/***********************************************************************************
 SplitTransactions_V1 and BudgetSplitTransactions_V1 tables have the same structure.
 The classes TSplitTransactionList and TSplitTransEntry can be used for both tables.
 **********************************************************************************/
const wxString SPLIT_TRANS_TABLE = "SPLITTRANSACTIONS_V1";
const wxString BUDGET_SPLIT_TRANS_TABLE = "BUDGETSPLITTRANSACTIONS_V1";

/***********************************************************************************
 New class TSplitTransEntry
 This class holds a single split transaction entry
 **********************************************************************************/
class TSplitEntry : public TEntryBase
{
private:
    friend class TSplitTransactionList;
    DECLARE_NO_COPY_CLASS(TSplitEntry);

    int id_trans_;

    void Add(wxSQLite3Database* db, wxString db_table);
    void Delete(wxSQLite3Database* db, wxString db_table);

    /// Used when creating a new entry for TSplitTransactionList
    TSplitEntry();

public:
    int id_category_;
    int id_subcategory_;
    double amount_;

    /// Used to load an entry from the database. 
    TSplitEntry(wxSQLite3ResultSet& q1);
    void Update(wxSQLite3Database* db, wxString db_table);
};

/***********************************************************************************
 Load all split transaction entries into memory for increased performance.
 **********************************************************************************/
class TSplitEntriesList : public TListBase
{
private:
    friend class TSplitTransactionList;

    wxString db_table_;
    std::vector< TSplitEntry* > global_entries_;

    void LoadSplitEntries();

public:
    TSplitEntriesList(wxSQLite3Database* db, wxString db_table = SPLIT_TRANS_TABLE);
};

/***********************************************************************************
 This class holds a list of split transaction entries for a single transaction
 **********************************************************************************/
class TSplitTransactionList
{
private:
    int id_transaction_;
    TSplitEntriesList& entries_List_;
    double total_;
    std::vector< TSplitEntry* > entries_;

    /// Load the transaction split entries from the global list.
    void LoadEntries();
    
    void AddEntry(TSplitEntry* split_entry);
    void ReEvaluateTotal();
    void RemoveGlobalEntry(int entry_id);

public:
    // Creates and loads the list for a transaction.
    TSplitTransactionList(int id_transaction, TSplitEntriesList& entries_List);

    // Create an empty list for a new transaction
    TSplitTransactionList(TSplitEntriesList& entries_List);

    // Add local entries - trans ID still unknown.
    void AddLocalEntry(int cat_id, int subcat_id, double amount);

    // Saves the new list to the database and add to memory list.
    void SaveListforTransaction(int id_transaction);


    double TotalAmount();

    int AddEntry(int cat_id, int subcat_id, double amount);
    void UpdateEntry(TSplitEntry* split_entry);
    void DeleteEntry(TSplitEntry* split_entry);
    
    int GetListSize();
    TSplitEntry* GetEntryPtr(int id_split_trans);
    TSplitEntry* GetIndexedEntryPtr(int index);
};
