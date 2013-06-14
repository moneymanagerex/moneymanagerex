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
#include "transaction.h"

const wxString REPEAT_TYPE_TRANSLATE[] = {
    wxTRANSLATE("None"),
    wxTRANSLATE("Weekly"),
    wxTRANSLATE("Bi-Weekly"),
    wxTRANSLATE("Monthly"),
    wxTRANSLATE("Bi-Monthly"),
    wxTRANSLATE("Quarterly"),
    wxTRANSLATE("Half-Yearly"),
    wxTRANSLATE("Yearly"),
    wxTRANSLATE("Four Months"),
    wxTRANSLATE("Four Weeks"),
    wxTRANSLATE("Daily"),
    wxTRANSLATE("In %s Days"),
    wxTRANSLATE("In %s Months"),
    wxTRANSLATE("Every %s Days"),
    wxTRANSLATE("Every %s Months"),
    wxTRANSLATE("Monthly (last day)"),
    wxTRANSLATE("Monthly (last business day)")
};

/***********************************************************************************
 This class holds a single bill transaction entry
 **********************************************************************************/
class TTransactionBillEntry : public TTransactionEntry
{
private:
    friend class TTransactionBillList;

    // wxString nextOccurDate_;
    // date of next transaction occurance
    wxDateTime trans_repeat_date_;
    
    int Add(wxSQLite3Database* db);
    // remove the entry from the database.
    void Delete(wxSQLite3Database* db);
    void SetBillDatabaseValues(wxSQLite3Statement& st, int& db_index);
    int MultiplexedRepeatType();

public:
    enum REPEAT_TYPE {
        TYPE_INACTIVE = -1,
        TYPE_NONE,
        TYPE_WEEKLY,
        TYPE_BI_WEEKLY,      // FORTNIGHTLY
        TYPE_MONTHLY,
        TYPE_BI_MONTHLY,
        TYPE_QUARTERLY,      // TRI_MONTHLY
        TYPE_HALF_YEARLY,
        TYPE_YEARLY,
        TYPE_FOUR_MONTHLY,   // QUAD_MONTHLY
        TYPE_FOUR_WEEKLY,    // QUAD_WEEKLY
        TYPE_DAILY,
        TYPE_IN_X_DAYS,
        TYPE_IN_X_MONTHS,
        TYPE_EVERY_X_DAYS,
        TYPE_EVERY_X_MONTHS,
        TYPE_MONTHLY_LAST_DAY,
        TYPE_MONTHLY_LAST_BUSINESS_DAY,
        TYPE_REPEAT_TYPE_SIZE
    };

    bool autoExecuteManual_;
    bool autoExecuteSilent_;
    int repeat_type_;
    int num_repeats_;

    // Constructor for creating a new transaction entry
    TTransactionBillEntry();
    
    // Copy constructor using a pointer
    TTransactionBillEntry(TTransactionBillEntry* pEntry);

    // Constructor used to load a transaction from the database.
    TTransactionBillEntry(wxSQLite3ResultSet& q1);

    void Update(wxSQLite3Database* db);
    // Get the transaction entry from the bill transaction entry
    TTransactionEntry* GetTransaction();
    // Set the bill transaction entry from a transaction
    void SetTransaction(TTransactionEntry* pEntry);

    void AdjustNextOccuranceDate();
    bool RequiresExecution(int& remaining_days);
    void SetNextOccurDate(const wxDateTime date);
    // return the next occurance date
    wxDateTime NextOccurDate();
    
    // Display next occurance date according to required user format
    wxString DisplayNextOccurDate();
    // Returns true when using any type of repeating transaction.
    bool UsingRepeatProcessing();
    // Returns true when using In X Days/Months transaction.
    bool UsingIn_X_Processing();
    // Returns true when using Every X Days/Months transaction.
    bool UsingEvery_X_Processing();
};

/************************************************************************************
 Class TTransactionBillsList
 ***********************************************************************************/
class TTransactionBillList : public TTransactionList
{
private:
    void LoadEntries(bool load_entries = true);
    // delete all the objects in the list and clear the list.
    void DestroyEntryList();

public:
    std::vector<TTransactionBillEntry*> entrylist_;

    TTransactionBillList(wxSQLite3Database* db, bool load_entries = true);
    ~TTransactionBillList();

    /// Allows specialised loads by providing the required SQL statement
    void LoadEntriesUsing(const wxString& sql_statement);

    int AddEntry(TTransactionBillEntry* pTransBillsEntry);
    void DeleteEntry(int trans_bill_id);

    TTransactionBillEntry* GetEntryPtr(int trans_bill_id);
    TTransactionBillEntry* GetIndexedEntryPtr(unsigned int list_index);

    int CurrentListSize();
};
