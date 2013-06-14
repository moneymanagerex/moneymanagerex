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

#include "transactionbill.h"

const int REPEAT_TYPE_MULTIPLEX_BASE = 100;

/************************************************************************************
 TTransactionBillEntry Methods
 ***********************************************************************************/

/// Constructor for creating a new transaction entry
TTransactionBillEntry::TTransactionBillEntry()
: TTransactionEntry()
, autoExecuteManual_(false)
, autoExecuteSilent_(false)
, repeat_type_(TYPE_NONE)
, num_repeats_(0)
{}

/// Copy constructor using a pointer
TTransactionBillEntry::TTransactionBillEntry(TTransactionBillEntry* pEntry)
: TTransactionEntry(pEntry)
{
    autoExecuteManual_ = pEntry->autoExecuteManual_;
    autoExecuteSilent_ = pEntry->autoExecuteSilent_;
    repeat_type_       = pEntry->repeat_type_;
    trans_repeat_date_ = pEntry->trans_repeat_date_;
    num_repeats_       = pEntry->num_repeats_;

    //nextOccurDate_ = trans_repeat_date_.FormatISOCombined(' ');
}

/// Constructor used to load a transaction from the database.
TTransactionBillEntry::TTransactionBillEntry(wxSQLite3ResultSet& q1)
: TTransactionEntry()
, autoExecuteManual_(false)
, autoExecuteSilent_(false)
{
    id_ = q1.GetInt("BDID");
    GetDatabaseValues(q1);
    repeat_type_       = q1.GetInt("REPEATS");
    trans_repeat_date_ = q1.GetDate("NEXTOCCURRENCEDATE");
    num_repeats_       = q1.GetInt("NUMOCCURRENCES");

    // DeMultiplex the Auto Executable fields from the db entry: REPEATS
    if (repeat_type_ >= REPEAT_TYPE_MULTIPLEX_BASE)    // Auto Execute - User Acknowlegement
    {
        repeat_type_ -= REPEAT_TYPE_MULTIPLEX_BASE;
        autoExecuteManual_ = true;
    }

    if (repeat_type_ >= REPEAT_TYPE_MULTIPLEX_BASE)    // Auto Execute - Silent mode
    {
        repeat_type_ -= REPEAT_TYPE_MULTIPLEX_BASE;
        autoExecuteManual_ = false;               // Can only be manual or auto. Not both
        autoExecuteSilent_ = true;
    }

//    nextOccurDate_ = trans_repeat_date_.FormatISOCombined(' ');
}

int TTransactionBillEntry::MultiplexedRepeatType()
{
    // Multiplex Auto executable onto the repeat field of the database.
    int db_repeats = repeat_type_;
    if (autoExecuteManual_) db_repeats += REPEAT_TYPE_MULTIPLEX_BASE;
    if (autoExecuteSilent_) db_repeats += REPEAT_TYPE_MULTIPLEX_BASE;
    return db_repeats;
}

int TTransactionBillEntry::Add(wxSQLite3Database* db)
{
    try
    {
        const char SQL_STATEMENT[] =
        "insert into BILLSDEPOSITS_V1"
        " (ACCOUNTID, TOACCOUNTID, PAYEEID, TRANSCODE, TRANSAMOUNT,"
        " STATUS, TRANSACTIONNUMBER, NOTES, CATEGID, SUBCATEGID,"
        " TRANSDATE, FOLLOWUPID, TOTRANSAMOUNT,"
        " REPEATS, NEXTOCCURRENCEDATE, NUMOCCURRENCES)"
        " values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

        wxSQLite3Statement st = db->PrepareStatement(SQL_STATEMENT);
        int db_index = 0;
        SetDatabaseValues(st, db_index);        // call to inherited Database Values
        SetBillDatabaseValues(st, db_index);

        FinaliseAdd(db, st);
    }
    catch(const wxSQLite3Exception& e)
    {
        wxLogError("TTransactionBillEntry:Add: %s", e.GetMessage().c_str());
    }
    return id_;
}

void TTransactionBillEntry::Delete(wxSQLite3Database* db)
{
    DeleteEntry(db, "delete from BILLSDEPOSITS_V1 where BDID = ?");
}

void TTransactionBillEntry::SetBillDatabaseValues(wxSQLite3Statement& st, int& db_index)
{
    st.Bind(++db_index, MultiplexedRepeatType());
    st.BindDate(++db_index, trans_repeat_date_);
    st.Bind(++db_index, num_repeats_);
}

void TTransactionBillEntry::Update(wxSQLite3Database* db)
{
    try
    {
        const char SQL_STATEMENT[] =
        "update BILLSDEPOSITS_V1 set"
        " ACCOUNTID = ?, TOACCOUNTID = ?, PAYEEID = ?, TRANSCODE = ?, TRANSAMOUNT = ?,"
        " STATUS = ?, TRANSACTIONNUMBER = ?, NOTES = ?, CATEGID = ?, SUBCATEGID = ?,"
        " TRANSDATE = ?, FOLLOWUPID = ?, TOTRANSAMOUNT = ?,"
        " REPEATS = ?, NEXTOCCURRENCEDATE = ?, NUMOCCURRENCES = ? "
        "where BDID = ?";
        wxSQLite3Statement st = db->PrepareStatement(SQL_STATEMENT);
        int db_index = 0;
        SetDatabaseValues(st, db_index);    // call to inherited SetDatabaseValues()
        SetBillDatabaseValues(st, db_index);
        st.Bind(++db_index, id_);

        FinaliseStatement(st);
    }
    catch(const wxSQLite3Exception& e)
    {
        wxLogError("TTransactionBillEntry:update: %s", e.GetMessage().c_str());
    }
}

TTransactionEntry* TTransactionBillEntry::GetTransaction()
{
    TTransactionEntry* pEntry(new TTransactionEntry());
    
    pEntry->id_from_account_ = id_from_account_;
    pEntry->id_to_account_   = id_to_account_;
    pEntry->id_payee_        = id_payee_;
    pEntry->id_category_     = id_category_;
    pEntry->id_subcategory_  = id_subcategory_;
    pEntry->id_followup_     = id_followup_;
    
    pEntry->amount_from_     = amount_from_;
    pEntry->amount_to_       = amount_to_;

    pEntry->trans_date_      = trans_date_;
    pEntry->trans_num_       = trans_num_;
    pEntry->trans_type_      = trans_type_;   // transcode in database
    pEntry->trans_status_    = trans_status_;
    pEntry->trans_notes_     = trans_notes_;

    return pEntry;
}

void TTransactionBillEntry::SetTransaction(TTransactionEntry* pEntry)
{
    id_from_account_ = pEntry->id_from_account_;
    id_to_account_   = pEntry->id_to_account_;
    id_payee_        = pEntry->id_payee_;
    id_category_     = pEntry->id_category_;
    id_subcategory_  = pEntry->id_subcategory_;
    id_followup_     = pEntry->id_followup_;
    
    amount_from_     = pEntry->amount_from_;
    amount_to_       = pEntry->amount_to_;

    trans_date_      = pEntry->trans_date_;
    trans_num_       = pEntry->trans_num_;
    trans_type_      = pEntry->trans_type_;   // transcode in database
    trans_status_    = pEntry->trans_status_;
    trans_notes_     = pEntry->trans_notes_;
}

/*
 num_repeats represents the number of times the bill transaction is to occur,
 and is decremented by 1 each time except for IN_X_DAYS .. EVERY_X_MONTHS
 where it represents a time span, and becomes inactive for IN_X_periods only.
*/
void TTransactionBillEntry::AdjustNextOccuranceDate()
{
    if (repeat_type_ == TYPE_NONE)
    {
        num_repeats_ = 0;
    }
    else if (repeat_type_ == TYPE_WEEKLY)
    {
        trans_repeat_date_.Add(wxTimeSpan::Week());
    }
    else if (repeat_type_ == TYPE_BI_WEEKLY)
    {
        trans_repeat_date_.Add(wxTimeSpan::Weeks(2));
    }
    else if (repeat_type_ == TYPE_MONTHLY)
    {
        trans_repeat_date_.Add(wxDateSpan::Month());
    }
    else if (repeat_type_ == TYPE_BI_MONTHLY)
    {
        trans_repeat_date_.Add(wxDateSpan::Months(2));
    }
    else if (repeat_type_ == TYPE_QUARTERLY)
    {
        trans_repeat_date_.Add(wxDateSpan::Months(3));
    }
    else if (repeat_type_ == TYPE_HALF_YEARLY)
    {
        trans_repeat_date_.Add(wxDateSpan::Months(6));
    }
    else if (repeat_type_ == TYPE_YEARLY)
    {
        trans_repeat_date_.Add(wxDateSpan::Year());
    }
    else if (repeat_type_ == TYPE_FOUR_MONTHLY)
    {
        trans_repeat_date_.Add(wxDateSpan::Months(4));
    }
    else if (repeat_type_ == TYPE_FOUR_WEEKLY)
    {
        trans_repeat_date_.Add(wxDateSpan::Weeks(4));
    }
    else if (repeat_type_ == TYPE_DAILY)
    {
        trans_repeat_date_.Add(wxDateSpan::Day());
    }
    else if ((repeat_type_ == TYPE_IN_X_DAYS) || (repeat_type_ == TYPE_IN_X_MONTHS))
    {
        num_repeats_ = TYPE_INACTIVE;
    }
    else if (repeat_type_ == TYPE_EVERY_X_DAYS)
    {
        if (num_repeats_ > 0)
        {
            trans_repeat_date_.Add(wxDateSpan::Days(num_repeats_));
        }
    }
    else if (repeat_type_ == TYPE_EVERY_X_MONTHS)
    {
        if (num_repeats_ > 0)
        {
            trans_repeat_date_.Add(wxDateSpan::Months(num_repeats_));
        }
    }
    else if ((repeat_type_ == TYPE_MONTHLY_LAST_DAY) || (repeat_type_ == TYPE_MONTHLY_LAST_BUSINESS_DAY))
    {
        trans_repeat_date_.Add(wxDateSpan::Month());
        trans_repeat_date_.SetToLastMonthDay(trans_repeat_date_.GetMonth(), trans_repeat_date_.GetYear());
        if (repeat_type_ == TYPE_MONTHLY_LAST_BUSINESS_DAY) // last weekday of month
        {
            if ((trans_repeat_date_.GetWeekDay() == wxDateTime::Sun) ||
                (trans_repeat_date_.GetWeekDay() == wxDateTime::Sat))
            {
                trans_repeat_date_.SetToPrevWeekDay(wxDateTime::Fri);
            }
        }
    }

    if (num_repeats_ > 0)
    {
        if ((repeat_type_ < TYPE_IN_X_DAYS) || (repeat_type_ > TYPE_EVERY_X_MONTHS))
        {
            --num_repeats_;
        }
    }

    //nextOccurDate_ = trans_repeat_date_.FormatISOCombined(' ');
}

bool TTransactionBillEntry::RequiresExecution(int& remaining_days)
{
    wxTimeSpan ts = trans_repeat_date_.Subtract(wxDateTime::Now());
    
    remaining_days = ts.GetDays();
    int remaining_minutes = ts.GetMinutes();

    bool execution_required = false;
    if (remaining_minutes > 0)
    {
        remaining_days += 1;
    }
    if (remaining_days < 1)
    {
        execution_required = true;
    }

    return execution_required;
}

bool TTransactionBillEntry::UsingIn_X_Processing()
{
    bool result = false;
    if ((repeat_type_ == TYPE_IN_X_DAYS) || (repeat_type_ == TYPE_IN_X_MONTHS))
    {
        result = true;
    }
    return result;
}

bool TTransactionBillEntry::UsingEvery_X_Processing()
{
    bool result = false;
    if ((repeat_type_ == TYPE_EVERY_X_DAYS) || (repeat_type_ == TYPE_EVERY_X_MONTHS))
    {
        result = true;
    }
    return result;
}

bool TTransactionBillEntry::UsingRepeatProcessing()
{
    bool result = false;
    if ((repeat_type_ >= TYPE_IN_X_DAYS) || (repeat_type_ <= TYPE_EVERY_X_MONTHS))
    {
        if (num_repeats_ > 0)
        {
            result = true;
        }
    }
    return result;
}

void TTransactionBillEntry::SetNextOccurDate(wxDateTime date)
{
    trans_repeat_date_ = date;
    //nextOccurDate_ = trans_repeat_date_.FormatISOCombined(' ');
}

wxDateTime TTransactionBillEntry::NextOccurDate()
{
    return trans_repeat_date_;
}

wxString TTransactionBillEntry::DisplayNextOccurDate()
{
    return mmGetDateForDisplay(trans_repeat_date_);
}

/************************************************************************************
 TTransactionBillList Methods
 ***********************************************************************************/
/// Constructor
TTransactionBillList::TTransactionBillList(wxSQLite3Database* db, bool load_entries)
: TTransactionList(db, false)
{
    LoadEntries(load_entries);
}

TTransactionBillList::~TTransactionBillList()
{
    DestroyEntryList();
}

void TTransactionBillList::LoadEntries(bool load_entries)
{
    try
    {
        if (!ListDatabase()->TableExists("BILLSDEPOSITS_V1"))
        {
            const char CREATE_TABLE_BILLSDEPOSITS_V1[] =
            "CREATE TABLE BILLSDEPOSITS_V1 (BDID INTEGER PRIMARY KEY, "
            "ACCOUNTID INTEGER NOT NULL, TOACCOUNTID INTEGER, PAYEEID integer NOT NULL, TRANSCODE TEXT NOT NULL, "
            "TRANSAMOUNT numeric NOT NULL, STATUS TEXT, TRANSACTIONNUMBER TEXT, NOTES TEXT, "
            "CATEGID integer, SUBCATEGID integer, TRANSDATE TEXT, FOLLOWUPID integer, TOTRANSAMOUNT numeric, "
            "REPEATS numeric, NEXTOCCURRENCEDATE TEXT, NUMOCCURRENCES numeric)";

            ListDatabase()->ExecuteUpdate(CREATE_TABLE_BILLSDEPOSITS_V1);
        }

        if (load_entries)
        {
            LoadEntriesUsing("select * from BILLSDEPOSITS_V1");
        }
    }
    catch (const wxSQLite3Exception& e)
    {
        wxLogError("TTransactionBillList:LoadEntries %s", e.GetMessage().c_str());
    }
}

void TTransactionBillList::LoadEntriesUsing(const wxString& sql_statement)
{
    DestroyEntryList();
    wxSQLite3ResultSet q1 = ListDatabase()->ExecuteQuery(sql_statement);
    while (q1.NextRow())
    {
        entrylist_.push_back(new TTransactionBillEntry(q1));
    }
    q1.Finalize();
}

// delete all the objects in the list and clear the list.
void TTransactionBillList::DestroyEntryList()
{
    for (size_t i = 0; i < entrylist_.size(); ++i)
    {
        delete entrylist_[i];
    }
    entrylist_.clear();
}

int TTransactionBillList::AddEntry(TTransactionBillEntry* pTransBillEntry)
{
    entrylist_.push_back(pTransBillEntry);
    pTransBillEntry->Add(ListDatabase());

    return pTransBillEntry->id_;
}

void TTransactionBillList::DeleteEntry(int bill_entry_id)
{
    TTransactionBillEntry* pEntry = GetEntryPtr(bill_entry_id);
    if (pEntry)
    {
        pEntry->Delete(ListDatabase());
        entrylist_.erase(entrylist_.begin() + current_index_);
        delete pEntry;
    }
}

TTransactionBillEntry* TTransactionBillList::GetEntryPtr(int bill_entry_id)
{
    TTransactionBillEntry* pEntry = 0;
    size_t index = 0;
    while (index < entrylist_.size())
    {
        if (entrylist_[index]->id_ == bill_entry_id)
        {
            pEntry = entrylist_[index];
            current_index_ = index;
            break;
        }
        ++ index;
    }

    return pEntry;
}

TTransactionBillEntry* TTransactionBillList::GetIndexedEntryPtr(unsigned int list_index)
{
    TTransactionBillEntry* pEntry = 0;
    if (list_index < entrylist_.size())
    {
        pEntry = entrylist_[list_index];
    }

    return pEntry;
}

int TTransactionBillList::CurrentListSize()
{
    return entrylist_.size();
}
