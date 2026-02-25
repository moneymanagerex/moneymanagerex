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
#include "TransactionLinkModel.h"
#include "TransactionShareModel.h"
#include "CurrencyHistoryModel.h"

StockModel::StockModel()
: Model<StockTable>()
{
}

StockModel::~StockModel()
{
}

/**
* Initialize the global StockModel table.
* Reset the StockModel table or create the table if it does not exist.
*/
StockModel& StockModel::instance(wxSQLite3Database* db)
{
    StockModel& ins = Singleton<StockModel>::instance();
    ins.m_db = db;
    ins.destroy_cache();
    ins.ensure_table();

    return ins;
}

wxString StockModel::get_stock_name(int64 stock_id)
{
    Data* stock = instance().get_id(stock_id);
    if (stock)
        return stock->STOCKNAME;
    else
        return _t("Stock Error");
}

/** Return the static instance of StockModel table */
StockModel& StockModel::instance()
{
    return Singleton<StockModel>::instance();
}

wxDate StockModel::PURCHASEDATE(const Data* stock)
{
    return parseDateTime(stock->PURCHASEDATE);
}

wxDate StockModel::PURCHASEDATE(const Data& stock)
{
    return parseDateTime(stock.PURCHASEDATE);
}

/** Original value of Stocks */
double StockModel::InvestmentValue(const Data* r)
{
    return r->VALUE;
}

/** Original value of Stocks */
double StockModel::InvestmentValue(const Data& r)
{
    return InvestmentValue(&r);
}

double StockModel::CurrentValue(const Data* r)
{
    return r->NUMSHARES * r->CURRENTPRICE;
}

double StockModel::CurrentValue(const Data& r)
{
    return CurrentValue(&r);
}

/**
* Remove the Data record from memory and the database.
* Delete also all stock history
*/
bool StockModel::remove(const int64 id)
{
    StockModel::Data *data = this->get_id(id);
    const auto &stocks = StockModel::instance().find(StockModel::SYMBOL(data->SYMBOL));
    if (stocks.size() == 1)
    {
        this->Savepoint();
        for (const auto& r : StockHistoryModel::instance().find(StockHistoryModel::SYMBOL(data->SYMBOL)))
            StockHistoryModel::instance().remove(r.id());
        this->ReleaseSavepoint();
    }

    return Model<StockTable>::remove(id);
}

/**
Returns the last price date of a given stock
*/
wxString StockModel::lastPriceDate(const Data* entity)
{
    wxString dtStr = entity->PURCHASEDATE;
    StockHistoryModel::Data_Set histData = StockHistoryModel::instance().find(SYMBOL(entity->SYMBOL));

    std::sort(histData.begin(), histData.end(), StockHistoryRow::SorterByDATE());
    if (!histData.empty())
        dtStr = histData.back().DATE;

    return dtStr;
}

/**
Returns the total stock balance at a given date
*/
double StockModel::getDailyBalanceAt(const AccountModel::Data *account, const wxDate& date)
{
    wxString strDate = date.FormatISODate();
    std::map<int64, double> totBalance;

    Data_Set stocks = this->instance().find(HELDAT(account->id()));
    for (const auto & stock : stocks)
    {
        wxString precValueDate, nextValueDate;
        StockHistoryModel::Data_Set stock_hist = StockHistoryModel::instance().find(SYMBOL(stock.SYMBOL));
        std::stable_sort(stock_hist.begin(), stock_hist.end(), StockHistoryRow::SorterByDATE());
        std::reverse(stock_hist.begin(), stock_hist.end());

        double valueAtDate = 0.0,  precValue = 0.0, nextValue = 0.0;

        for (const auto & hist : stock_hist)
        {
            // test for the date requested
            if (hist.DATE == strDate)
            {
                valueAtDate = hist.VALUE;
                break;
            }
            // if not found, search for previous and next date
            if (precValue == 0.0 && hist.DATE < strDate)
            {
                precValue = hist.VALUE;
                precValueDate = hist.DATE;
            }
            if (hist.DATE > strDate)
            {
                nextValue = hist.VALUE;
                nextValueDate = hist.DATE;
            }
            // end conditions: prec value assigned and price date < requested date
            if (precValue != 0.0 && hist.DATE < strDate)
                break;
        }
        if (valueAtDate == 0.0)
        {
            //  if previous not found but if the given date is after purchase date, takes purchase price
            if (precValue == 0.0 && date >= PURCHASEDATE(stock))
            {
                precValue = stock.PURCHASEPRICE;
                precValueDate = stock.PURCHASEDATE;
            }
            //  if next not found and the accoung is open, takes previous date
            if (nextValue == 0.0 && AccountModel::status_id(account) == AccountModel::STATUS_ID_OPEN)
            {
                nextValue = precValue;
                nextValueDate = precValueDate;
            }
            if (precValue > 0.0 && nextValue > 0.0 && precValueDate >= stock.PURCHASEDATE && nextValueDate >= stock.PURCHASEDATE)
                valueAtDate = precValue;
        }

        double numShares = 0.0;

        TransactionLinkModel::Data_Set linkrecords = TransactionLinkModel::TranslinkList<StockModel>(stock.STOCKID);
        for (const auto& linkrecord : linkrecords)
        {
            TransactionModel::Data* txn = TransactionModel::instance().get_id(linkrecord.CHECKINGACCOUNTID);
            if (txn->TRANSID > -1 && txn->DELETEDTIME.IsEmpty() && TransactionModel::getTransDateTime(txn).FormatISODate() <= strDate) {
                numShares += TransactionShareModel::instance().ShareEntry(linkrecord.CHECKINGACCOUNTID)->SHARENUMBER;
            }
        }

        if (linkrecords.empty() && stock.PURCHASEDATE <= strDate)
            numShares = stock.NUMSHARES;

        totBalance[stock.id()] += numShares * valueAtDate;
    }

    double balance = 0.0;
    for (const auto& it : totBalance)
        balance += it.second;

    return balance;
}

/**
Returns the realized gain/loss of the stock due to sold shares.
If the optional parameter to_base_curr = true is passed values are converted
to base currency.
*/
double StockModel::RealGainLoss(const Data* r, bool to_base_curr)
{
    CurrencyModel::Data* currency = AccountModel::currency(AccountModel::instance().get_id(r->HELDAT));
    TransactionLinkModel::Data_Set trans_list = TransactionLinkModel::TranslinkList<StockModel>(r->STOCKID);
    double real_gain_loss = 0;
    double total_shares = 0;
    double total_initial_value = 0;
    double avg_share_price = 0;
    double conv_rate = 1;

    TransactionModel::Data_Set checking_list;
    for (const auto &trans : trans_list)
    {
        TransactionModel::Data* checking_entry = TransactionModel::instance().get_id(trans.CHECKINGACCOUNTID);
        if (checking_entry->TRANSID > -1 && checking_entry->DELETEDTIME.IsEmpty()) checking_list.push_back(*checking_entry);
    }
    std::stable_sort(checking_list.begin(), checking_list.end(), TransactionRow::SorterByTRANSDATE());

    for (const auto &trans : checking_list)
    {
        TransactionShareModel::Data* share_entry = TransactionShareModel::ShareEntry(trans.TRANSID);
        conv_rate = to_base_curr ? CurrencyHistoryModel::getDayRate(currency->CURRENCYID, trans.TRANSDATE) : 1;
        total_shares += share_entry->SHARENUMBER;

        if (share_entry->SHARENUMBER > 0) {
            total_initial_value += (share_entry->SHARENUMBER * share_entry->SHAREPRICE + share_entry->SHARECOMMISSION) * conv_rate;
        }
        else {
            total_initial_value += share_entry->SHARENUMBER * avg_share_price;
            real_gain_loss += -share_entry->SHARENUMBER * (share_entry->SHAREPRICE * conv_rate - avg_share_price) - share_entry->SHARECOMMISSION * conv_rate;
        }

        if (total_shares < 0) total_shares = 0;
        if (total_initial_value < 0) total_initial_value = 0;
        if (total_shares > 0) avg_share_price = total_initial_value / total_shares;
        else avg_share_price = 0;
    }

    return real_gain_loss;
}

/**
Returns the realized gain/loss of the stock due to sold shares.
If the optional parameter to_base_curr = true is passed values are converted
to base currency.
*/
double StockModel::RealGainLoss(const Data& r, bool to_base_curr)
{
    return RealGainLoss(&r, to_base_curr);
}

/**
Returns the current unrealized gain/loss.
If the optional parameter to_base_curr = true is passed values are converted
to base currency.
*/
double StockModel::UnrealGainLoss(const Data& r, bool to_base_curr)
{
    return UnrealGainLoss(&r, to_base_curr);
}

/**
Returns the current unrealized gain/loss.
If the optional parameter to_base_curr = true is passed values are converted
to base currency.
*/
double StockModel::UnrealGainLoss(const Data* r, bool to_base_curr)
{
    if (!to_base_curr)
        return CurrentValue(r) - InvestmentValue(r);
    else
    {
        CurrencyModel::Data* currency = AccountModel::currency(AccountModel::instance().get_id(r->HELDAT));
        double conv_rate = CurrencyHistoryModel::getDayRate(currency->CURRENCYID);
        TransactionLinkModel::Data_Set trans_list = TransactionLinkModel::TranslinkList<StockModel>(r->STOCKID);
        if (!trans_list.empty())
        {
            double total_shares = 0;
            double total_initial_value = 0;
            double avg_share_price = 0;

            TransactionModel::Data_Set checking_list;
            for (const auto &trans : trans_list)
            {
                TransactionModel::Data* checking_entry = TransactionModel::instance().get_id(trans.CHECKINGACCOUNTID);
                if (checking_entry->TRANSID > -1 && checking_entry->DELETEDTIME.IsEmpty()) checking_list.push_back(*checking_entry);
            }
            std::stable_sort(checking_list.begin(), checking_list.end(), TransactionRow::SorterByTRANSDATE());

            for (const auto &trans : checking_list)
            {
                TransactionShareModel::Data* share_entry = TransactionShareModel::ShareEntry(trans.TRANSID);
                conv_rate = CurrencyHistoryModel::getDayRate(currency->CURRENCYID, trans.TRANSDATE);
                total_shares += share_entry->SHARENUMBER;
                if (total_shares < 0) total_shares = 0;

                if (share_entry->SHARENUMBER > 0) {
                    total_initial_value += (share_entry->SHARENUMBER * share_entry->SHAREPRICE + share_entry->SHARECOMMISSION) * conv_rate;
                }
                else {
                    total_initial_value += share_entry->SHARENUMBER * avg_share_price;
                }

                if (total_initial_value < 0) total_initial_value = 0;
                if (total_shares > 0) avg_share_price = total_initial_value / total_shares;
            }
            conv_rate = CurrencyHistoryModel::getDayRate(currency->CURRENCYID);
            return CurrentValue(r) * conv_rate - total_initial_value;
        }
        else {
            return (CurrentValue(r) - InvestmentValue(r)) * conv_rate;
        }
    }
}

/** Updates the current price across all accounts which hold the stock */
void StockModel::UpdateCurrentPrice(const wxString& symbol, const double price)
{
    double current_price = price;
    if (price == -1) {
        StockHistoryModel::Data_Set histData = StockHistoryModel::instance().find(StockHistoryModel::SYMBOL(symbol));
        if (!histData.empty())
        {
            std::sort(histData.begin(), histData.end(), StockHistoryRow::SorterByDATE());
            current_price = histData.back().VALUE;
        }
    }
    if (current_price != -1)
    {
        StockModel::Data_Set stocks = StockModel::instance().find(StockModel::SYMBOL(symbol));
        for (auto& stock : stocks) {
            StockModel::Data* stockRecord = StockModel::instance().get_id(stock.STOCKID);
            stockRecord->CURRENTPRICE = current_price;
            StockModel::instance().save(stockRecord);
        }
    }
}

void StockModel::UpdatePosition(StockModel::Data* stock_entry)
{
    TransactionLinkModel::Data_Set trans_list = TransactionLinkModel::TranslinkList<StockModel>(stock_entry->STOCKID);
    double total_shares = 0;
    double total_initial_value = 0;
    double total_commission = 0;
    double avg_share_price = 0;
    wxString earliest_date = wxDate::Today().FormatISODate();
    TransactionModel::Data_Set checking_list;
    for (const auto &trans : trans_list)
    {
        TransactionModel::Data* checking_entry = TransactionModel::instance().get_id(trans.CHECKINGACCOUNTID);
        if (checking_entry->TRANSID > -1 && checking_entry->DELETEDTIME.IsEmpty() && TransactionModel::status_id(checking_entry->STATUS) != TransactionModel::STATUS_ID_VOID)
            checking_list.push_back(*checking_entry);
    }
    std::stable_sort(checking_list.begin(), checking_list.end(), TransactionRow::SorterByTRANSDATE());
    for (const auto &trans : checking_list)
    {
        TransactionShareModel::Data* share_entry = TransactionShareModel::ShareEntry(trans.TRANSID);

        total_shares += share_entry->SHARENUMBER;
        if (total_shares < 0)
            total_shares = 0;

        if (share_entry->SHARENUMBER > 0)
            total_initial_value += share_entry->SHARENUMBER * share_entry->SHAREPRICE + share_entry->SHARECOMMISSION;
        else
            total_initial_value += share_entry->SHARENUMBER * avg_share_price;

        if (total_initial_value < 0)
            total_initial_value = 0;
        if (total_shares > 0)
            avg_share_price = total_initial_value / total_shares;

        total_commission += share_entry->SHARECOMMISSION;

        wxString transdate = trans.TRANSDATE;
        if (transdate < earliest_date)
            earliest_date = transdate;
    }

    // The stock record contains the total of share transactions.
    if (trans_list.empty())
    {
        stock_entry->PURCHASEPRICE = stock_entry->CURRENTPRICE;
    }
    else
    {
        wxDateTime purchasedate;
        purchasedate.ParseDateTime(earliest_date) || purchasedate.ParseDate(earliest_date);
        stock_entry->PURCHASEDATE = purchasedate.FormatISODate();
        stock_entry->PURCHASEPRICE = avg_share_price;
        stock_entry->NUMSHARES = total_shares;
        stock_entry->VALUE = total_initial_value;
        stock_entry->COMMISSION = total_commission;
    }
    StockModel::instance().save(stock_entry);
}
