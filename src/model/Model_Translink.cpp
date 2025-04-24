/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)
 Copyright (C) 2016 Stefano Giorgio

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

#include "Model_Translink.h"
#include "Model_Shareinfo.h"
#include "Model_CurrencyHistory.h"

Model_Translink::Model_Translink()
    : Model<DB_Table_TRANSLINK_V1>()
{
}

Model_Translink::~Model_Translink()
{
}

/**
* Initialize the global Model_Translink table.
* Reset the Model_Translink table or create the table if it does not exist.
*/
Model_Translink& Model_Translink::instance(wxSQLite3Database* db)
{
    Model_Translink& ins = Singleton<Model_Translink>::instance();
    ins.db_ = db;
    ins.destroy_cache();
    ins.ensure(db);

    return ins;
}

/** Return the static instance of Model_Translink table */
Model_Translink& Model_Translink::instance()
{
    return Singleton<Model_Translink>::instance();
}

Model_Translink::CHECKING_TYPE Model_Translink::type_checking(const int64 tt)
{
    if (tt == AS_INCOME_EXPENSE || tt == -1)
    {
        return AS_INCOME_EXPENSE;
    }
    else
    {
        return AS_TRANSFER;
    }
}

Model_Translink::Data* Model_Translink::SetAssetTranslink(const int64 asset_id
    , const int64 checking_id
    , const CHECKING_TYPE checking_type)
{
    return SetTranslink(checking_id, checking_type
        , Model_Asset::refTypeName, asset_id);
}

Model_Translink::Data* Model_Translink::SetStockTranslink(const int64 stock_id
    , const int64 checking_id
    , const CHECKING_TYPE checking_type)
{
    return SetTranslink(checking_id, checking_type
        , Model_Stock::refTypeName, stock_id);
}

Model_Translink::Data* Model_Translink::SetTranslink(const int64 checking_id
    , const CHECKING_TYPE checking_type
    , const wxString& link_type, const int64 link_record_id)
{
    Model_Translink::Data* translink = Model_Translink::instance().create();
    translink->CHECKINGACCOUNTID = checking_id;
    translink->LINKTYPE = link_type;
    translink->LINKRECORDID = link_record_id;
    Model_Translink::instance().save(translink);

    /*
    set the checking entry to recognise it as a foreign transaction
    set the checking type as AS_INCOME_EXPENSE = 32701 or AS_TRANSFER
    */
    Model_Checking::Data* checking_entry = Model_Checking::instance().get(checking_id);
    // checking_entry->TOACCOUNTID = checking_type;
    Model_Checking::instance().save(checking_entry);

    return translink;
}

template <typename T>
Model_Translink::Data_Set Model_Translink::TranslinkList(const int64 link_entry_id)
{
    Model_Translink::Data_Set translink_list = Model_Translink::instance().find(
        Model_Translink::LINKTYPE(T::refTypeName)
        , Model_Translink::LINKRECORDID(link_entry_id));

    return translink_list;
}

bool Model_Translink::HasShares(const int64 stock_id)
{
    if (TranslinkList<Model_Stock>(stock_id).empty())
    {
        return false;
    }

    return true;
}

Model_Translink::Data Model_Translink::TranslinkRecord(const int64 checking_id)
{
    auto i = Model_Translink::CHECKINGACCOUNTID(checking_id);
    Model_Translink::Data_Set translink_list = Model_Translink::instance().find(i);

    if (!translink_list.empty())
        return *translink_list.begin();
    else {
        wxSharedPtr<Model_Translink::Data> t(new Model_Translink::Data);
        return *t;
    }
}

template <typename T>
void Model_Translink::RemoveTransLinkRecords(const int64 entry_id)
{
    for (const auto& translink : TranslinkList<T>(entry_id))
    {
        Model_Checking::instance().remove(translink.CHECKINGACCOUNTID);
    }
}

// Explicit Instantiation
template void Model_Translink::RemoveTransLinkRecords<Model_Asset>(const int64);
template void Model_Translink::RemoveTransLinkRecords<Model_Stock>(const int64);

void Model_Translink::RemoveTranslinkEntry(const int64 checking_account_id)
{
    Data translink = TranslinkRecord(checking_account_id);
    Model_Shareinfo::RemoveShareEntry(translink.CHECKINGACCOUNTID);
    Model_Translink::instance().remove(translink.TRANSLINKID);

    if (translink.LINKTYPE == Model_Asset::refTypeName)
    {
        Model_Asset::Data* asset_entry = Model_Asset::instance().get(translink.LINKRECORDID);
        UpdateAssetValue(asset_entry);
    }

    if (translink.LINKTYPE == Model_Stock::refTypeName)
    {
        Model_Stock::Data* stock_entry = Model_Stock::instance().get(translink.LINKRECORDID);
        Model_Stock::UpdatePosition(stock_entry);
    }
}

void Model_Translink::UpdateAssetValue(Model_Asset::Data* asset_entry)
{
    Data_Set trans_list = TranslinkList<Model_Asset>(asset_entry->ASSETID);
    double new_value = 0;
    for (const auto &trans : trans_list)
    {
        Model_Checking::Data* asset_trans = Model_Checking::instance().get(trans.CHECKINGACCOUNTID);
        if (asset_trans && asset_trans->DELETEDTIME.IsEmpty() && Model_Checking::status_id(asset_trans->STATUS) != Model_Checking::STATUS_ID_VOID)
        {
            Model_Currency::Data* asset_currency = Model_Account::currency(Model_Account::instance().get(asset_trans->ACCOUNTID));
            const double conv_rate = Model_CurrencyHistory::getDayRate(asset_currency->CURRENCYID, asset_trans->TRANSDATE);

            if (asset_trans->TRANSCODE == Model_Checking::TYPE_NAME_DEPOSIT)
            {
                new_value -= asset_trans->TRANSAMOUNT * conv_rate; // Withdrawal from asset value
            }
            else
            {
                new_value += asset_trans->TRANSAMOUNT * conv_rate;  // Deposit to asset value
            }  
        }
    }

    if (asset_entry->VALUE != new_value)
    {
        asset_entry->VALUE = new_value;
        Model_Asset::instance().save(asset_entry);
    }
}

bool Model_Translink::ShareAccountId(int64& stock_entry_id)
{
    Model_Translink::Data_Set stock_translink_list = TranslinkList<Model_Stock>(stock_entry_id);

    if (!stock_translink_list.empty())
    {
        Model_Checking::Data_Set checking_entry = Model_Checking::instance().find(
            Model_Checking::TRANSID(stock_translink_list.at(0).CHECKINGACCOUNTID));
        if (!checking_entry.empty())
        {
            Model_Account::Data* account_entry = Model_Account::instance().get(checking_entry.at(0).ACCOUNTID);
            stock_entry_id = account_entry->ACCOUNTID;
            return true;
        }
    }

    return false;
}

