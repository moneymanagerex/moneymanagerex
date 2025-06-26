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

#include "Model_Stock.h"
#include "Model_StockHistory.h"
#include "Model_Translink.h"
#include "Model_Shareinfo.h"
#include "Model_CurrencyHistory.h"

Model_Stock::Model_Stock()
: Model<DB_Table_STOCK_V1>()
{
}

Model_Stock::~Model_Stock()
{
}

/**
* Initialize the global Model_Stock table.
* Reset the Model_Stock table or create the table if it does not exist.
*/
Model_Stock& Model_Stock::instance(wxSQLite3Database* db)
{
    Model_Stock& ins = Singleton<Model_Stock>::instance();
    ins.db_ = db;
    ins.destroy_cache();
    ins.ensure(db);

    return ins;
}

wxString Model_Stock::get_stock_name(int64 stock_id)
{
    Data* stock = instance().get(stock_id);
    if (stock)
        return stock->STOCKNAME;
    else
        return _t("Stock Error");
}

/** Return the static instance of Model_Stock table */
Model_Stock& Model_Stock::instance()
{
    return Singleton<Model_Stock>::instance();
}

wxDate Model_Stock::PURCHASEDATE(const Data* stock)
{
    return Model::to_date(stock->PURCHASEDATE);
}

wxDate Model_Stock::PURCHASEDATE(const Data& stock)
{
    return Model::to_date(stock.PURCHASEDATE);
}

/** Original value of Stocks */
double Model_Stock::InvestmentValue(const Data* r)
{
    return r->VALUE;
}

/** Original value of Stocks */
double Model_Stock::InvestmentValue(const Data& r)
{
    return InvestmentValue(&r);
}

double Model_Stock::CurrentValue(const Data* r)
{
    return r->NUMSHARES * r->CURRENTPRICE;
}

double Model_Stock::CurrentValue(const Data& r)
{
    return CurrentValue(&r);
}

/**
* Remove the Data record from memory and the database.
* Delete also all stock history
*/
bool Model_Stock::remove(int64 id)
{
    Model_Stock::Data *data = this->get(id);
    const auto &stocks = Model_Stock::instance().find(Model_Stock::SYMBOL(data->SYMBOL));
    if (stocks.size() == 1)
    {
        this->Savepoint();
        for (const auto& r : Model_StockHistory::instance().find(Model_StockHistory::SYMBOL(data->SYMBOL)))
            Model_StockHistory::instance().remove(r.id());
        this->ReleaseSavepoint();
    }

    return this->remove(id, db_);
}

/**
Returns the last price date of a given stock
*/
wxString Model_Stock::lastPriceDate(const Self::Data* entity)
{
    wxString dtStr = entity->PURCHASEDATE;
    Model_StockHistory::Data_Set histData = Model_StockHistory::instance().find(SYMBOL(entity->SYMBOL));

    std::sort(histData.begin(), histData.end(), SorterByDATE());
    if (!histData.empty())
        dtStr = histData.back().DATE;

    return dtStr;
}

/**
Returns the total stock balance at a given date
*/
double Model_Stock::getDailyBalanceAt(const Model_Account::Data *account, const wxDate& date)
{
    wxString strDate = date.FormatISODate();
    std::map<int64, double> totBalance;

    Data_Set stocks = this->instance().find(HELDAT(account->id()));
    for (const auto & stock : stocks)
    {
        wxString precValueDate, nextValueDate;
        Model_StockHistory::Data_Set stock_hist = Model_StockHistory::instance().find(SYMBOL(stock.SYMBOL));
        std::stable_sort(stock_hist.begin(), stock_hist.end(), SorterByDATE());
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
            if (nextValue == 0.0 && Model_Account::status_id(account) == Model_Account::STATUS_ID_OPEN)
            {
                nextValue = precValue;
                nextValueDate = precValueDate;
            }
            if (precValue > 0.0 && nextValue > 0.0 && precValueDate >= stock.PURCHASEDATE && nextValueDate >= stock.PURCHASEDATE)
                valueAtDate = precValue;
        }

        double numShares = 0.0;

        Model_Translink::Data_Set linkrecords = Model_Translink::TranslinkList<Model_Stock>(stock.STOCKID);
        for (const auto& linkrecord : linkrecords)
        {
            Model_Checking::Data* txn = Model_Checking::instance().get(linkrecord.CHECKINGACCOUNTID);
            if (txn->DELETEDTIME.IsEmpty() && Model_Checking::TRANSDATE(txn).FormatISODate() <= strDate) {
                numShares += Model_Shareinfo::instance().ShareEntry(linkrecord.CHECKINGACCOUNTID)->SHARENUMBER;
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
double Model_Stock::RealGainLoss(const Data* r, bool to_base_curr)
{
    Model_Currency::Data* currency = Model_Account::currency(Model_Account::instance().get(r->HELDAT));
    Model_Translink::Data_Set trans_list = Model_Translink::TranslinkList<Model_Stock>(r->STOCKID);
    double real_gain_loss = 0;
    double total_shares = 0;
    double total_initial_value = 0;
    double avg_share_price = 0;
    double conv_rate = 1;

    Model_Checking::Data_Set checking_list;
    for (const auto &trans : trans_list)
    {
        Model_Checking::Data* checking_entry = Model_Checking::instance().get(trans.CHECKINGACCOUNTID);
        if (checking_entry && checking_entry->DELETEDTIME.IsEmpty()) checking_list.push_back(*checking_entry);
    }
    std::stable_sort(checking_list.begin(), checking_list.end(), SorterByTRANSDATE());

    for (const auto &trans : checking_list)
    {
        Model_Shareinfo::Data* share_entry = Model_Shareinfo::ShareEntry(trans.TRANSID);
        conv_rate = to_base_curr ? Model_CurrencyHistory::getDayRate(currency->CURRENCYID, trans.TRANSDATE) : 1;
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
double Model_Stock::RealGainLoss(const Data& r, bool to_base_curr)
{
    return RealGainLoss(&r, to_base_curr);
}

/**
Returns the current unrealized gain/loss.
If the optional parameter to_base_curr = true is passed values are converted
to base currency.
*/
double Model_Stock::UnrealGainLoss(const Data& r, bool to_base_curr)
{
    return UnrealGainLoss(&r, to_base_curr);
}

/**
Returns the current unrealized gain/loss.
If the optional parameter to_base_curr = true is passed values are converted
to base currency.
*/
double Model_Stock::UnrealGainLoss(const Data* r, bool to_base_curr)
{
    if (!to_base_curr)
        return CurrentValue(r) - InvestmentValue(r);
    else
    {
        Model_Currency::Data* currency = Model_Account::currency(Model_Account::instance().get(r->HELDAT));
        double conv_rate = Model_CurrencyHistory::getDayRate(currency->CURRENCYID);
        Model_Translink::Data_Set trans_list = Model_Translink::TranslinkList<Model_Stock>(r->STOCKID);
        if (!trans_list.empty())
        {
            double total_shares = 0;
            double total_initial_value = 0;
            double avg_share_price = 0;

            Model_Checking::Data_Set checking_list;
            for (const auto &trans : trans_list)
            {
                Model_Checking::Data* checking_entry = Model_Checking::instance().get(trans.CHECKINGACCOUNTID);
                if (checking_entry && checking_entry->DELETEDTIME.IsEmpty()) checking_list.push_back(*checking_entry);
            }
            std::stable_sort(checking_list.begin(), checking_list.end(), SorterByTRANSDATE());

            for (const auto &trans : checking_list)
            {
                Model_Shareinfo::Data* share_entry = Model_Shareinfo::ShareEntry(trans.TRANSID);
                conv_rate = Model_CurrencyHistory::getDayRate(currency->CURRENCYID, trans.TRANSDATE);
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
            conv_rate = Model_CurrencyHistory::getDayRate(currency->CURRENCYID);
            return CurrentValue(r) * conv_rate - total_initial_value;
        }
        else {
            return (CurrentValue(r) - InvestmentValue(r)) * conv_rate;
        }
    }
}

/** Updates the current price across all accounts which hold the stock */
void Model_Stock::UpdateCurrentPrice(const wxString& symbol, const double price)
{
    double current_price = price;
    if (price == -1) {
        Model_StockHistory::Data_Set histData = Model_StockHistory::instance().find(Model_StockHistory::SYMBOL(symbol));
        if (!histData.empty())
        {
            std::sort(histData.begin(), histData.end(), SorterByDATE());
            current_price = histData.back().VALUE;
        }
    }
    if (current_price != -1)
    {
        Model_Stock::Data_Set stocks = Model_Stock::instance().find(Model_Stock::SYMBOL(symbol));
        for (auto& stock : stocks) {
            Model_Stock::Data* stockRecord = Model_Stock::instance().get(stock.STOCKID);
            stockRecord->CURRENTPRICE = current_price;
            Model_Stock::instance().save(stockRecord);
        }
    }
}

void Model_Stock::UpdatePosition(Model_Stock::Data* stock_entry)
{
    Model_Translink::Data_Set trans_list = Model_Translink::TranslinkList<Model_Stock>(stock_entry->STOCKID);
    double total_shares = 0;
    double total_initial_value = 0;
    double total_commission = 0;
    double avg_share_price = 0;
    wxString earliest_date = wxDate::Today().FormatISODate();
    Model_Checking::Data_Set checking_list;
    for (const auto &trans : trans_list)
    {
        Model_Checking::Data* checking_entry = Model_Checking::instance().get(trans.CHECKINGACCOUNTID);
        if (checking_entry && checking_entry->DELETEDTIME.IsEmpty() && Model_Checking::status_id(checking_entry->STATUS) != Model_Checking::STATUS_ID_VOID)
            checking_list.push_back(*checking_entry);
    }
    std::stable_sort(checking_list.begin(), checking_list.end(), SorterByTRANSDATE());
    for (const auto &trans : checking_list)
    {
        Model_Shareinfo::Data* share_entry = Model_Shareinfo::ShareEntry(trans.TRANSID);

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
    Model_Stock::instance().save(stock_entry);
}

