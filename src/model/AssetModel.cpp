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
#include "TransactionLinkModel.h"
#include "CurrencyHistoryModel.h"

ChoicesName AssetModel::TYPE_CHOICES = ChoicesName({
    { TYPE_ID_PROPERTY,  _n("Property") },
    { TYPE_ID_AUTO,      _n("Automobile") },
    { TYPE_ID_HOUSE,     _n("Household Object") },
    { TYPE_ID_ART,       _n("Art") },
    { TYPE_ID_JEWELLERY, _n("Jewellery") },
    { TYPE_ID_CASH,      _n("Cash") },
    { TYPE_ID_OTHER,     _n("Other") }
});

ChoicesName AssetModel::STATUS_CHOICES = ChoicesName({
    { STATUS_ID_CLOSED, _n("Closed") },
    { STATUS_ID_OPEN,   _n("Open") }
});

ChoicesName AssetModel::CHANGE_CHOICES = ChoicesName({
    { CHANGE_ID_NONE,       _n("None") },
    { CHANGE_ID_APPRECIATE, _n("Appreciates") },
    { CHANGE_ID_DEPRECIATE, _n("Depreciates") }
});

ChoicesName AssetModel::CHANGEMODE_CHOICES = ChoicesName({
    { CHANGEMODE_ID_PERCENTAGE, _n("Percentage") },
    { CHANGEMODE_ID_LINEAR,     _n("Linear") }
});

AssetModel::AssetModel()
: Model<AssetTable>()
{
}

AssetModel::~AssetModel()
{
}

/**
* Initialize the global AssetModel table.
* Reset the AssetModel table or create the table if it does not exist.
*/
AssetModel& AssetModel::instance(wxSQLite3Database* db)
{
    AssetModel& ins = Singleton<AssetModel>::instance();
    ins.m_db = db;
    ins.destroy_cache();
    ins.ensure_table();

    return ins;
}

/** Return the static instance of AssetModel table */
AssetModel& AssetModel::instance()
{
    return Singleton<AssetModel>::instance();
}

wxString AssetModel::get_asset_name(int64 asset_id)
{
    Data* asset = instance().cache_id(asset_id);
    if (asset)
        return asset->ASSETNAME;
    else
        return _t("Asset Error");
}

double AssetModel::balance()
{
    double balance = 0.0;
    for (const auto& r: this->get_all())
    {
        balance += value(r).second;
    }
    return balance;
}

AssetTable::ASSETTYPE AssetModel::ASSETTYPE(OP op, TYPE_ID type)
{
    return AssetTable::ASSETTYPE(op, type_name(type));
}

AssetTable::STARTDATE AssetModel::STARTDATE(OP op, const wxDate& date)
{
    return AssetTable::STARTDATE(op, date.FormatISODate());
}

wxDate AssetModel::STARTDATE(const Data* r)
{
    return parseDateTime(r->STARTDATE);
}

wxDate AssetModel::STARTDATE(const Data& r)
{
    return parseDateTime(r.STARTDATE);
}

CurrencyModel::Data* AssetModel::currency(const Data* /* r */)
{
    return CurrencyModel::instance().GetBaseCurrency();
}

std::pair<double, double> AssetModel::value(const Data* r)
{
    return instance().valueAtDate(r, wxDate::Today());
}

std::pair<double, double> AssetModel::value(const Data& r)
{
    return instance().valueAtDate(&r, wxDate::Today());
}

std::pair<double, double> AssetModel::valueAtDate(const Data* r, const wxDate& date)
{
    std::pair<double /*initial*/, double /*market*/> balance;
    if (date < STARTDATE(r)) return balance;

    TransactionLinkModel::Data_Set translink_records = TransactionLinkModel::instance().find(
        TransactionLinkModel::LINKRECORDID(r->ASSETID),
        TransactionLinkModel::LINKTYPE(this->refTypeName)
    );

    double dailyRate = r->VALUECHANGERATE / 36500.0;
    int changeType = change_id(r);

    auto applyChangeRate = [changeType, dailyRate](double& value, double days)
    {
        if (changeType == CHANGE_ID_APPRECIATE)
        {
            value *= exp(dailyRate * days);
        }
        else if (changeType == CHANGE_ID_DEPRECIATE)
        {
            value *= exp(-dailyRate * days);
        }
    };

    if (!translink_records.empty())
    {
        TransactionModel::Data_Set trans;
        for (const auto& link : translink_records)
        {
            const TransactionModel::Data* tran = TransactionModel::instance().cache_id(link.CHECKINGACCOUNTID);
            if(tran && tran->DELETEDTIME.IsEmpty()) trans.push_back(*tran);
        }

        std::stable_sort(trans.begin(), trans.end(), TransactionTable::SorterByTRANSDATE());

        wxDate last = date;
        for (const auto& tran: trans)
        {
            if (tran.ACCOUNTID < 0) {
              continue;
            }

            const wxDate tranDate = TransactionModel::getTransDateTime(tran);
            if (tranDate > date) break;

            if (last == date) last = tranDate;
            if (last < tranDate)
            {
                applyChangeRate(balance.second, static_cast<double>((tranDate - last).GetDays()));
                last = tranDate;
            }

            double accflow = TransactionModel::account_flow(tran, tran.ACCOUNTID);
            double amount = -1 * accflow *
                CurrencyHistoryModel::getDayRate(AccountModel::instance().cache_id(tran.ACCOUNTID)->CURRENCYID, tranDate);
            //double amount = -1 * TransactionModel::account_flow(tran, tran.ACCOUNTID) *
            //    CurrencyHistoryModel::getDayRate(AccountModel::instance().cache_id(tran.ACCOUNTID)->CURRENCYID, tranDate);

            if (amount >= 0)
            {
                balance.first += amount;
            }
            else
            {
                double unrealized_gl = balance.second - balance.first;
                balance.first += std::min(unrealized_gl + amount, 0.0);
            }

            balance.second += amount;

            // Self Transfer as Revaluation
            if (tran.ACCOUNTID == tran.TOACCOUNTID && TransactionModel::type_id(tran.TRANSCODE) == TransactionModel::TYPE_ID_TRANSFER)
            {
                // TODO honor TRANSAMOUNT => TOTRANSAMOUNT
                balance.second = tran.TOTRANSAMOUNT;
            }
        }

        applyChangeRate(balance.second, static_cast<double>((date - last).GetDays()));
    }
    else
    {
        balance = {r->VALUE, r->VALUE};
        applyChangeRate(balance.second, static_cast<double>((date - STARTDATE(r)).GetDays()));
    }
    return balance;
}
