/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)
 Copyright (C) 2016 Stefano Giorgio
 Copyright (C) 2025 Klaus Wich

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

#include "TransactionLinkModel.h"
#include "TransactionShareModel.h"
#include "CurrencyHistoryModel.h"

TransactionLinkModel::TransactionLinkModel()
    : Model<TransactionLinkTable>()
{
}

TransactionLinkModel::~TransactionLinkModel()
{
}

/**
* Initialize the global TransactionLinkModel table.
* Reset the TransactionLinkModel table or create the table if it does not exist.
*/
TransactionLinkModel& TransactionLinkModel::instance(wxSQLite3Database* db)
{
    TransactionLinkModel& ins = Singleton<TransactionLinkModel>::instance();
    ins.m_db = db;
    ins.destroy_cache();
    ins.ensure_table();

    return ins;
}

/** Return the static instance of TransactionLinkModel table */
TransactionLinkModel& TransactionLinkModel::instance()
{
    return Singleton<TransactionLinkModel>::instance();
}

TransactionLinkModel::CHECKING_TYPE TransactionLinkModel::type_checking(const int64 tt)
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

TransactionLinkModel::Data* TransactionLinkModel::SetAssetTranslink(const int64 asset_id
    , const int64 checking_id
    , const CHECKING_TYPE checking_type)
{
    return SetTranslink(checking_id, checking_type
        , AssetModel::refTypeName, asset_id);
}

TransactionLinkModel::Data* TransactionLinkModel::SetStockTranslink(const int64 stock_id
    , const int64 checking_id
    , const CHECKING_TYPE checking_type)
{
    return SetTranslink(checking_id, checking_type
        , StockModel::refTypeName, stock_id);
}

TransactionLinkModel::Data* TransactionLinkModel::SetTranslink(const int64 checking_id
    , [[maybe_unused]] const CHECKING_TYPE checking_type
    , const wxString& link_type, const int64 link_record_id)
{
    TransactionLinkModel::Data* translink = TransactionLinkModel::instance().create();
    translink->CHECKINGACCOUNTID = checking_id;
    translink->LINKTYPE = link_type;
    translink->LINKRECORDID = link_record_id;
    TransactionLinkModel::instance().save(translink);

    /*
    set the checking entry to recognise it as a foreign transaction
    set the checking type as AS_INCOME_EXPENSE = 32701 or AS_TRANSFER
    */
    TransactionModel::Data* checking_entry = TransactionModel::instance().cache_id(checking_id);
    // checking_entry->TOACCOUNTID = checking_type;
    TransactionModel::instance().save(checking_entry);

    return translink;
}

template <typename T>
TransactionLinkModel::Data_Set TransactionLinkModel::TranslinkList(const int64 link_entry_id)
{
    TransactionLinkModel::Data_Set translink_list = TransactionLinkModel::instance().find(
        TransactionLinkModel::LINKTYPE(T::refTypeName)
        , TransactionLinkModel::LINKRECORDID(link_entry_id));

    return translink_list;
}

TransactionLinkModel::Data_Set TransactionLinkModel::TranslinkListBySymbol(const wxString symbol)
{
    TransactionLinkModel::Data_Set result;
    StockModel::Data_Set stocks = StockModel::instance().find(StockModel::SYMBOL(symbol));
    for (auto& stock : stocks) {
       TransactionLinkModel::Data_Set t = TransactionLinkModel::instance().find(TransactionLinkModel::LINKRECORDID(stock.STOCKID));
       result.insert(result.end(), t.begin(), t.end());
    }
    return result;
}

bool TransactionLinkModel::HasShares(const int64 stock_id)
{
    if (TranslinkList<StockModel>(stock_id).empty())
    {
        return false;
    }

    return true;
}

TransactionLinkModel::Data TransactionLinkModel::TranslinkRecord(const int64 checking_id)
{
    auto i = TransactionLinkModel::CHECKINGACCOUNTID(checking_id);
    TransactionLinkModel::Data_Set translink_list = TransactionLinkModel::instance().find(i);

    if (!translink_list.empty())
        return *translink_list.begin();
    else {
        wxSharedPtr<TransactionLinkModel::Data> t(new TransactionLinkModel::Data);
        return *t;
    }
}

template <typename T>
void TransactionLinkModel::RemoveTransLinkRecords(const int64 entry_id)
{
    for (const auto& translink : TranslinkList<T>(entry_id))
    {
        TransactionModel::instance().remove(translink.CHECKINGACCOUNTID);
    }
}

// Explicit Instantiation
template void TransactionLinkModel::RemoveTransLinkRecords<AssetModel>(const int64);
template void TransactionLinkModel::RemoveTransLinkRecords<StockModel>(const int64);

void TransactionLinkModel::RemoveTranslinkEntry(const int64 checking_account_id)
{
    Data translink = TranslinkRecord(checking_account_id);
    TransactionShareModel::RemoveShareEntry(translink.CHECKINGACCOUNTID);
    TransactionLinkModel::instance().remove(translink.TRANSLINKID);

    if (translink.LINKTYPE == AssetModel::refTypeName)
    {
        AssetModel::Data* asset_entry = AssetModel::instance().cache_id(translink.LINKRECORDID);
        UpdateAssetValue(asset_entry);
    }

    if (translink.LINKTYPE == StockModel::refTypeName)
    {
        StockModel::Data* stock_entry = StockModel::instance().cache_id(translink.LINKRECORDID);
        StockModel::UpdatePosition(stock_entry);
    }
}

void TransactionLinkModel::UpdateAssetValue(AssetModel::Data* asset_entry)
{
    Data_Set trans_list = TranslinkList<AssetModel>(asset_entry->ASSETID);
    double new_value = 0;
    for (const auto &trans : trans_list)
    {
        TransactionModel::Data* asset_trans = TransactionModel::instance().cache_id(trans.CHECKINGACCOUNTID);
        if (asset_trans && asset_trans->DELETEDTIME.IsEmpty() && TransactionModel::status_id(asset_trans->STATUS) != TransactionModel::STATUS_ID_VOID)
        {
            CurrencyModel::Data* asset_currency = AccountModel::currency(AccountModel::instance().cache_id(asset_trans->ACCOUNTID));
            const double conv_rate = CurrencyHistoryModel::getDayRate(asset_currency->CURRENCYID, asset_trans->TRANSDATE);

            if (asset_trans->TRANSCODE == TransactionModel::TYPE_NAME_DEPOSIT)
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
        AssetModel::instance().save(asset_entry);
    }
}

bool TransactionLinkModel::ShareAccountId(int64& stock_entry_id)
{
    TransactionLinkModel::Data_Set stock_translink_list = TranslinkList<StockModel>(stock_entry_id);

    if (!stock_translink_list.empty())
    {
        TransactionModel::Data_Set checking_entry = TransactionModel::instance().find(
            TransactionModel::TRANSID(stock_translink_list.at(0).CHECKINGACCOUNTID));
        if (!checking_entry.empty())
        {
            AccountModel::Data* account_entry = AccountModel::instance().cache_id(checking_entry.at(0).ACCOUNTID);
            stock_entry_id = account_entry->ACCOUNTID;
            return true;
        }
    }

    return false;
}
