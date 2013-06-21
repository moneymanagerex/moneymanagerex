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

#include "splittransaction.h"

/***********************************************************************************
 TSplitEntry Methods
 **********************************************************************************/
TSplitEntry::TSplitEntry()
: TEntryBase()
, id_trans_(-1)
, id_category_(-1)
, id_subcategory_(-1)
, amount_(0.0)
{}

// Holds a single entry for a known transaction
TSplitEntry::TSplitEntry(wxSQLite3ResultSet& q1)
: TEntryBase()
{
    id_             = q1.GetInt("SPLITTRANSID");
    id_trans_       = q1.GetInt("TRANSID");  
    id_category_    = q1.GetInt("CATEGID");
    id_subcategory_ = q1.GetInt("SUBCATEGID");
    amount_         = q1.GetDouble("SPLITTRANSAMOUNT");
}

void TSplitEntry::Add(wxSQLite3Database* db, wxString db_table)
{
    wxString sql_statement;
    sql_statement << "INSERT INTO " << db_table
                  << " (TRANSID, CATEGID, SUBCATEGID, SPLITTRANSAMOUNT )"
                  << " values (?, ?, ?, ?)";
    wxSQLite3Statement st = db->PrepareStatement(sql_statement);
    st.Bind(1, id_trans_);
    st.Bind(2, id_category_);
    st.Bind(3, id_subcategory_);
    st.Bind(4, amount_);

    this->FinaliseAdd(db, st);
}

void TSplitEntry::Update(wxSQLite3Database* db, wxString db_table)
{
    wxString sql_statement;
    sql_statement << "UPDATE " << db_table << " SET "
                  << "TRANSID = ?, CATEGID = ?, SUBCATEGID = ?, "
                  << "SPLITTRANSAMOUNT = ? "
                  << "WHERE SPLITTRANSID = ?";
    wxSQLite3Statement st = db->PrepareStatement(sql_statement);
    st.Bind(1, id_trans_);
    st.Bind(2, id_category_);
    st.Bind(3, id_subcategory_);
    st.Bind(4, amount_);
    st.Bind(5, id_);

    this->FinaliseStatement(st);
}

void TSplitEntry::Delete(wxSQLite3Database* db, wxString db_table)
{
    wxString sql_statement;
    sql_statement << "delete from " << db_table 
                  << " where SPLITTRANSID = ?";
    DeleteEntry(db, sql_statement);
}
//-----------------------------------------------------------------------------


/***********************************************************************************
 TSplitEntriesList
 **********************************************************************************/
TSplitEntriesList::TSplitEntriesList(wxSQLite3Database* db, wxString db_table)
: TListBase(db)
, db_table_(db_table)
, global_entries_()
{
    LoadSplitEntries();
}

void TSplitEntriesList::LoadSplitEntries()
{
    try
    {
        if (! ListDatabase()->TableExists(db_table_))
        {
            wxString sql_statement;
            sql_statement << "CREATE TABLE " << db_table_
                          << "(SPLITTRANSID integer primary key, "
                          << " TRANSID numeric NOT NULL, "
                          << " CATEGID integer, SUBCATEGID integer, SPLITTRANSAMOUNT numeric)";
            ListDatabase()->ExecuteUpdate(sql_statement);
        }

        wxString sql_statement;
        sql_statement << "select SPLITTRANSID, TRANSID,"
                      << "       CATEGID, SUBCATEGID,"
                      << "       SPLITTRANSAMOUNT "
                      << "from " << db_table_ << " order by SPLITTRANSID";
        wxSQLite3ResultSet q1 = ListDatabase()->ExecuteQuery(sql_statement);
        while (q1.NextRow())
        {
            TSplitEntry* pEntry(new TSplitEntry(q1));
            global_entries_.push_back(pEntry);
        }
        q1.Finalize();
    }
    catch (const wxSQLite3Exception& e)
    {
        wxString msg = "LoadSplitEntries " + db_table_ + " ";
        wxLogDebug(msg, e.GetMessage().c_str());
        wxLogError(msg + wxString::Format(_("Error: %s"), e.GetMessage().c_str()));
    }
}
//-----------------------------------------------------------------------------


/***********************************************************************************
 TSplitTransactionList
 Holds the list of split transactions for a transaction
 **********************************************************************************/
TSplitTransactionList::TSplitTransactionList(int id_transaction, TSplitEntriesList& entries_List)
: id_transaction_(id_transaction)
, entries_List_(entries_List)
, total_(0.0)
, entries_()
{
    LoadEntries();
}

// Load a list of entries for a transaction from the global list
void TSplitTransactionList::LoadEntries()
{
    for (size_t index = 0; index < entries_List_.global_entries_.size(); ++index)
    {
        if (entries_List_.global_entries_[index]->id_trans_ == id_transaction_)
        {
            TSplitEntry* pEntry = entries_List_.global_entries_[index];
            entries_.push_back(pEntry);
            total_ += pEntry->amount_;
        }
    }
}

// creating a new transaction list - Trans ID not known yet.
TSplitTransactionList::TSplitTransactionList(TSplitEntriesList& entries_List)
: id_transaction_(-1)
, entries_List_(entries_List)
, total_(0.0)
, entries_()
{}

void TSplitTransactionList::AddLocalEntry(int cat_id, int subcat_id, double amount)
{
    TSplitEntry* pEntry(new TSplitEntry());
    pEntry->id_category_ = cat_id;
    pEntry->id_subcategory_ = subcat_id;
    pEntry->amount_ = amount;
    total_ += pEntry->amount_;
    entries_.push_back(pEntry);                 // Add to local list
}

void TSplitTransactionList::SaveListforTransaction(int id_transaction)
{
    id_transaction_ = id_transaction;
    entries_List_.ListDatabase()->Begin();

    for (size_t i = 0; i < entries_.size(); ++i)
    {
        entries_List_.global_entries_.push_back(entries_[i]);   // Add to global list
        entries_[i]->Add(entries_List_.ListDatabase(), entries_List_.db_table_);
    }
    entries_List_.ListDatabase()->Commit();
}

double TSplitTransactionList::TotalAmount()
{
    return total_;
}

void TSplitTransactionList::AddEntry(TSplitEntry* pEntry)
{
    total_ += pEntry->amount_;
    entries_.push_back(pEntry);                 // Add to local list
    entries_List_.global_entries_.push_back(pEntry);   // Add to global list
    pEntry->Add(entries_List_.ListDatabase(), entries_List_.db_table_);
}

int TSplitTransactionList::AddEntry(int cat_id, int subcat_id, double amount)
{
    TSplitEntry* pEntry(new TSplitEntry());

    pEntry->id_trans_ = id_transaction_;
    pEntry->id_category_ = cat_id;
    pEntry->id_subcategory_ = subcat_id;
    pEntry->amount_ = amount;
    AddEntry(pEntry);
    return pEntry->id_;
}

void TSplitTransactionList::UpdateEntry(TSplitEntry* split_entry)
{
    split_entry->Update(entries_List_.ListDatabase(), entries_List_.db_table_);
    ReEvaluateTotal();
}

void TSplitTransactionList::DeleteEntry(TSplitEntry* split_entry)
{
    split_entry->Delete(entries_List_.ListDatabase(), entries_List_.db_table_);
    entries_.clear();
    RemoveGlobalEntry(split_entry->id_);
    total_ = 0.0;
    LoadEntries();
}

void TSplitTransactionList::RemoveGlobalEntry(int entry_id)
{
    for (size_t index = 0; index < entries_List_.global_entries_.size(); ++index)
    {
        if (entries_List_.global_entries_[index]->id_ == entry_id)
        {
            entries_List_.global_entries_.erase(entries_List_.global_entries_.begin() + index);
            break;
        }
    }
}

int TSplitTransactionList::GetListSize()
{
    return entries_.size();
}

TSplitEntry* TSplitTransactionList::GetEntryPtr(int id_split_trans)
{
    TSplitEntry* entry_ptr = 0;
    for (size_t i = 0; i < entries_.size(); ++i)
    {
        if (entries_[i]->id_ == id_split_trans)
        {
            entry_ptr = entries_[i];
            break;
        }
    }

    return entry_ptr;
}

TSplitEntry* TSplitTransactionList::GetIndexedEntryPtr(int index)
{
    return entries_[index];
}

void TSplitTransactionList::ReEvaluateTotal()
{
    total_ = 0.0;
    for (size_t i = 0; i < entries_.size(); ++i)
    {
        total_ += entries_[i]->amount_;
    }
}
