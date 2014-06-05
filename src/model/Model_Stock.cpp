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
wxString Model_Stock::lastPriceDate(Self::Data* entity)
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
double Model_Stock::getDailyBalanceAt(const Model_Account::Data *account, wxDate date)
{
    int                         stockID;
    double                      value, precValue, nextValue, balance = 0.0;
    wxString                    strWhere, strSQL;
    wxDateTime                  valueDate, precValueDate, nextValueDate;
    std::map<int, double>        totBalance, mapNumShares, mapPurchPrize;
    std::map<int, wxDateTime>    mapPurchDate;

    strSQL = wxString::Format("SELECT * FROM STOCK_V1 WHERE HELDAT=%d", account->id());
    wxSQLite3Statement  st0 = db_->PrepareStatement(strSQL);
    wxSQLite3ResultSet  q0 = st0.ExecuteQuery();
    while (q0.NextRow())
    {
        if (!strWhere.IsEmpty())
            strWhere += " OR ";
        stockID = q0.GetInt("STOCKID");
        strWhere += "STOCKID=" + wxString::Format("%d", stockID);
        totBalance[stockID] = 0.0;
        mapNumShares[stockID] = q0.GetDouble("NUMSHARES");
        mapPurchPrize[stockID] = q0.GetDouble("PURCHASEPRICE");
        mapPurchDate[stockID] = q0.GetDate("PURCHASEDATE");
    }

    if (!strWhere.IsEmpty())
    {
        strSQL = wxString::Format("SELECT * FROM STOCKHISTORY_V1 WHERE (%s) AND DATE='%s' ORDER BY STOCKID", strWhere, date.FormatISODate());
        wxSQLite3Statement  st1 = db_->PrepareStatement(strSQL);
        wxSQLite3ResultSet  q1 = st1.ExecuteQuery();
        while (q1.NextRow())
        {
            stockID = q1.GetInt("STOCKID");
            value = q1.GetDouble("VALUE");
            totBalance[stockID] += mapNumShares[stockID] * value;
        }

        for (const auto& it : totBalance)
        {
            if (it.second != 0.0)
                continue;
            precValue = nextValue = 0.0;
            //  searching of the next and previous price at the given date
            strSQL = wxString::Format("SELECT * FROM STOCKHISTORY_V1 WHERE STOCKID=%d AND DATE<='%s' ORDER BY DATE DESC", it.first, date.FormatISODate());
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
        st1.Finalize();
    }

    for (const auto& it : totBalance)
        balance += it.second;

    return balance;
}
