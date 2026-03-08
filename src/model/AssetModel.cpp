/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)
 Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)

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

#include "AssetModel.h"
#include "TrxLinkModel.h"
#include "CurrencyHistoryModel.h"

const RefTypeN AssetModel::s_ref_type = RefTypeN(RefTypeN::e_asset);

AssetModel::AssetModel() :
    TableFactory<AssetTable, AssetData>()
{
}

AssetModel::~AssetModel()
{
}

// Initialize the global AssetModel table.
// Reset the AssetModel table or create the table if it does not exist.
AssetModel& AssetModel::instance(wxSQLite3Database* db)
{
    AssetModel& ins = Singleton<AssetModel>::instance();
    ins.reset_cache();
    ins.m_db = db;
    ins.ensure_table();

    return ins;
}

// Return the static instance of AssetModel table
AssetModel& AssetModel::instance()
{
    return Singleton<AssetModel>::instance();
}

AssetCol::ASSETTYPE AssetModel::ASSETTYPE(OP op, AssetType type)
{
    return AssetCol::ASSETTYPE(op, type.name());
}

AssetCol::STARTDATE AssetModel::STARTDATE(OP op, const mmDate& date)
{
    // OP_EQ and OP_NE should not be used for date comparisons.
    // if needed, create an equivalent AND/OR combination of two other operators.
    return AssetCol::STARTDATE(op,
        (op == OP_GE || op == OP_LT) ? date.isoStart() :
        (op == OP_LE || op == OP_GT) ? date.isoEnd() :
        date.isoDate()
    );
}

const wxString AssetModel::get_id_name(int64 asset_id)
{
    const Data* asset_n = get_id_data_n(asset_id);
    if (asset_n)
        return asset_n->m_name;
    else
        return _t("Asset Error");
}

// Return the value of an asset at a given date
const std::pair<double, double> AssetModel::get_data_value_date(const Data& asset_d, const mmDate& date)
{
    std::pair<double /*initial*/, double /*market*/> balance;

    if (date < asset_d.m_start_date)
        return balance;

    mmChoiceId change_id = asset_d.m_change.id();
    double daily_rate = asset_d.m_change_rate / 36500.0;
    auto apply_change = [change_id, daily_rate](double& value, mmDate from_date, mmDate to_date) {
        if (change_id == AssetChange::e_none)
            return;

        double days = static_cast<double>(
            (to_date.getDateTime() - from_date.getDateTime()).GetDays()
        );

        if (change_id == AssetChange::e_appreciates) {
            value *= exp(daily_rate * days);
        }
        else if (change_id == AssetChange::e_depreciates) {
            value *= exp(-daily_rate * days);
        }
    };

    TrxLinkModel::DataA tl_a = TrxLinkModel::instance().find(
        TrxLinkCol::LINKRECORDID(asset_d.m_id),
        TrxLinkCol::LINKTYPE(s_ref_type.name_n())
    );

    TrxModel::DataA trx_a;
    for (const auto& tl_d : tl_a) {
        const TrxData* trx_n = TrxModel::instance().get_id_data_n(tl_d.m_trx_id);
        if (trx_n &&
            trx_n->DELETEDTIME.IsEmpty() &&
            // FIXME: ignore Void transactions
            trx_n->m_account_id >= 0 &&
            date < mmDate(TrxModel::getTransDateTime(*trx_n))
        ) {
            trx_a.push_back(*trx_n);
        }
    }
    std::stable_sort(trx_a.begin(), trx_a.end(), TrxData::SorterByTRANSDATE());

    if (!tl_a.empty()) {
        mmDateN last_n = mmDateN();
        for (const auto& trx_d : trx_a) {
            const mmDate trx_date = mmDate(TrxModel::getTransDateTime(trx_d));
            const AccountData* account_n = AccountModel::instance().get_id_data_n(trx_d.m_account_id);
            int64 currency_id_n = account_n ? account_n->m_currency_id : -1;
            double currency_rate = CurrencyHistoryModel::getDayRate(currency_id_n, trx_date.getDateTime());
            double account_flow = TrxModel::account_flow(trx_d, trx_d.m_account_id);
            double base_amount = -(account_flow * currency_rate);

            if (!last_n.has_value())
                last_n = trx_date;
            else if (last_n.value() < trx_date) {
                apply_change(balance.second, last_n.value(), trx_date);
                last_n = trx_date;
            }

            // FIXME: if (base_amount >= 0 || balance.second < balance.first)
            if (base_amount >= 0) {
                // cash flow from account to asset
                balance.first += base_amount;
            }
            else {
                // cash flow from asset to account
                double unrealized_gl = balance.second - balance.first;
                balance.first += std::min(unrealized_gl + base_amount, 0.0);
            }

            balance.second += base_amount;

            // Self Transfer as Revaluation
            // FIXME: missing currency conversion
            if (trx_d.m_account_id == trx_d.m_to_account_id_n &&
                TrxModel::type_id(trx_d.TRANSCODE) == TrxModel::TYPE_ID_TRANSFER
            ) {
                // TODO honor m_amount => m_to_amount
                balance.second = trx_d.m_to_amount;
            }
        }

        if (last_n.has_value()) {
            apply_change(balance.second, last_n.value(), date);
        }
    }
    else {
        balance = {asset_d.m_value, asset_d.m_value};
        apply_change(balance.second, asset_d.m_start_date, date);
    }
    return balance;
}

// Return the current value of an asset
const std::pair<double, double> AssetModel::get_data_value(const Data& asset_d)
{
    return get_data_value_date(asset_d, mmDate::today());
}

// Return the current value of all assets
double AssetModel::find_all_balance()
{
    double balance = 0.0;
    for (const auto& asset_d : find_all()) {
        balance += get_data_value(asset_d).second;
    }
    return balance;
}
