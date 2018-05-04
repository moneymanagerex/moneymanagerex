/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)

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

Model_Stock::Model_Stock()
: Model<DB_Table_STOCK>()
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
bool Model_Stock::remove(int id)
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
    std::map<int, double> totBalance;

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
            if (nextValue == 0.0 && Model_Account::status(account) == Model_Account::OPEN)
            {
                nextValue = precValue;
                nextValueDate = precValueDate;
            }
            if (precValue > 0.0 && nextValue > 0.0 && precValueDate >= stock.PURCHASEDATE && nextValueDate >= stock.PURCHASEDATE)
                valueAtDate = precValue;
        }

        totBalance[stock.id()] += stock.NUMSHARES * valueAtDate;
    }

    double balance = 0.0;
    for (const auto& it : totBalance)
        balance += it.second;

    return balance;
}
