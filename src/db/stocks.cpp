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

#include <numeric>
#include "stocks.h"
#include "../mmCurrencyFormatter.h"

/************************************************************************************
 TStockEntry Methods
 ***********************************************************************************/
/// Constructor used when loading stocks from the database
TStockEntry::TStockEntry(wxSQLite3ResultSet& q1)
: TEntryBase()
{
    id_         = q1.GetInt("STOCKID");
    heldat_     = q1.GetInt("HELDAT");
    pur_date_   = q1.GetDate("PURCHASEDATE");
    name_       = q1.GetString("STOCKNAME");
    symbol_     = q1.GetString("SYMBOL");
    num_shares_ = q1.GetDouble("NUMSHARES");
    pur_price_  = q1.GetDouble("PURCHASEPRICE");
    notes_      = q1.GetString("NOTES");
    cur_price_  = q1.GetDouble("CURRENTPRICE");
    value_      = q1.GetDouble("VALUE");
    commission_ = q1.GetDouble("COMMISSION");
}

/// Copy constructor using a pointer
TStockEntry::TStockEntry(TStockEntry* pEntry)
: TEntryBase()
{
    heldat_     = pEntry->heldat_;
    pur_date_   = pEntry->pur_date_;
    name_       = pEntry->name_;
    symbol_     = pEntry->symbol_;
    num_shares_ = pEntry->num_shares_;
    pur_price_  = pEntry->pur_price_;
    notes_      = pEntry->notes_;
    cur_price_  = pEntry->cur_price_;
    value_      = pEntry->value_;
    commission_ = pEntry->commission_;
}

void TStockEntry::SetDatabaseValues(wxSQLite3Statement& st, int& db_index)
{
    st.Bind(++db_index, heldat_);       // "HELDAT"
    st.BindDate(++db_index, pur_date_); // "PURCHASEDATE"
    st.Bind(++db_index, name_);         // "STOCKNAME"
    st.Bind(++db_index, symbol_);       // "SYMBOL"
    st.Bind(++db_index, num_shares_);   // "NUMSHARES"
    st.Bind(++db_index, pur_price_);    // "PURCHASEPRICE"
    st.Bind(++db_index, notes_);        // "NOTES"
    st.Bind(++db_index, cur_price_);    // "CURRENTPRICE"
    st.Bind(++db_index, value_);        // "VALUE"
    st.Bind(++db_index, commission_);   // "COMMISSION"
}

/// Constructor for creating a new stock entry.
TStockEntry::TStockEntry()
: TEntryBase()
, heldat_(-1)
, num_shares_(0.0)
, pur_price_(0.0)
, cur_price_(0.0)
, value_(0.0)
, commission_(0.0)
{}

int TStockEntry::Add(wxSQLite3Database* db)
{
    const char ADD_STOCK_V1_ROW[] =
    "insert into STOCK_V1 "
    " (HELDAT, PURCHASEDATE, STOCKNAME, SYMBOL, NUMSHARES,"
    " PURCHASEPRICE, NOTES, CURRENTPRICE, VALUE, COMMISSION)"
    " values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

    wxSQLite3Statement st = db->PrepareStatement(ADD_STOCK_V1_ROW);
    int db_index = 0;
    SetDatabaseValues(st, db_index);
    
    FinaliseAdd(db, st);

    return id_;
}

void TStockEntry::Delete(wxSQLite3Database* db)
{
    DeleteEntry(db, "delete from STOCK_V1 where STOCKID = ?"); 
}

void TStockEntry::Update(wxSQLite3Database* db)
{
    try
    {
        const char SQL_STATEMENT[] =
        "update STOCK_V1 set"
        " HELDAT = ?, PURCHASEDATE = ?, STOCKNAME = ?, SYMBOL = ?, NUMSHARES = ?,"
        " PURCHASEPRICE = ?, NOTES = ?, CURRENTPRICE = ?, VALUE = ?, COMMISSION = ? "
        "where STOCKID = ?";
        
        wxSQLite3Statement st = db->PrepareStatement(SQL_STATEMENT);
        int db_index = 0;
        SetDatabaseValues(st, db_index);
        st.Bind(++db_index, id_);

        FinaliseStatement(st);
    }
    catch(const wxSQLite3Exception& e)
    {
        wxLogError("TStockEntry:Update: %s", e.GetMessage().c_str());
    }
}

wxString TStockEntry::DisplayDate()
{
    return mmGetDateForDisplay(pur_date_);
}

wxString TStockEntry::NumberOfShares(bool whole_num)
{
    wxString format = "%.4f";
    if (whole_num)
    {
        format = "%.0f";
    }
    return wxString::Format(format, num_shares_);
}

wxString TStockEntry::CurrentPrice()
{
    return wxString::Format("%.4f", cur_price_);
}

double TStockEntry::GetValue()
{
    return (cur_price_ * num_shares_) - commission_; 
}

wxString TStockEntry::GetValueCurrencyEditFormat(bool initial_value)
{
    wxString formatted_value;
    if (initial_value)  formatted_value = CurrencyFormatter::float2String(value_);
    else  formatted_value = CurrencyFormatter::float2String(GetValue());

    return formatted_value;
}

wxString TStockEntry::CommissionCurrencyEditFormat()
{
    wxString formatted_value = CurrencyFormatter::float2String(value_);
    return formatted_value;
}

/************************************************************************************
 TStockList Methods
 ***********************************************************************************/
/// Constructor
TStockList::TStockList(wxSQLite3Database* db, bool load_entries)
: TListBase(db)
{
    LoadEntries(load_entries);
}

TStockList::~TStockList()
{
    DestroyEntryList();
}

void TStockList::LoadEntries(bool load_entries)
{
    try
    {
        if (!ListDatabase()->TableExists("STOCK_V1"))
        {
            const char CREATE_TABLE_STOCK_V1[] =
            "CREATE TABLE STOCK_V1(STOCKID integer primary key, "
            "HELDAT numeric, PURCHASEDATE TEXT NOT NULL, STOCKNAME TEXT, SYMBOL TEXT, "
            "NUMSHARES numeric, PURCHASEPRICE numeric NOT NULL, NOTES TEXT, CURRENTPRICE numeric NOT NULL, "
            "VALUE numeric, COMMISSION numeric)";

            ListDatabase()->ExecuteUpdate(CREATE_TABLE_STOCK_V1);
        }

        if (load_entries)
        {
            LoadEntriesUsing("select * from STOCK_V1");
        }
    }
    catch (const wxSQLite3Exception& e)
    {
        wxLogError("TStockList::LoadEntries: %s", e.GetMessage().c_str());
    }
}

void TStockList::LoadEntriesUsing(const wxString& sql_statement)
{
    try
    {
        DestroyEntryList();
        wxSQLite3ResultSet q1 = ListDatabase()->ExecuteQuery(sql_statement);
        while (q1.NextRow())
        {
            entrylist_.push_back(new TStockEntry(q1));
        }
        q1.Finalize();
    }
    catch (const wxSQLite3Exception& e)
    {
        wxLogError("TStockList::LoadEntriesUsing: %s", e.GetMessage().c_str());
    }
}

void TStockList::DestroyEntryList()
{
    for (size_t i = 0; i < entrylist_.size(); ++i)
    {
        delete entrylist_[i];
    }
    entrylist_.clear();
}

int TStockList::AddEntry(TStockEntry* pEntry)
{
    pEntry->Add(ListDatabase());
    entrylist_.push_back(pEntry);

    return pEntry->id_;
}

void TStockList::DeleteEntry(int stock_id)
{
    TStockEntry* pEntry = GetEntryPtr(stock_id);
    if (pEntry)
    {
        pEntry->Delete(ListDatabase());
        entrylist_.erase(entrylist_.begin() + current_index_);
        delete pEntry;
    }
}

TStockEntry* TStockList::GetEntryPtr(int stock_id)
{
    TStockEntry* pEntry = 0;
    size_t index = 0;

    while (index < entrylist_.size())
    {
        if (entrylist_[index]->id_ == stock_id)
        {
            pEntry = entrylist_[index];
            current_index_ = index;
            break;
        }
        ++ index;
    }

    return pEntry;
}

TStockEntry* TStockList::GetIndexedEntryPtr(unsigned int list_index)
{
    TStockEntry* pEntry = 0;
    if (list_index < entrylist_.size())
    {
        pEntry = entrylist_[list_index];
    }

    return pEntry;
}

int TStockList::CurrentListSize()
{
    return entrylist_.size();
}

double TStockList::GetStockBalance()
{
    double total_value = 0.0;
    for (const auto& entry : entrylist_) total_value += entry->value_;
    return total_value;
}

wxString TStockList::GetStockBalanceCurrencyFormat()
{
    return CurrencyFormatter::float2Money(GetStockBalance());
}

wxString TStockList::GetStockBalanceCurrencyEditFormat()
{
    return CurrencyFormatter::float2String(GetStockBalance());
}
