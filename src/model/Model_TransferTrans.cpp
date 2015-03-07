/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)
 Copyright (C) 2015 Stefano Giorgio

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

#include "Model_TransferTrans.h"

const std::vector<std::pair<Model_TransferTrans::TABLE_TYPE, wxString> > Model_TransferTrans::TABLE_TYPE_CHOICES = 
{
    std::make_pair(Model_TransferTrans::ASSETS, wxTRANSLATE("Assets"))
    , std::make_pair(Model_TransferTrans::STOCKS, wxTRANSLATE("Stocks"))
};

Model_TransferTrans::Model_TransferTrans()
: Model<DB_Table_TRANSFERTRANS_V1>()
{
}

Model_TransferTrans::~Model_TransferTrans()
{
}

/**
* Initialize the global Model_TransferTrans table.
* Reset the Model_TransferTrans table or create the table if it does not exist.
*/
Model_TransferTrans& Model_TransferTrans::instance(wxSQLite3Database* db)
{
    Model_TransferTrans& ins = Singleton<Model_TransferTrans>::instance();
    ins.db_ = db;
    ins.destroy_cache();
    ins.ensure(db);

    return ins;
}

/** Return the static instance of Model_TransferTrans table */
Model_TransferTrans& Model_TransferTrans::instance()
{
    return Singleton<Model_TransferTrans>::instance();
}

wxArrayString Model_TransferTrans::all_table_type()
{
    wxArrayString table_type;
    for (const auto& item : TABLE_TYPE_CHOICES)
    {
        table_type.Add(item.second);
    }
    return table_type;
}

Model_TransferTrans::TABLE_TYPE Model_TransferTrans::type(const Data* r)
{
    for (const auto & item : TABLE_TYPE_CHOICES)
    {
        if (item.second.CmpNoCase(r->TABLETYPE) == 0)
            return item.first;
    }
    return TABLE_TYPE(-1);
}

Model_TransferTrans::TABLE_TYPE Model_TransferTrans::type(const Data& r)
{
    return type(&r);
}

Model_TransferTrans::CHECKING_TYPE Model_TransferTrans::type_checking(const int& tt)
{
    if (tt == AS_TRANSFER)
        return AS_TRANSFER;
    else
        return AS_INCOME_EXPENSE;
}

Model_Currency::Data* Model_TransferTrans::Currency(const Data* r)
{
    Model_Currency::Data * currency = Model_Currency::instance().get(r->ID_CURRENCY);
    if (!currency)
    {
        return Model_Currency::GetBaseCurrency();
    }
    return currency;
}

Model_TransferTrans::Data* Model_TransferTrans::SetAssetTransferTransaction(const int& asset_id, const int& checking_id
    , const CHECKING_TYPE& transfer, const wxString& currency_symbol)
{
    return SetTransferTransaction(Model_TransferTrans::all_table_type()[Model_TransferTrans::ASSETS]
        , asset_id, checking_id, transfer, currency_symbol);
}

Model_TransferTrans::Data* Model_TransferTrans::SetShareTransferTransaction(const int& share_id, const int& checking_id
    , const double& share_price, const double& share_number, const double& share_commission
    , const CHECKING_TYPE& transfer, const wxString& currency_symbol)
{
    return SetTransferTransaction(Model_TransferTrans::all_table_type()[Model_TransferTrans::STOCKS]
        , share_id, checking_id, transfer, currency_symbol, share_price, share_number, share_commission);
}

Model_TransferTrans::Data* Model_TransferTrans::SetTransferTransaction(const wxString& trans_type
    , const int& table_record_id, const int& checking_id, const CHECKING_TYPE& transfer
    , const wxString& currency_symbol
    , const double& share_price, const double& share_number, const double& share_commission)
{
    int currency_id = -1;
    if (!currency_symbol.IsEmpty())
    {
        currency_id = Model_Currency::instance().GetCurrencyRecord(currency_symbol)->id();
    }
    else
    {
        currency_id = Model_Infotable::instance().GetBaseCurrencyId();
    }

    /*  Set up the transfer table for the purchase*/
    Model_TransferTrans::Data* trans_entry = Model_TransferTrans::instance().create();
    trans_entry->TABLETYPE = trans_type;
    trans_entry->ID_TABLERECORD = table_record_id;
    trans_entry->ID_CHECKINGACCOUNT = checking_id;
    trans_entry->ID_CURRENCY = currency_id;
    trans_entry->SHARE_UNITPRICE = share_price;
    trans_entry->SHARE_NUMBER = share_number;
    trans_entry->SHARE_COMMISSION = share_commission;
    Model_TransferTrans::instance().save(trans_entry);

    Model_Checking::Data* checking_entry = Model_Checking::instance().get(checking_id);
    checking_entry->TOACCOUNTID = transfer;
    Model_Checking::instance().save(checking_entry);

    return trans_entry;
}

Model_TransferTrans::Data_Set Model_TransferTrans::TransferList(Model_TransferTrans::TABLE_TYPE table_type, const int& entry_id)
{
    // select * from Model_TransferTrans where TABLETYPE = "Assets" AND ID_TABLERECORD = entry_id;
    // select * from Model_TransferTrans where TABLETYPE = "Stocks" AND ID_TABLERECORD = entry_id;
    Model_TransferTrans::Data_Set trans_list = Model_TransferTrans::instance().find(
        Model_TransferTrans::TABLETYPE(Model_TransferTrans::all_table_type()[table_type]),
        Model_TransferTrans::ID_TABLERECORD(entry_id));

    return trans_list;
}

Model_Checking::Data_Set Model_TransferTrans::TransferRecordList(Model_TransferTrans::TABLE_TYPE table_type, const int& entry_id)
{
    Model_TransferTrans::Data_Set transfer_list = TransferList(table_type, entry_id);
    Model_Checking::Data_Set record_list;
    for (const auto& trans_entry : transfer_list)
    {
        Model_Checking::Data* entry = Model_Checking::instance().get(trans_entry.ID);
        record_list.push_back(*entry);
    }

    return record_list;
}

Model_TransferTrans::Data Model_TransferTrans::TransferEntry(const int& checking_account_id)
{
    // select * from Model_TransferTrans where TABLETYPE = table_type AND ID_CHECKINGACCOUNT = checking_account_id;
    Model_TransferTrans::Data_Set transfer_list = Model_TransferTrans::instance().find(Model_TransferTrans::ID_CHECKINGACCOUNT(checking_account_id));
    
    return transfer_list.at(0);
}

void Model_TransferTrans::RemoveTransferTransactions(Model_TransferTrans::TABLE_TYPE table_type, const int& entry_id)
{
    Data_Set transfer_list = TransferList(table_type, entry_id);
    for (const auto& trans_entry : transfer_list)
    {
        Model_Checking::instance().remove(trans_entry.ID_CHECKINGACCOUNT);
        Model_TransferTrans::instance().remove(trans_entry.ID);
    }
}

void Model_TransferTrans::RemoveTransferEntry(const int& checking_account_id)
{
    Data trans_entry = TransferEntry(checking_account_id);
    Model_Checking::instance().remove(trans_entry.ID_CHECKINGACCOUNT);
    Model_TransferTrans::instance().remove(trans_entry.ID);
}
