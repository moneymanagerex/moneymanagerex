/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)
 Copyright (C) 2022,2025 Mark Whalley (mark@ipx.co.uk)

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

#include "StockModel.h"
#include "StockHistoryModel.h"
#include "TrxLinkModel.h"
#include "TrxShareModel.h"
#include "CurrencyHistoryModel.h"

StockModel::StockModel() :
    TableFactory<StockTable, StockData>()
{
}

StockModel::~StockModel()
{
}

// Initialize the global StockModel table.
// Reset the StockModel table or create the table if it does not exist.
StockModel& StockModel::instance(wxSQLite3Database* db)
{
    StockModel& ins = Singleton<StockModel>::instance();
    ins.reset_cache();
    ins.m_db = db;
    ins.ensure_table();

    return ins;
}

wxString StockModel::get_id_name(int64 stock_id)
{
    const Data* stock_n = instance().get_id_data_n(stock_id);
    if (stock_n)
        return stock_n->m_name;
    else
        return _t("Stock Error");
}

// Return the static instance of StockModel table
StockModel& StockModel::instance()
{
    return Singleton<StockModel>::instance();
}

wxDate StockModel::PURCHASEDATE(const Data& stock_d)
{
    return stock_d.m_purchase_date.getDateTime();
}

// Original value of Stocks
double StockModel::InvestmentValue(const Data& stock_d)
{
    return stock_d.m_purchase_value;
}

double StockModel::CurrentValue(const Data& stock_d)
{
    return stock_d.m_num_shares * stock_d.m_current_price;
}

// Remove the Data record from memory and the database.
// Delete also all stock history
bool StockModel::purge_id(int64 id)
{
    const StockData *stock_n = get_id_data_n(id);
    const auto& stock_a = StockModel::instance().find(
        StockCol::SYMBOL(stock_n->m_symbol)
    );
    if (stock_a.size() == 1) {
        db_savepoint();
        for (const auto& sh_d : StockHistoryModel::instance().find(
            StockHistoryCol::SYMBOL(stock_n->m_symbol)
        ))
            StockHistoryModel::instance().purge_id(sh_d.id());
        db_release_savepoint();
    }

    // FIXME: remove AttachmentData owned by id

    return unsafe_remove_id(id);
}

// Return the last price date of a given stock
wxString StockModel::lastPriceDate(const Data& stock_d)
{
    wxString dtStr = stock_d.m_purchase_date.isoDate();
    StockHistoryModel::DataA sh_a = StockHistoryModel::instance().find(
        StockCol::SYMBOL(stock_d.m_symbol)
    );

    std::sort(sh_a.begin(), sh_a.end(), StockHistoryData::SorterByDATE());
    if (!sh_a.empty())
        dtStr = sh_a.back().m_date.isoDate();

    return dtStr;
}

// Return the total stock balance at a given date
double StockModel::getDailyBalanceAt(const AccountData& account_d, const wxDate& date)
{
    std::map<int64, double> totBalance;

    for (const Data& stock_d : find(
        StockCol::HELDAT(account_d.m_id)
    )) {
        mmDateN precValueDate, nextValueDate;
        StockHistoryModel::DataA sh_a = StockHistoryModel::instance().find(
            StockCol::SYMBOL(stock_d.m_symbol)
        );
        std::stable_sort(sh_a.begin(), sh_a.end(), StockHistoryData::SorterByDATE());
        std::reverse(sh_a.begin(), sh_a.end());

        double valueAtDate = 0.0,  precValue = 0.0, nextValue = 0.0;

        for (const auto & sh_d : sh_a) {
            // test for the date requested
            if (sh_d.m_date == mmDate(date)) {
                valueAtDate = sh_d.m_price;
                break;
            }
            // if not found, search for previous and next date
            if (precValue == 0.0 && sh_d.m_date < mmDate(date)) {
                precValue = sh_d.m_price;
                precValueDate = sh_d.m_date;
            }
            if (sh_d.m_date > mmDate(date)) {
                nextValue = sh_d.m_price;
                nextValueDate = sh_d.m_date;
            }
            // end conditions: prec value assigned and price date < requested date
            if (precValue != 0.0 && sh_d.m_date < mmDate(date))
                break;
        }
        if (valueAtDate == 0.0) {
            //  if previous not found but if the given date is after purchase date, takes purchase price
            if (precValue == 0.0 && date >= PURCHASEDATE(stock_d)) {
                precValue = stock_d.m_purchase_price;
                precValueDate = stock_d.m_purchase_date;
            }
            //  if next not found and the accoung is open, takes previous date
            if (nextValue == 0.0 && account_d.is_open()) {
                nextValue = precValue;
                nextValueDate = precValueDate;
            }
            if (precValue > 0.0 && nextValue > 0.0 &&
                precValueDate.has_value() && precValueDate.value() >= stock_d.m_purchase_date &&
                nextValueDate.has_value() && nextValueDate.value() >= stock_d.m_purchase_date
            )
                valueAtDate = precValue;
        }

        double numShares = 0.0;

        TrxLinkModel::DataA tl_a = TrxLinkModel::TranslinkList<StockModel>(stock_d.m_id);
        for (const auto& tl_d : tl_a) {
            const TrxData* trx_n = TrxModel::instance().get_id_data_n(
                tl_d.CHECKINGACCOUNTID
            );
            if (trx_n &&
                trx_n->m_id > -1 &&
                trx_n->DELETEDTIME.IsEmpty() &&
                mmDate(TrxModel::getTransDateTime(*trx_n)) <= mmDate(date)
            ) {
                numShares += TrxShareModel::instance().unsafe_get_trx_share_n(
                    tl_d.CHECKINGACCOUNTID
                )->m_number;
            }
        }

        if (tl_a.empty() && stock_d.m_purchase_date <= mmDate(date))
            numShares = stock_d.m_num_shares;

        totBalance[stock_d.id()] += numShares * valueAtDate;
    }

    double balance = 0.0;
    for (const auto& it : totBalance)
        balance += it.second;

    return balance;
}

// Returns the realized gain/loss of the stock due to sold shares.
// If the optional parameter to_base_curr = true is passed values are converted
// to base currency.
double StockModel::RealGainLoss(const Data& stock_d, bool to_base_curr)
{
    TrxLinkModel::DataA tl_a = TrxLinkModel::TranslinkList<StockModel>(stock_d.m_id);
    double real_gain_loss = 0;
    double total_shares = 0;
    double total_initial_value = 0;
    double avg_share_price = 0;
    double conv_rate = 1;

    TrxModel::DataA trx_a;
    for (const auto& tl_d : tl_a) {
        const TrxData* trx_d = TrxModel::instance().get_id_data_n(
            tl_d.CHECKINGACCOUNTID
        );
        if (trx_d && trx_d->m_id > -1 && trx_d->DELETEDTIME.IsEmpty())
            trx_a.push_back(*trx_d);
    }
    std::stable_sort(trx_a.begin(), trx_a.end(), TrxData::SorterByTRANSDATE());

    const CurrencyData* currency = to_base_curr ? AccountModel::instance().get_id_currency_p(
                                                    stock_d.m_account_id_n
                                                ) : nullptr; // only retrieve currency data if needed

    for (const auto& trx_d : trx_a) {
        const TrxShareData* ts_n = TrxShareModel::instance().unsafe_get_trx_share_n(
            trx_d.m_id
        );
        conv_rate = to_base_curr
            ? CurrencyHistoryModel::getDayRate(currency->m_id, trx_d.TRANSDATE)
            : 1;
        total_shares += ts_n->m_number;

        if (ts_n->m_number > 0) {
            total_initial_value += (
                ts_n->m_number * ts_n->m_price + ts_n->m_commission
            ) * conv_rate;
        }
        else {
            total_initial_value += ts_n->m_number * avg_share_price;
            real_gain_loss +=
                -ts_n->m_number * (ts_n->m_price * conv_rate - avg_share_price) -
                ts_n->m_commission * conv_rate;
        }

        if (total_shares < 0) total_shares = 0;
        if (total_initial_value < 0) total_initial_value = 0;
        if (total_shares > 0) avg_share_price = total_initial_value / total_shares;
        else avg_share_price = 0;
    }

    return real_gain_loss;
}

// Returns the current unrealized gain/loss.
// If the optional parameter to_base_curr = true is passed values are converted
// to base currency.
double StockModel::UnrealGainLoss(const Data& stock_d, bool to_base_curr)
{
    if (!to_base_curr)
        return CurrentValue(stock_d) - InvestmentValue(stock_d);

    const CurrencyData* currency_n = AccountModel::instance().get_id_currency_p(
        stock_d.m_account_id_n
    );
    double conv_rate = CurrencyHistoryModel::getDayRate(currency_n->m_id);
    TrxLinkModel::DataA tl_a = TrxLinkModel::TranslinkList<StockModel>(stock_d.m_id);
    if (!tl_a.empty()) {
        double total_shares = 0;
        double total_initial_value = 0;
        double avg_share_price = 0;

        TrxModel::DataA trx_a;
        for (const auto& tl_d : tl_a) {
            const TrxData* trx_d = TrxModel::instance().get_id_data_n(
                tl_d.CHECKINGACCOUNTID
            );
            if (trx_d && trx_d->m_id > -1 && trx_d->DELETEDTIME.IsEmpty())
                trx_a.push_back(*trx_d);
        }
        std::stable_sort(trx_a.begin(), trx_a.end(),
            TrxData::SorterByTRANSDATE()
        );

        for (const auto& trx_d : trx_a) {
            const TrxShareData* ts_d = TrxShareModel::instance().unsafe_get_trx_share_n(
                trx_d.m_id
            );
            conv_rate = CurrencyHistoryModel::getDayRate(currency_n->m_id, trx_d.TRANSDATE);
            total_shares += ts_d->m_number;
            if (total_shares < 0) total_shares = 0;

            if (ts_d->m_number > 0) {
                total_initial_value += (
                    ts_d->m_number * ts_d->m_price + ts_d->m_commission
                ) * conv_rate;
            }
            else {
                total_initial_value += ts_d->m_number * avg_share_price;
            }

            if (total_initial_value < 0) total_initial_value = 0;
            if (total_shares > 0) avg_share_price = total_initial_value / total_shares;
        }
        conv_rate = CurrencyHistoryModel::getDayRate(currency_n->m_id);
        return CurrentValue(stock_d) * conv_rate - total_initial_value;
    }
    else {
        return (CurrentValue(stock_d) - InvestmentValue(stock_d)) * conv_rate;
    }
}

/** Updates the current price across all accounts which hold the stock */
void StockModel::UpdateCurrentPrice(const wxString& symbol, const double price)
{
    double current_price = price;
    if (price == -1) {
        StockHistoryModel::DataA sh_a = StockHistoryModel::instance().find(
            StockHistoryCol::SYMBOL(symbol)
        );
        if (!sh_a.empty()) {
            std::sort(sh_a.begin(), sh_a.end(), StockHistoryData::SorterByDATE());
            current_price = sh_a.back().m_price;
        }
    }
    if (current_price != -1) {
        StockModel::DataA stock_a = StockModel::instance().find(
            StockCol::SYMBOL(symbol)
        );
        for (auto& stock_d : stock_a) {
            // CHECK: use stock_d directly
            StockData* stock_n = StockModel::instance().unsafe_get_id_data_n(stock_d.m_id);
            stock_n->m_current_price = current_price;
            StockModel::instance().unsafe_update_data_n(stock_n);
        }
    }
}

void StockModel::UpdatePosition(StockData* stock_n)
{
    TrxLinkModel::DataA tl_a = TrxLinkModel::TranslinkList<StockModel>(stock_n->m_id);
    double total_shares = 0;
    double total_initial_value = 0;
    double total_commission = 0;
    double avg_share_price = 0;
    wxString earliest_date = wxDate::Today().FormatISODate();
    TrxModel::DataA trx_a;
    for (const auto& tl_d : tl_a) {
        const TrxData* trx_n = TrxModel::instance().get_id_data_n(tl_d.CHECKINGACCOUNTID);
        if (trx_n && trx_n->m_id > -1 && trx_n->DELETEDTIME.IsEmpty() &&
            TrxModel::status_id(trx_n->STATUS) != TrxModel::STATUS_ID_VOID
        ) {
            trx_a.push_back(*trx_n);
        }
    }
    std::stable_sort(trx_a.begin(), trx_a.end(), TrxData::SorterByTRANSDATE());
    for (const auto& trx_d : trx_a) {
        const TrxShareData* ts_n = TrxShareModel::instance().unsafe_get_trx_share_n(
            trx_d.m_id
        );

        total_shares += ts_n->m_number;
        if (total_shares < 0)
            total_shares = 0;

        if (ts_n->m_number > 0)
            total_initial_value += ts_n->m_number * ts_n->m_price + ts_n->m_commission;
        else
            total_initial_value += ts_n->m_number * avg_share_price;

        if (total_initial_value < 0)
            total_initial_value = 0;
        if (total_shares > 0)
            avg_share_price = total_initial_value / total_shares;

        total_commission += ts_n->m_commission;

        wxString transdate = trx_d.TRANSDATE;
        if (transdate < earliest_date)
            earliest_date = transdate;
    }

    // The stock record contains the total of share transactions.
    if (tl_a.empty()) {
        stock_n->m_purchase_price = stock_n->m_current_price;
    }
    else {
        wxDateTime purchasedate;
        purchasedate.ParseDateTime(earliest_date) || purchasedate.ParseDate(earliest_date);
        stock_n->m_purchase_date  = mmDate(purchasedate);
        stock_n->m_purchase_price = avg_share_price;
        stock_n->m_num_shares     = total_shares;
        stock_n->m_purchase_value = total_initial_value;
        stock_n->m_commission     = total_commission;
    }
    StockModel::instance().unsafe_save_data_n(stock_n);
}
