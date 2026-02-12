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
#include "model/_Model.h"
#include "model/AccountModel.h"
#include "reportbase.h"

class mmHistoryItem
{
public:
    mmHistoryItem();

    int64    acctId;
    int64    stockId;
    wxDate   purchaseDate;
    wxString purchaseDateStr;
    double   purchasePrice;
    double   numShares;
    StockHistoryModel::Data_Set stockHist;
};

class mmReportSummaryByDate : public ReportBase
{
public:
    enum PERIOD_ID
    {
        MONTH = 0,
        YEAR
    };

private:
    PERIOD_ID m_period_id;
    std::map<int64, std::map<wxDate, double>> m_account_date_balance;
    std::vector<mmHistoryItem> m_stock_a;
    std::map<wxString, double> m_currencyDateRateCache;

public:
    mmReportSummaryByDate(PERIOD_ID period_id);
    wxString getHTMLText();

private:
    std::map<wxDate, double> loadCheckingDateBalance(const AccountModel::Data& account);
    double getCheckingBalance(const AccountModel::Data* account, const wxDate& date);
    std::pair<double, double> getBalance(const AccountModel::Data* account, const wxDate& date);
    double getCurrencyDateRate(int64 currencyid, const wxDate& date);
};

