/*******************************************************
Copyright (C) 2012 Guan Lisheng (guanlisheng@gmail.com)
Copyright (C) 2017 James Higley

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

#include "cashflow.h"
#include "mmex.h"
#include "mmframe.h"
#include "util.h"
#include "reports/htmlbuilder.h"
#include "model/Model_Account.h"
#include "model/Model_Billsdeposits.h"
#include "model/Model_CurrencyHistory.h"

static const wxString COLORS [] = {
    ""
    , "rgba(240, 248, 255, 0.5)"
};

mmReportCashFlow::mmReportCashFlow(const wxString& name)
    : mmPrintableBase(name)
    , today_(wxDateTime::Today())
{
    m_only_active = true;
}

mmReportCashFlow::~mmReportCashFlow()
{
}

wxString mmReportCashFlow::getHTMLText()
{
    return this->getHTMLText_i();
}

void mmReportCashFlow::getStats(double& tInitialBalance, std::vector<ValuePair>& forecastVector)
{
    int years = cashFlowReportType_ == MONTHLY ? 10 : 1;// Monthly for 10 years or Daily for 1 year
    std::map<wxDateTime, double> daily_balance;
    wxArrayInt account_id;

    for (const auto& account : Model_Account::instance().find(
        Model_Account::ACCOUNTTYPE(Model_Account::all_type()[Model_Account::INVESTMENT], NOT_EQUAL)
        , Model_Account::STATUS(Model_Account::CLOSED, NOT_EQUAL)))
    {
        if (accountArray_ && accountArray_->Index(account.ACCOUNTNAME) == wxNOT_FOUND) {
            continue;
        }

        double convRate = Model_CurrencyHistory::getDayRate(account.CURRENCYID
            , today_.FormatISODate());
        tInitialBalance += account.INITIALBAL * convRate;

        account_id.Add(account.ACCOUNTID);
        const auto transactions = Model_Account::transaction(account);

        for (const auto& tran : transactions)
        {
            // Do not include asset or stock transfers in income expense calculations.
            if (Model_Checking::foreignTransactionAsTransfer(tran))
                continue;

            daily_balance[Model_Checking::TRANSDATE(tran)] += Model_Checking::balance(tran, account.ACCOUNTID) * convRate;
        }
    }

    // We now know the total balance on the account
    // Start by walking through the recurring transaction list

    const wxDateTime yearFromNow = today_.Add(wxDateSpan::Years(years));
    forecastVec fvec;

    for (const auto& entry : Model_Billsdeposits::instance().all())
    {
        wxDateTime nextOccurDate = Model_Billsdeposits::NEXTOCCURRENCEDATE(entry);
        if (nextOccurDate > yearFromNow) continue;

        int repeatsType = entry.REPEATS;
        int numRepeats = entry.NUMOCCURRENCES;
        double amt = entry.TRANSAMOUNT;
        double toAmt = entry.TOTRANSAMOUNT;

        // DeMultiplex the Auto Executable fields from the db entry: REPEATS
        repeatsType %= BD_REPEATS_MULTIPLEX_BASE;

        bool processNumRepeats = numRepeats != -1 || repeatsType == 0;
        if (repeatsType == 0)
        {
            numRepeats = 1;
            processNumRepeats = true;
        }

        bool isAccountFound = account_id.Index(entry.ACCOUNTID) != wxNOT_FOUND;
        bool isToAccountFound = account_id.Index(entry.TOACCOUNTID) != wxNOT_FOUND;
        if (!isAccountFound && !isToAccountFound)
            continue; // skip account

        // Process all possible recurring transactions for this BD
        while (1)
        {
            if (nextOccurDate > yearFromNow) break;
            if (processNumRepeats) numRepeats--;

            mmRepeatForecast rf;
            rf.date = nextOccurDate;
            rf.amount = 0.0;
            const double convRate = Model_CurrencyHistory::getDayRate(Model_Account::instance().get(entry.ACCOUNTID)->CURRENCYID, rf.date);

            switch (Model_Billsdeposits::type(entry))
            {
            case Model_Billsdeposits::WITHDRAWAL:
                rf.amount = -amt * convRate;
                break;
            case Model_Billsdeposits::DEPOSIT:
                rf.amount = +amt * convRate;
                break;
            case Model_Billsdeposits::TRANSFER:
                if (isAccountFound)
                    rf.amount -= amt * convRate;
                if (isToAccountFound)
                {
                    const double toConvRate = Model_CurrencyHistory::getDayRate(Model_Account::instance().get(entry.TOACCOUNTID)->CURRENCYID, rf.date);
                    rf.amount += toAmt * toConvRate;
                }
                break;
            default:
                break;
            }

            fvec.push_back(rf);

            if (processNumRepeats && (numRepeats <= 0))
                break;

            nextOccurDate = Model_Billsdeposits::nextOccurDate(repeatsType, numRepeats, nextOccurDate);

            if (repeatsType == Model_Billsdeposits::REPEAT_IN_X_DAYS) // repeat in numRepeats Days (Once only)
            {
                if (numRepeats > 0)
                    numRepeats = -1;
                else
                    break;
            }
            else if (repeatsType == Model_Billsdeposits::REPEAT_IN_X_MONTHS) // repeat in numRepeats Months (Once only)
            {
                if (numRepeats > 0)
                    numRepeats = -1;
                else
                    break;
            }
            else if (repeatsType == Model_Billsdeposits::REPEAT_EVERY_X_DAYS) // repeat every numRepeats Days
                numRepeats = entry.NUMOCCURRENCES;
            else if (repeatsType == Model_Billsdeposits::REPEAT_EVERY_X_MONTHS) // repeat every numRepeats Months
                numRepeats = entry.NUMOCCURRENCES;
        } // end while
    } //end query

    const wxDateTime& dtBegin = today_;
    for (size_t idx = 0; idx < forecastVector.size(); idx++)
    {
        wxDateTime dtEnd = cashFlowReportType_ == MONTHLY
            ? dtBegin.Add(wxDateSpan::Months(idx)) : dtBegin.Add(wxDateSpan::Days(idx));

        for (const auto& balance : fvec)
        {
            if (balance.date.IsBetween(dtBegin, dtEnd))
                forecastVector[idx].amount += balance.amount;
        }

        for (const auto& d_balance : daily_balance)
        {
            if (!d_balance.first.IsLaterThan(dtEnd))
                forecastVector[idx].amount += d_balance.second;
        }
        forecastVector[idx].label = dtEnd.FormatISODate();
    }
}

wxString mmReportCashFlow::getHTMLText_i()
{
    // Grab the data
    bool monthly_report = (cashFlowReportType_ == MONTHLY);
    int years = (monthly_report ? 10 : 1); // Monthly for 10 years or Daily for 1 year
    double tInitialBalance = 0.0;

    // Now we have a vector of dates and amounts over next year
    int forecastItemsNum = (monthly_report ? 12 * years : 366 * years);
    std::vector<ValuePair> forecastVector(forecastItemsNum, {"", 0.0});
    getStats(tInitialBalance, forecastVector);

    // Build the report
    mmHTMLBuilder hb;
    hb.init();
    const wxString& headingStr = wxString::Format(wxPLURAL("Cash Flow Forecast for %i Year Ahead",
        "Cash Flow Forecast for %i Years Ahead",
        years),
        years);
    hb.addReportHeader(headingStr, 1, false);
    hb.DisplayFooter(getAccountNames());

    GraphData gd;
    GraphSeries gs;

    for (const auto& entry : forecastVector)
    {
        double amount = entry.amount + tInitialBalance;
        gs.values.push_back(amount);
        gd.labels.push_back(entry.label);
    }
    gd.series.push_back(gs);

    if (getChartSelection() == 0 && !gd.series.empty())
    {
        gd.type = GraphData::LINE_DATETIME;
        hb.addChart(gd);
    }

    hb.addDivContainer("shadow");
    {
        hb.startTable();
        {
            hb.startThead();
            {
                hb.startTableRow();
                hb.addTableHeaderCell(_("Date"));
                hb.addTableHeaderCell(_("Total"), "text-right");
                hb.addTableHeaderCell(_("Difference"), "text-right");
                hb.endTableRow();
            }
            hb.endThead();

            hb.startTbody();
            {
                int rowType = 0;
                for (size_t idx = 0; idx < forecastVector.size(); idx++)
                {
                    double balance = forecastVector[idx].amount + tInitialBalance;
                    double diff = (idx == 0 ? 0 : forecastVector[idx].amount - forecastVector[idx-1].amount) ;
                    const wxDateTime dtEnd = cashFlowReportType_ == MONTHLY
                        ? today_.Add(wxDateSpan::Months(idx)) : today_.Add(wxDateSpan::Days(idx));

                    // Add a separator for each year/month in daily cash flow report
                    if (cashFlowReportType_ == MONTHLY)
                    {
                        rowType = dtEnd.GetYear() % 2;
                    }
                    else
                    {
                        const wxDateTime& firstDayOfTheMonth = wxDateTime(dtEnd).SetDay(1);
                        if (dtEnd == firstDayOfTheMonth) rowType = (rowType + 1) % 2;
                    }

                    if (rowType == 0)
                        hb.startTableRow();
                    else
                        hb.startAltTableRow();
                    {
                        hb.addTableCell(gd.labels[idx]);
                        hb.addMoneyCell(balance);
                        hb.addMoneyCell(diff);
                    }
                    hb.endTableRow();
                }
            }
            hb.endTbody();
        }
        hb.endTable();
    }
    hb.endDiv();

    hb.end();

    wxLogDebug("======= mmReportCashFlow:getHTMLText =======");
    wxLogDebug("%s", hb.getHTMLText());

    return hb.getHTMLText();
}

mmReportCashFlowDaily::mmReportCashFlowDaily()
    : mmReportCashFlow("Cash Flow - Daily")
{
    cashFlowReportType_ = TYPE::DAILY;
    setReportParameters(Reports::DailyCashFlow);
}

mmReportCashFlowMonthly::mmReportCashFlowMonthly()
    : mmReportCashFlow("Cash Flow - Monthly")
{
    cashFlowReportType_ = TYPE::MONTHLY;
    setReportParameters(Reports::MonthlyCashFlow);
}

