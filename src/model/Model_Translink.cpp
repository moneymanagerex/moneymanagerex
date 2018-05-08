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
    : Model<DB_Table_TRANSLINK>()
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

Model_Translink::CHECKING_TYPE Model_Translink::type_checking(const int tt)
{
    if (tt == AS_TRANSFER)
    {
        return AS_TRANSFER;
    }
    else
    {
        return AS_INCOME_EXPENSE;
    }
}

Model_Translink::Data* Model_Translink::SetAssetTranslink(const int asset_id
    , const int checking_id
    , const CHECKING_TYPE checking_type)
{
    return SetTranslink(checking_id, checking_type
        , Model_Attachment::reftype_desc(Model_Attachment::ASSET), asset_id);
}

Model_Translink::Data* Model_Translink::SetStockTranslink(const int stock_id
    , const int checking_id
    , const CHECKING_TYPE checking_type)
{
    return SetTranslink(checking_id, checking_type
        , Model_Attachment::reftype_desc(Model_Attachment::STOCK), stock_id);
}

Model_Translink::Data* Model_Translink::SetTranslink(const int checking_id
    , const CHECKING_TYPE checking_type
    , const wxString& link_type, const int link_record_id)
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
    checking_entry->TOACCOUNTID = checking_type;
    Model_Checking::instance().save(checking_entry);

    return translink;
}

Model_Translink::Data_Set Model_Translink::TranslinkList(Model_Attachment::REFTYPE link_table
    , const int link_entry_id)
{
    Model_Translink::Data_Set translink_list = Model_Translink::instance().find(
        Model_Translink::LINKTYPE(Model_Attachment::reftype_desc(link_table))
        , Model_Translink::LINKRECORDID(link_entry_id));

    return translink_list;
}

bool Model_Translink::HasShares(const int stock_id)
{
    if (TranslinkList(Model_Attachment::STOCK, stock_id).empty())
    {
        return false;
    }

    return true;
}

Model_Translink::Data Model_Translink::TranslinkRecord(const int checking_id)
{
    Model_Translink::Data_Set translink_list = Model_Translink::instance().find(
        Model_Translink::CHECKINGACCOUNTID(checking_id));

    return translink_list.at(0);
}

void Model_Translink::RemoveTransLinkRecords(Model_Attachment::REFTYPE table_type, const int entry_id)
{
    for (const auto& translink : TranslinkList(table_type, entry_id))
    {
        if (table_type == Model_Attachment::STOCK)
        {
            Model_Shareinfo::RemoveShareEntry(translink.CHECKINGACCOUNTID);
        }
        Model_Checking::instance().remove(translink.CHECKINGACCOUNTID);
        Model_Translink::instance().remove(translink.TRANSLINKID);
    }
}

void Model_Translink::RemoveTranslinkEntry(const int checking_account_id)
{
    Data translink = TranslinkRecord(checking_account_id);
    Model_Shareinfo::RemoveShareEntry(translink.CHECKINGACCOUNTID);
    Model_Checking::instance().remove(translink.CHECKINGACCOUNTID);
    Model_Translink::instance().remove(translink.TRANSLINKID);

    if (translink.LINKTYPE == Model_Attachment::reftype_desc(Model_Attachment::ASSET))
    {
        Model_Asset::Data* asset_entry = Model_Asset::instance().get(translink.LINKRECORDID);
        UpdateAssetValue(asset_entry);
    }

    if (translink.LINKTYPE == Model_Attachment::reftype_desc(Model_Attachment::STOCK))
    {
        Model_Stock::Data* stock_entry = Model_Stock::instance().get(translink.LINKRECORDID);
        UpdateStockValue(stock_entry);
    }
}

void Model_Translink::UpdateStockValue(Model_Stock::Data* stock_entry)
{
    Data_Set trans_list = TranslinkList(Model_Attachment::REFTYPE::STOCK, stock_entry->STOCKID);
    double total_shares = 0;
    double total_initial_value = 0;
    double total_commission = 0;
    for (const auto trans : trans_list)
    {
        Model_Shareinfo::Data* share_entry = Model_Shareinfo::ShareEntry(trans.CHECKINGACCOUNTID);

        total_shares += share_entry->SHARENUMBER;
        if (total_shares < 0) total_shares = 0;

        total_initial_value += share_entry->SHARENUMBER * share_entry->SHAREPRICE;
        if (total_initial_value < 0) total_initial_value = 0;

        total_commission += share_entry->SHARECOMMISSION;
    }

    // The stock record contains the total of share transactions.
    if (trans_list.empty())
    {
        stock_entry->PURCHASEPRICE = stock_entry->CURRENTPRICE;
    }
    else
    {
        stock_entry->PURCHASEPRICE = 0;
        stock_entry->NUMSHARES = total_shares;
        stock_entry->VALUE = total_initial_value;
        stock_entry->COMMISSION = total_commission;
    }
    Model_Stock::instance().save(stock_entry);
}

void Model_Translink::UpdateAssetValue(Model_Asset::Data* asset_entry)
{
    Data_Set trans_list = TranslinkList(Model_Attachment::REFTYPE::ASSET, asset_entry->ASSETID);
    bool value_updated = false;
    double new_value = 0;
    for (const auto trans : trans_list)
    {
        Model_Checking::Data* asset_trans = Model_Checking::instance().get(trans.CHECKINGACCOUNTID);
        if (asset_trans)
        {
            if (!Model_Checking::foreignTransactionAsTransfer(*asset_trans))
            {
                Model_Currency::Data* asset_currency = Model_Account::currency(Model_Account::instance().get(asset_trans->ACCOUNTID));
                const double conv_rate = Model_CurrencyHistory::getDayRate(asset_currency->CURRENCYID, asset_trans->TRANSDATE);

                if (asset_trans->TRANSCODE == Model_Checking::all_type()[Model_Checking::DEPOSIT])
                {
                    new_value -= asset_trans->TRANSAMOUNT * conv_rate; // Withdrawal from asset value
                }
                else
                {
                    new_value += asset_trans->TRANSAMOUNT * conv_rate;  // Deposit to asset value
                }

                asset_entry->VALUE = new_value;
                value_updated = true;
            }
        }
    }

    if (value_updated)
    {
        Model_Asset::instance().save(asset_entry);
    }
}

bool Model_Translink::ShareAccountId(int& stock_entry_id)
{
    Model_Translink::Data_Set stock_translink_list = TranslinkList(Model_Attachment::STOCK, stock_entry_id);

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

