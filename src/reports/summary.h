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

#ifndef _MM_EX_REPORTSUMMARY_H_
#define _MM_EX_REPORTSUMMARY_H_

#include "reportbase.h"
#include <vector>
#include "mmex.h"
#include "model/Model.h"
#include "model/Model_Account.h"


class mmHistoryItem
{
public:
    mmHistoryItem();

    int         acctId;
    int         stockId;
    wxDate      purchaseDate;
    wxString    purchaseDateStr;
    double      purchasePrice;
    double      numShares;
    Model_StockHistory::Data_Set stockHist;
};

class mmHistoryData : public std::vector<mmHistoryItem>
{
public:
    double getDailyBalanceAt(const Model_Account::Data* account, const wxDate& date);
};

class mmReportSummaryByDate : public mmPrintableBase
{
public:
    mmReportSummaryByDate(int mode);
    wxString getHTMLText();
protected:
    enum TYPE { MONTHLY = 0, YEARLY };
private:
    int mode_;
    std::map<int, std::map<wxDate, double>> accountsBalanceMap;
    mmHistoryData   arHistory;
    std::map<wxString, double> currencyDateRateCache;

    std::map<wxDate, double> createCheckingBalanceMap(const Model_Account::Data& account);
    double getCheckingDailyBalanceAt(const Model_Account::Data* account, const wxDate& date);
    double getInvestingDailyBalanceAt(const Model_Account::Data* account, const wxDate& date);
    double getDailyBalanceAt(const Model_Account::Data* account, const wxDate& date);
    double getDayRate(int currencyid, const wxDate& date);
};

class mmReportSummaryByDateMontly : public mmReportSummaryByDate
{
public:
    mmReportSummaryByDateMontly();
};

class mmReportSummaryByDateYearly : public mmReportSummaryByDate
{
public:
    mmReportSummaryByDateYearly();
};

#endif //_MM_EX_REPORTSUMMARY_H_
