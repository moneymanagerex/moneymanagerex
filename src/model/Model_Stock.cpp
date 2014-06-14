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

double Model_Stock::value(const Data* r)
{
    return r->NUMSHARES * r->PURCHASEPRICE + r->COMMISSION;
}

double Model_Stock::value(const Data& r)
{
    return value(&r);
}

/**
Returns the last price date of a given stock
*/
wxString Model_Stock::lastPriceDate(const Self::Data* entity)
{
    wxString dtStr = entity->PURCHASEDATE;
    wxString strSql = wxString::Format("SELECT * FROM STOCKHISTORY_V1 WHERE STOCKID=%ld ORDER BY DATE DESC", entity->id());
    wxSQLite3Statement std = db_->PrepareStatement(strSql);
    wxSQLite3ResultSet qd = std.ExecuteQuery();
    if (qd.NextRow())
        dtStr = qd.GetString("DATE");

    return dtStr;
}

/**
Returns the total stock balance at a given date
*/
double Model_Stock::getDailyBalanceAt(const Model_Account::Data *account, const wxDate& date)
{
    wxDateTime                  precValueDate, nextValueDate;
    std::map<int, double>        totBalance, mapNumShares, mapPurchPrize;
    std::map<int, wxDateTime>    mapPurchDate;

    Data_Set stocks = this->instance().find(HELDAT(account->id()));
    for (const auto & stock : stocks)
    {
        mapNumShares[stock.id()] = stock.NUMSHARES;
        mapPurchPrize[stock.id()] = stock.PURCHASEPRICE;
        mapPurchDate[stock.id()] = PURCHASEDATE(stock);

        Model_StockHistory::Data_Set stock_hist = Model_StockHistory::instance().find(Model_StockHistory::STOCKID(stock.id())
                , DB_Table_STOCKHISTORY_V1::DATE(date.FormatISODate()));

        for (const auto & hist : stock_hist)
        {
            totBalance[hist.STOCKID] += hist.VALUE;
        }
    }

    if (1)
    {
        for (const auto& it : totBalance)
        {
            if (it.second != 0.0)
                continue;
            double precValue = 0.0, nextValue = 0.0;
            //  searching of the next and previous price at the given date
            wxString strSQL = wxString::Format("SELECT * FROM STOCKHISTORY_V1 WHERE STOCKID=%d AND DATE<='%s' ORDER BY DATE DESC", it.first, date.FormatISODate());
            wxSQLite3Statement  st2 = db_->PrepareStatement(strSQL);
            wxSQLite3ResultSet  q2 = st2.ExecuteQuery();
            if (q2.NextRow())
            {
                precValue = q2.GetDouble("VALUE");
                precValueDate = q2.GetDate("DATE");
            }
            //  if not found but if the given date is after purchase date, takes purchase price
            else if (date >= mapPurchDate[it.first])
            {
                precValue = mapPurchPrize[it.first];
                precValueDate = mapPurchDate[it.first];
            }
            //  searching of the first price next the given date
            strSQL = wxString::Format("SELECT * FROM STOCKHISTORY_V1 WHERE STOCKID=%d AND DATE>'%s' ORDER BY DATE", it.first, date.FormatISODate());
            st2 = db_->PrepareStatement(strSQL);
            q2 = st2.ExecuteQuery();
            if (q2.NextRow())
            {
                nextValue = q2.GetDouble("VALUE");
                nextValueDate = q2.GetDate("DATE");
            }
            //  if not found and the accoung is open, takes previous date
            else if (account->STATUS.CmpNoCase(Model_Account::all_status()[Model_Account::OPEN]) == 0)
            {
                nextValue = precValue;
                nextValueDate = precValueDate;
            }
            if (precValue > 0.0 && nextValue > 0.0 && precValueDate >= mapPurchDate[it.first] && nextValueDate >= mapPurchDate[it.first])
                totBalance[it.first] += mapNumShares[it.first] * precValue;
            st2.Finalize();
        }
    }

    double balance = 0;
    for (const auto& it : totBalance)
        balance += it.second;

    return balance;
}
