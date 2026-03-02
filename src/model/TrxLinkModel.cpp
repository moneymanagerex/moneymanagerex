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

#include "TrxLinkModel.h"
#include "TrxShareModel.h"
#include "CurrencyHistoryModel.h"

TrxLinkModel::TrxLinkModel() :
    TableFactory<TrxLinkTable, TrxLinkData>()
{
}

TrxLinkModel::~TrxLinkModel()
{
}

/**
* Initialize the global TrxLinkModel table.
* Reset the TrxLinkModel table or create the table if it does not exist.
*/
TrxLinkModel& TrxLinkModel::instance(wxSQLite3Database* db)
{
    TrxLinkModel& ins = Singleton<TrxLinkModel>::instance();
    ins.reset_cache();
    ins.m_db = db;
    ins.ensure_table();

    return ins;
}

/** Return the static instance of TrxLinkModel table */
TrxLinkModel& TrxLinkModel::instance()
{
    return Singleton<TrxLinkModel>::instance();
}

TrxLinkModel::CHECKING_TYPE TrxLinkModel::type_checking(const int64 tt)
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

void TrxLinkModel::SetAssetTranslink(const int64 asset_id
    , const int64 checking_id
    , const CHECKING_TYPE checking_type)
{
    SetTranslink(checking_id, checking_type, AssetModel::refTypeName, asset_id);
}

void TrxLinkModel::SetStockTranslink(
    const int64 stock_id,
    const int64 checking_id,
    const CHECKING_TYPE checking_type
) {
    SetTranslink(checking_id, checking_type, StockModel::refTypeName, stock_id);
}

void TrxLinkModel::SetTranslink(
    const int64 checking_id,
    [[maybe_unused]] const CHECKING_TYPE checking_type,
    const wxString& link_type,
    const int64 link_record_id
) {
    TrxLinkData new_tl_d = TrxLinkData();
    new_tl_d.CHECKINGACCOUNTID = checking_id;
    new_tl_d.LINKTYPE          = link_type;
    new_tl_d.LINKRECORDID      = link_record_id;
    TrxLinkModel::instance().add_data_n(new_tl_d);

    // set the checking entry to recognise it as a foreign transaction
    // set the checking type as AS_INCOME_EXPENSE = 32701 or AS_TRANSFER
    TrxData* trx_n = TrxModel::instance().unsafe_get_id_data_n(checking_id);
    // trx_n->m_to_account_id_n = checking_type;
    TrxModel::instance().unsafe_save_trx(trx_n);
    //TrxLinkModel::instance().get_id_data_n(new_tl_d.id());
}

template <typename T>
TrxLinkModel::DataA TrxLinkModel::TranslinkList(const int64 link_entry_id)
{
    TrxLinkModel::DataA translink_list = TrxLinkModel::instance().find(
        TrxLinkCol::LINKTYPE(T::refTypeName),
        TrxLinkCol::LINKRECORDID(link_entry_id)
    );

    return translink_list;
}

TrxLinkModel::DataA TrxLinkModel::TranslinkListBySymbol(const wxString symbol)
{
    TrxLinkModel::DataA result;
    StockModel::DataA stocks = StockModel::instance().find(StockCol::SYMBOL(symbol));
    for (auto& stock : stocks) {
       TrxLinkModel::DataA t = TrxLinkModel::instance().find(
            TrxLinkCol::LINKRECORDID(stock.m_id)
        );
       result.insert(result.end(), t.begin(), t.end());
    }
    return result;
}

bool TrxLinkModel::HasShares(const int64 stock_id)
{
    if (TranslinkList<StockModel>(stock_id).empty())
    {
        return false;
    }

    return true;
}

TrxLinkData TrxLinkModel::TranslinkRecord(const int64 checking_id)
{
    auto i = TrxLinkCol::CHECKINGACCOUNTID(checking_id);
    TrxLinkModel::DataA translink_list = TrxLinkModel::instance().find(i);

    if (!translink_list.empty())
        return *translink_list.begin();
    else {
        wxSharedPtr<TrxLinkData> t(new TrxLinkData);
        return *t;
    }
}

template <typename T>
void TrxLinkModel::RemoveTransLinkRecords(const int64 entry_id)
{
    for (const auto& translink : TranslinkList<T>(entry_id))
    {
        TrxModel::instance().purge_id(translink.CHECKINGACCOUNTID);
    }
}

// Explicit Instantiation
template void TrxLinkModel::RemoveTransLinkRecords<AssetModel>(const int64);
template void TrxLinkModel::RemoveTransLinkRecords<StockModel>(const int64);

void TrxLinkModel::RemoveTranslinkEntry(const int64 checking_account_id)
{
    Data translink = TranslinkRecord(checking_account_id);
    TrxShareModel::instance().remove_trx_share(translink.CHECKINGACCOUNTID);
    TrxLinkModel::instance().purge_id(translink.TRANSLINKID);

    if (translink.LINKTYPE == AssetModel::refTypeName) {
        AssetData* asset_entry = AssetModel::instance().unsafe_get_id_data_n(translink.LINKRECORDID);
        UpdateAssetValue(asset_entry);
    }

    if (translink.LINKTYPE == StockModel::refTypeName) {
        StockData* stock_entry = StockModel::instance().unsafe_get_id_data_n(translink.LINKRECORDID);
        StockModel::UpdatePosition(stock_entry);
    }
}

void TrxLinkModel::UpdateAssetValue(AssetData* asset_n)
{
    DataA trans_list = TranslinkList<AssetModel>(asset_n->m_id);
    double new_value = 0;
    for (const auto &trans : trans_list) {
        const TrxData* trx_n = TrxModel::instance().get_id_data_n(trans.CHECKINGACCOUNTID);
        if (trx_n && trx_n->DELETEDTIME.IsEmpty()
            && TrxModel::status_id(trx_n->STATUS) != TrxModel::STATUS_ID_VOID
        ) {
            const CurrencyData* currency_n = AccountModel::instance().get_id_currency_p(
                trx_n->m_account_id
            );
            const double conv_rate = CurrencyHistoryModel::getDayRate(
                currency_n->m_id,
                trx_n->TRANSDATE
            );

            if (trx_n->TRANSCODE == TrxModel::TYPE_NAME_DEPOSIT) {
                new_value -= trx_n->m_amount * conv_rate; // Withdrawal from asset value
            }
            else {
                new_value += trx_n->m_amount * conv_rate;  // Deposit to asset value
            }
        }
    }

    if (asset_n->m_value != new_value) {
        asset_n->m_value = new_value;
        AssetModel::instance().unsafe_save_data_n(asset_n);
    }
}

bool TrxLinkModel::ShareAccountId(int64& stock_entry_id)
{
    TrxLinkModel::DataA stock_translink_list = TranslinkList<StockModel>(stock_entry_id);

    if (!stock_translink_list.empty())
    {
        TrxModel::DataA checking_entry = TrxModel::instance().find(
            TrxCol::TRANSID(stock_translink_list.at(0).CHECKINGACCOUNTID));
        if (!checking_entry.empty())
        {
            const AccountData* account_entry = AccountModel::instance().get_id_data_n(checking_entry.at(0).m_account_id);
            stock_entry_id = account_entry->m_id;
            return true;
        }
    }

    return false;
}
