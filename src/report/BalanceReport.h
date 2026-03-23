/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel

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

#pragma once

#include <vector>
#include "mmex.h"
#include "model/AccountModel.h"
#include "_ReportBase.h"

// FIXME: m_stock_xa is set but mot used
struct StockDataExt : StockData
{
    StockHistoryModel::DataA m_hist_data_a;

    StockDataExt() : StockData() {}
    StockDataExt(const StockData& stock_d) : StockData(stock_d) {}
};

class BalanceReport : public ReportBase
{
public:
    enum PERIOD_ID
    {
        MONTH = 0,
        YEAR
    };

private:
    PERIOD_ID m_period_id;
    std::map<int64, std::map<mmDate, double>> m_account_balance_mDate_mId;
    std::vector<StockDataExt> m_stock_xa;
    std::map<wxString, double> m_currencyDateRateCache;

public:
    BalanceReport(PERIOD_ID period_id);
    wxString getHTMLText();

private:
    std::map<mmDate, double> loadAccountBalance_mDate(const AccountData& account_d);
    double getCheckingBalance(const AccountData* account_n, const mmDate& date);
    std::pair<double, double> getBalance(const AccountData* account_n, const mmDate& date);
    double getCurrencyDateRate(int64 currency_id, const mmDate& date);
};

