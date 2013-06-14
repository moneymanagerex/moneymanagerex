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

#include "transaction.h"

/************************************************************************************
 TTransactionEntry Methods
 ***********************************************************************************/

/// Constructor for creating a new transaction entry
TTransactionEntry::TTransactionEntry()
: TEntryBase()
, id_from_account_(-1)
, id_to_account_(-1)
, id_payee_(-1)
, amount_from_(0.0)
, id_category_(-1)
, id_subcategory_(-1)
, id_followup_(-1)
, amount_to_(0.0)
{}

/// Copy constructor
TTransactionEntry::TTransactionEntry(TTransactionEntry* pEntry)
: TEntryBase()  // id has not been set yet
{
    id_from_account_ = pEntry->id_from_account_;
    id_to_account_   = pEntry->id_to_account_;
    id_payee_        = pEntry->id_payee_;
    trans_type_      = pEntry->trans_type_;   // transcode in database
    amount_from_     = pEntry->amount_from_;
    trans_status_    = pEntry->trans_status_;
    trans_num_       = pEntry->trans_num_;
    trans_notes_     = pEntry->trans_notes_;
    id_category_     = pEntry->id_category_;
    id_subcategory_  = pEntry->id_subcategory_;
    trans_date_      = pEntry->trans_date_;
    id_followup_     = pEntry->id_followup_;
    amount_to_       = pEntry->amount_to_;
}

/// Constructor used to load a transaction from the database.
TTransactionEntry::TTransactionEntry(wxSQLite3ResultSet& q1)
: TEntryBase()
{
    id_ = q1.GetInt("TRANSID");
    GetDatabaseValues(q1);
}

void TTransactionEntry::GetDatabaseValues(wxSQLite3ResultSet& q1)
{
    id_from_account_ = q1.GetInt("ACCOUNTID");
    id_to_account_   = q1.GetInt("TOACCOUNTID");
    id_payee_        = q1.GetInt("PAYEEID");
    trans_type_      = q1.GetString("TRANSCODE");
    amount_from_     = q1.GetDouble("TRANSAMOUNT");
    trans_status_    = q1.GetString("STATUS");
    trans_num_       = q1.GetString("TRANSACTIONNUMBER");
    trans_notes_     = q1.GetString("NOTES");
    id_category_     = q1.GetInt("CATEGID");
    id_subcategory_  = q1.GetInt("SUBCATEGID");
    trans_date_      = q1.GetDate("TRANSDATE");
    id_followup_     = q1.GetInt("FOLLOWUPID");
    amount_to_       = q1.GetDouble("TOTRANSAMOUNT");
}

void TTransactionEntry::SetDatabaseValues(wxSQLite3Statement& st, int& db_index)
{
    st.Bind(++db_index, id_from_account_);  // ACCOUNTID
    st.Bind(++db_index, id_to_account_);    // TOACCOUNTID
    st.Bind(++db_index, id_payee_);         // PAYEEID
    st.Bind(++db_index, trans_type_);       // TRANSCODE
    st.Bind(++db_index, amount_from_);      // TRANSAMOUNT
    st.Bind(++db_index, trans_status_);     // STATUS
    st.Bind(++db_index, trans_num_);        // TRANSACTIONNUMBER
    st.Bind(++db_index, trans_notes_);      // NOTES
    st.Bind(++db_index, id_category_);      // CATEGID
    st.Bind(++db_index, id_subcategory_);   // SUBCATEGID
    st.BindDate(++db_index, trans_date_);   // TRANSDATE
    st.Bind(++db_index, id_followup_);      // FOLLOWUPID
    st.Bind(++db_index, amount_to_);        // TOTRANSAMOUNT
}

int TTransactionEntry::Add(wxSQLite3Database* db)
{
    const char ADD_CHECKINGACCOUNT_V1_ROW[] =
    "insert into CHECKINGACCOUNT_V1"
    " (ACCOUNTID, TOACCOUNTID, PAYEEID, TRANSCODE, TRANSAMOUNT,"
    " STATUS, TRANSACTIONNUMBER, NOTES, CATEGID, SUBCATEGID,"
    " TRANSDATE, FOLLOWUPID, TOTRANSAMOUNT) "
    " values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

    wxSQLite3Statement st = db->PrepareStatement(ADD_CHECKINGACCOUNT_V1_ROW);
    int db_index = 0;
    SetDatabaseValues(st, db_index);

    FinaliseAdd(db, st);

    return id_;
}

void TTransactionEntry::Delete(wxSQLite3Database* db)
{
    DeleteEntry(db, "delete from CHECKINGACCOUNT_V1 where where TRANSID = ?");
}

void TTransactionEntry::Update(wxSQLite3Database* db)
{
    try
    {
        const char SQL_STATEMENT[] =
        "update CHECKINGACCOUNT_V1 set"
        " ACCOUNTID = ?, TOACCOUNTID = ?, PAYEEID = ?, TRANSCODE = ?, TRANSAMOUNT = ?,"
        " STATUS = ?, TRANSACTIONNUMBER = ?, NOTES = ?, CATEGID = ?, SUBCATEGID = ?,"
        " TRANSDATE = ?, FOLLOWUPID = ?, TOTRANSAMOUNT = ? "
        "where TRANSID = ? ";

        wxSQLite3Statement st = db->PrepareStatement(SQL_STATEMENT);
        int db_index = 0;
        SetDatabaseValues(st, db_index);
        st.Bind(++db_index, id_);

        FinaliseStatement(st);
    }
    catch(const wxSQLite3Exception& e)
    {
        wxLogError("TTransactionEntry:Update: %s", e.GetMessage().c_str());
    }
}

wxString TTransactionEntry::DisplayTransactionDate()
{
    return mmGetDateForDisplay(trans_date_);
}

bool TTransactionEntry::operator < (const TTransactionEntry& trans) const
{
    if (this->trans_date_ < trans.trans_date_)
        return true;
    else if (this->trans_date_ > trans.trans_date_)
        return false;

    return this->GetId() < trans.GetId();
}

double TTransactionEntry::AdjustedValue(int ref_account_id)
{
    double value = 0.0;
    if (trans_type_ == TRANS_TYPE_DEF[TYPE_WITHDRAWAL])
    {
        value = -amount_from_;
    }
    else if (trans_type_ == TRANS_TYPE_DEF[TYPE_DEPOSIT])
    {
        value = amount_from_;
    }
    else if (trans_type_ == TRANS_TYPE_DEF[TYPE_TRANSFER])
    {
        if (ref_account_id == id_from_account_) value = -amount_from_;
        if (ref_account_id == id_to_account_)   value = amount_to_;
    }

    return value;
}

bool TTransactionEntry::IsTransferTo(int ref_account_id)
{
    bool result = false;
    if (trans_type_ == TRANS_TYPE_DEF[TYPE_TRANSFER])
    {
        if (ref_account_id == id_to_account_)
        {
            result = true;
        }
    }

    return result;
}

/************************************************************************************
 TTransactionList Methods
 ***********************************************************************************/
/// Constructor
TTransactionList::TTransactionList(wxSQLite3Database* db, bool load_entries)
: TListBase(db)
{
    LoadEntries(load_entries);
}

TTransactionList::~TTransactionList()
{
    DestroyEntryList();
}

void TTransactionList::LoadEntries(bool load_entries)
{
    try
    {
        if (!ListDatabase()->TableExists("CHECKINGACCOUNT_V1"))
        {
            const char CREATE_TABLE_CHECKINGACCOUNT_V1[]=
            "CREATE TABLE CHECKINGACCOUNT_V1(TRANSID integer primary key, "
            "ACCOUNTID integer NOT NULL, TOACCOUNTID integer, PAYEEID integer NOT NULL, TRANSCODE TEXT NOT NULL, "
            "TRANSAMOUNT numeric NOT NULL, STATUS TEXT, TRANSACTIONNUMBER TEXT, NOTES TEXT, "
            "CATEGID integer, SUBCATEGID integer, TRANSDATE TEXT, FOLLOWUPID integer, TOTRANSAMOUNT numeric)";

            ListDatabase()->ExecuteUpdate(CREATE_TABLE_CHECKINGACCOUNT_V1);
        }

        if (load_entries)
        {
            LoadEntriesUsing("select * from CHECKINGACCOUNT_V1");
        }
    }
    catch (const wxSQLite3Exception& e)
    {
        wxLogError("TTransactionList:LoadEntries %s", e.GetMessage().c_str());
    }
}

void TTransactionList::LoadEntriesUsing(const wxString& sql_statement)
{
    DestroyEntryList();
    wxSQLite3ResultSet q1 = ListDatabase()->ExecuteQuery(sql_statement);
    while (q1.NextRow())
    {
        entrylist_.push_back(new TTransactionEntry(q1));
    }
    q1.Finalize();
}

void TTransactionList::DestroyEntryList()
{
    for (size_t i = 0; i < entrylist_.size(); ++i)
    {
        delete entrylist_[i];
    }
    entrylist_.clear();
}

int TTransactionList::AddEntry(TTransactionEntry* pTransEntry)
{
    entrylist_.push_back(pTransEntry);
    pTransEntry->Add(ListDatabase());

    return pTransEntry->id_;
}

void TTransactionList::DeleteEntry(int trans_id)
{
    TTransactionEntry* pEntry = GetEntryPtr(trans_id);
    if (pEntry)
    {
        pEntry->Delete(ListDatabase());
        entrylist_.erase(entrylist_.begin() + current_index_);
        delete pEntry;
    }
}

TTransactionEntry* TTransactionList::GetEntryPtr(int trans_id)
{
    TTransactionEntry* pEntry = 0;
    size_t index = 0;

    while (index < entrylist_.size())
    {
        if (entrylist_[index]->id_ == trans_id)
        {
            pEntry = entrylist_[index];
            current_index_ = index;
            break;
        }
        ++ index;
    }

    return pEntry;
}

TTransactionEntry* TTransactionList::GetIndexedEntryPtr(unsigned int list_index)
{
    TTransactionEntry* pEntry = 0;
    if (list_index < entrylist_.size())
    {
        pEntry = entrylist_[list_index];
    }

    return pEntry;
}

int TTransactionList::CurrentListSize()
{
    return entrylist_.size();
}
