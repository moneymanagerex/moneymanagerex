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

const RefTypeN StockModel::s_ref_type = RefTypeN(RefTypeN::e_stock);

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

// Return the static instance of StockModel table
StockModel& StockModel::instance()
{
    return Singleton<StockModel>::instance();
}

// Remove the Data record from memory and the database.
// Delete also all stock history
bool StockModel::purge_id(int64 id)
{
    const StockData *stock_n = get_id_data_n(id);
    const auto& stock_a = find(
        StockCol::SYMBOL(stock_n->m_symbol)
    );
    if (stock_a.size() == 1) {
        db_savepoint();
        for (const auto& sh_d : StockHistoryModel::instance().find(
            StockHistoryCol::SYMBOL(stock_n->m_symbol)
        ))
            StockHistoryModel::instance().purge_id(sh_d.m_id);
        db_release_savepoint();
    }

    // FIXME: remove AttachmentData owned by id

    return unsafe_remove_id(id);
}

const wxString StockModel::get_id_name(int64 stock_id)
{
    const Data* stock_n = instance().get_id_data_n(stock_id);
    return stock_n ? stock_n->m_name : _t("Stock Error");
}

// Return the last price date of a given stock
const mmDate StockModel::find_last_hist_date(const Data& stock_d)
{
    mmDate date = stock_d.m_purchase_date;
    StockHistoryModel::DataA sh_a = StockHistoryModel::instance().find(
        StockCol::SYMBOL(stock_d.m_symbol)
    );

    std::sort(sh_a.begin(), sh_a.end(), StockHistoryData::SorterByDATE());
    if (!sh_a.empty())
        date = sh_a.back().m_date;

    return date;
}

// Return the total stock balance at a given date
double StockModel::calculate_account_balance(const AccountData& account_d, const mmDate& date)
{
    double balance = 0.0;

    for (const Data& stock_d : find(
        StockCol::HELDAT(account_d.m_id)
    )) {
        StockHistoryModel::DataA sh_a = StockHistoryModel::instance().find(
            StockCol::SYMBOL(stock_d.m_symbol)
        );
        std::stable_sort(sh_a.begin(), sh_a.end(), StockHistoryData::SorterByDATE());
        std::reverse(sh_a.begin(), sh_a.end());

        mmDateN prev_date; double prev_price = 0.0;
        mmDateN next_date; //double next_price = 0.0;
        for (const StockHistoryData& sh_d : sh_a) {
            // stop if the exact date is found
            if (sh_d.m_date == date) {
                prev_date = sh_d.m_date; prev_price = sh_d.m_price;
                next_date = sh_d.m_date; //next_price = sh_d.m_price;
                break;
            }
            // stop at the first past date
            if (sh_d.m_date < date) {
                prev_date = sh_d.m_date; prev_price = sh_d.m_price;
                break;
            }
            // scan all future dates
            if (sh_d.m_date > date) {
                next_date = sh_d.m_date; //next_price = sh_d.m_price;
            }
        }
        // if no previous date is found, fallback to purchase date and price
        if (!prev_date.has_value() && stock_d.m_purchase_date <= date) {
            prev_date = stock_d.m_purchase_date;
            prev_price = stock_d.m_purchase_price;
        }
        //  if no next date is found and the account is open, fallback to previous
        if (!next_date.has_value() && account_d.is_open()) {
            next_date = prev_date; //next_price = prev_price;
        }
        // if previous and next date is still not found, skip this stock
        if (!prev_date.has_value() || prev_date.value() < stock_d.m_purchase_date ||
            !next_date.has_value() || next_date.value() < stock_d.m_purchase_date
        ) {
            continue;
        }
        // take the previous price
        double price = prev_price;

        double num_shares = 0.0;
        TrxLinkModel::DataA tl_a = TrxLinkModel::instance().find_ref_data_a(
            s_ref_type, stock_d.m_id
        );
        for (const TrxLinkModel::Data& tl_d : tl_a) {
            const TrxData* trx_n = TrxModel::instance().get_id_data_n(
                tl_d.m_trx_id
            );
            if (trx_n && trx_n->m_id > 0 &&
                // CHECK: ignore Void transactions
                !trx_n->is_deleted() &&
                trx_n->m_date() <= date
            ) {
                const TrxShareData* ts_n = TrxShareModel::instance().get_trxId_data_n(
                    tl_d.m_trx_id
                );
                if (ts_n)
                    num_shares += ts_n->m_number;
            }
        }
        if (tl_a.empty() && stock_d.m_purchase_date <= date)
            num_shares = stock_d.m_num_shares;

        balance += num_shares * price;
    }

    return balance;
}

// Returns the realized gain/loss of the stock due to sold shares.
// If the optional parameter to_base_curr = true is passed values are converted
// to base currency.
double StockModel::calculate_realized_gain(const Data& stock_d, bool to_base_curr)
{
    // retrieve currency data only if needed
    const CurrencyData* currency_n = nullptr;
    if (to_base_curr)
        currency_n = AccountModel::instance().get_id_currency_p(stock_d.m_account_id_n);

    double realized_gain = 0;  // positive means gain, negative means loss
    double total_shares = 0;
    double total_initial_value = 0;
    double avg_share_price = 0;
    double conv_rate = 1;

    TrxModel::DataA trx_a;
    for (const auto& tl_d : TrxLinkModel::instance().find_ref_data_a(
        s_ref_type, stock_d.m_id
    )) {
        const TrxData* trx_n = TrxModel::instance().get_id_data_n(
            tl_d.m_trx_id
        );
        if (trx_n && trx_n->m_id > 0 &&
            // FIXME: ignore void transactions
            !trx_n->is_deleted()
        )
            trx_a.push_back(*trx_n);
    }
    std::stable_sort(trx_a.begin(), trx_a.end(), TrxData::SorterByDateTime());

    for (const TrxData& trx_d : trx_a) {
        const TrxShareData* ts_n = TrxShareModel::instance().get_trxId_data_n(
            trx_d.m_id
        );
        if (!ts_n)
            continue;

        // only long positions are supported
        // TODO: support short positions
        total_shares += ts_n->m_number;
        if (total_shares < 0)
            total_shares = 0;

        if (to_base_curr) {
            conv_rate = CurrencyHistoryModel::instance().get_id_date_rate(
                currency_n->m_id,
                trx_d.m_date()
            );
        }
        if (ts_n->m_number > 0) {
            // BUY
            total_initial_value += (
                ts_n->m_number * ts_n->m_price + ts_n->m_commission
            ) * conv_rate;
        }
        else {
            // SELL
            total_initial_value += ts_n->m_number * avg_share_price;
            realized_gain +=
                (-ts_n->m_number) * (ts_n->m_price * conv_rate - avg_share_price) -
                ts_n->m_commission * conv_rate;
        }

        if (total_initial_value < 0)
            total_initial_value = 0;

        avg_share_price = (total_shares > 0)
            ? total_initial_value / total_shares
            : 0.0;
    }

    return realized_gain;
}

// Returns the current unrealized gain/loss.
// If the optional parameter to_base_curr = true is passed values are converted
// to base currency.
double StockModel::calculate_unrealiazed_gain(const Data& stock_d, bool to_base_curr)
{
    if (!to_base_curr)
        return stock_d.current_value() - stock_d.m_purchase_value;

    const CurrencyData* currency_n = AccountModel::instance().get_id_currency_p(
        stock_d.m_account_id_n
    );
    double conv_rate = CurrencyHistoryModel::instance().get_id_date_rate(currency_n->m_id);
    TrxLinkModel::DataA tl_a = TrxLinkModel::instance().find_ref_data_a(
        s_ref_type, stock_d.m_id
    );
    if (!tl_a.empty()) {
        double total_shares = 0;
        double total_initial_value = 0;
        double avg_share_price = 0;

        TrxModel::DataA trx_a;
        for (const auto& tl_d : tl_a) {
            const TrxData* trx_d = TrxModel::instance().get_id_data_n(
                tl_d.m_trx_id
            );
            if (trx_d && trx_d->m_id > 0 &&
                // FIXME: ignore void transactions
                !trx_d->is_deleted()
            )
                trx_a.push_back(*trx_d);
        }
        std::stable_sort(trx_a.begin(), trx_a.end(),
            TrxData::SorterByDateTime()
        );

        for (const auto& trx_d : trx_a) {
            const TrxShareData* ts_d = TrxShareModel::instance().get_trxId_data_n(
                trx_d.m_id
            );
            conv_rate = CurrencyHistoryModel::instance().get_id_date_rate(
                currency_n->m_id,
                trx_d.m_date()
            );
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
        conv_rate = CurrencyHistoryModel::instance().get_id_date_rate(currency_n->m_id);
        return stock_d.current_value() * conv_rate - total_initial_value;
    }
    else {
        return (stock_d.current_value() - stock_d.m_purchase_value) * conv_rate;
    }
}

// Updates the current price across all accounts which hold the stock
// TODO: use std::optional<double> price
void StockModel::update_symbol_current_price(const wxString& symbol, double price)
{
    double current_price = price;
    if (current_price == -1) {
        StockHistoryModel::DataA sh_a = StockHistoryModel::instance().find(
            StockHistoryCol::SYMBOL(symbol)
        );
        if (!sh_a.empty()) {
            std::sort(sh_a.begin(), sh_a.end(), StockHistoryData::SorterByDATE());
            current_price = sh_a.back().m_price;
        }
    }
    if (current_price == -1)
        return;

    for (const Data& stock_d : find(
        StockCol::SYMBOL(symbol)
    )) {
        // TODO: use stock_d directly
        StockData* stock_n = unsafe_get_id_data_n(stock_d.m_id);
        stock_n->m_current_price = current_price;
        unsafe_update_data_n(stock_n);
    }
}

// stock_entry.m_purchase_price = avg price of shares purchased.
// stock_entry.m_num_shares = total amount of shares purchased.
// stock_entry.VALUE     = value of shares based on:
// ... share_entry.SHARENUMBER * share_entry.SHAREPRICE
void StockModel::update_data_position(StockData* stock_n)
{
    TrxLinkModel::DataA tl_a = TrxLinkModel::instance().find_ref_data_a(
        s_ref_type, stock_n->m_id
    );
    double total_shares = 0;
    double total_initial_value = 0;
    double total_commission = 0;
    double avg_share_price = 0;
    mmDate min_trx_date = mmDate::today();
    TrxModel::DataA trx_a;
    for (const auto& tl_d : tl_a) {
        const TrxData* trx_n = TrxModel::instance().get_id_data_n(tl_d.m_trx_id);
        if (trx_n && trx_n->m_id > 0 && trx_n->is_valid()) {
            trx_a.push_back(*trx_n);
        }
    }
    std::stable_sort(trx_a.begin(), trx_a.end(), TrxData::SorterByDateTime());
    for (const auto& trx_d : trx_a) {
        const TrxShareData* ts_n = TrxShareModel::instance().get_trxId_data_n(
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

        mmDate trx_date = trx_d.m_date();
        if (trx_date < min_trx_date)
            min_trx_date = trx_date;
    }

    // The stock record contains the total of share transactions.
    if (tl_a.empty()) {
        stock_n->m_purchase_price = stock_n->m_current_price;
    }
    else {
        //wxDateTime purchasedate;
        stock_n->m_purchase_date  = min_trx_date;
        stock_n->m_purchase_price = avg_share_price;
        stock_n->m_num_shares     = total_shares;
        stock_n->m_purchase_value = total_initial_value;
        stock_n->m_commission     = total_commission;
    }
    unsafe_save_data_n(stock_n);
}
