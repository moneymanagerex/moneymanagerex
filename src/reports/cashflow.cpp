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
#include "Model_Account.h"
#include "Model_Billsdeposits.h"
#include "Model_CurrencyHistory.h"

static const wxString COLORS [] = {
    ""
    , "rgba(240, 248, 255, 0.5)"
};

mmReportCashFlow::mmReportCashFlow(TYPE cashflowreporttype)
    : mmPrintableBase(_("Cash Flow"))
    , cashFlowReportType_(cashflowreporttype)
    , today_(wxDateTime::Today())
    , colorId_(0)
{
    m_only_active = true;
}

mmReportCashFlow::~mmReportCashFlow()
{
}

int mmReportCashFlow::report_parameters()
{
    return RepParams::ACCOUNTS_LIST;
}

wxString mmReportCashFlow::getHTMLText()
{
    return this->getHTMLText_i();
}

void mmReportCashFlow::getStats(double& tInitialBalance, std::vector<ValueTrio>& forecastVector)
{
    int years = cashFlowReportType_ == MONTHLY ? 10 : 1;// Monthly for 10 years or Daily for 1 year
    std::map<wxDateTime, double> daily_balance;
    wxArrayInt account_id;

    for (const auto& account : Model_Account::instance().all())
    {
        if (Model_Account::status(account) == Model_Account::CLOSED
            || Model_Account::type(account) == Model_Account::INVESTMENT) continue;

        if (accountArray_)
        {
            if (wxNOT_FOUND == accountArray_->Index(account.ACCOUNTNAME)) continue;
        }

        const auto transactions = Model_Account::transaction(account);
        if (transactions.empty()) continue;
        
        // Use account first transaction date for initial balance
        const double initConvRate = Model_CurrencyHistory::getDayRate(account.CURRENCYID, transactions[0].TRANSDATE);
        tInitialBalance += account.INITIALBAL * initConvRate;

        account_id.Add(account.ACCOUNTID);
        for (const auto& tran : transactions)
        {
            // Do not include asset or stock transfers in income expense calculations.
            if (Model_Checking::foreignTransactionAsTransfer(tran))
                continue;

            const double convRate = Model_CurrencyHistory::getDayRate(account.CURRENCYID, tran.TRANSDATE);
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
        if (repeatsType >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute User Acknowlegement required
            repeatsType -= BD_REPEATS_MULTIPLEX_BASE;
        if (repeatsType >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute Silent mode
            repeatsType -= BD_REPEATS_MULTIPLEX_BASE;

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
    for (int idx = 0; idx < (int) forecastVector.size(); idx++)
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
        forecastVector[idx].label = mmGetDateForDisplay(dtEnd.FormatISODate());
    }
}

wxString mmReportCashFlow::getHTMLText_i()
{
    int years = cashFlowReportType_ == MONTHLY ? 10 : 1;// Monthly for 10 years or Daily for 1 year
    double tInitialBalance = 0.0;

    // Now we have a vector of dates and amounts over next year
    int forecastItemsNum = cashFlowReportType_ == MONTHLY ? 12 * years : 366 * years;
    std::vector<ValueTrio> forecastVector(forecastItemsNum, {"", "", 0.0});
    getStats(tInitialBalance, forecastVector);

    mmHTMLBuilder hb;
    hb.init();
    hb.addDivContainer();

    const wxString& headerMsg = wxString::Format (_("Cash Flow Forecast for %i Years Ahead"), years);
    hb.addHeader(2, headerMsg );
    wxString accountsMsg;
    if (accountArray_) 
    {
        for (const auto& entry : *accountArray_)
            accountsMsg.Append((accountsMsg.empty() ? "" : ", ") + entry);
    } 
    else 
    {
        accountsMsg << _("All Accounts");
    }
    
    if (accountsMsg.empty())
        accountsMsg = _("None");
    accountsMsg.Prepend(_("Accounts: "));

    hb.addHeader(2, accountsMsg);
    hb.addDateNow();
    hb.addLineBreak();

    //hb.addLineChart(forecastVector, "test");
    hb.addDivRow();
    hb.addDivCol17_67();
    hb.startTable();
    hb.startThead();
    hb.startTableRow();
    hb.addTableHeaderCell(_("Date"));
    hb.addTableHeaderCell(_("Total"), true);
    hb.addTableHeaderCell(_("Difference"), true);
    hb.endTableRow();
    hb.endThead();

    hb.startTbody();
    for (int idx = 0; idx < (int)forecastVector.size(); idx++)
    {        
        double balance = forecastVector[idx].amount + tInitialBalance;
        double diff = (idx == 0 ? 0 : forecastVector[idx].amount - forecastVector[idx-1].amount) ;
        const wxDateTime dtEnd = cashFlowReportType_ == MONTHLY
            ? today_.Add(wxDateSpan::Months(idx)) : today_.Add(wxDateSpan::Days(idx));

        // Add a separator for each year/month in daily cash flow report
        if (cashFlowReportType_ == MONTHLY)
        {
            colorId_ = dtEnd.GetYear() % 2;
        }
        else
        {
            const wxDateTime& firstDayOfTheMonth = wxDateTime(dtEnd).SetDay(1);
            if (dtEnd == firstDayOfTheMonth) colorId_ = (colorId_+1) % 2;
        }

        hb.startTableRow(COLORS[colorId_]);
        hb.addTableCell(forecastVector[idx].label);
        hb.addMoneyCell(balance);
        hb.addMoneyCell(diff);
        hb.endTableRow();
    }
    hb.endTbody();

    hb.endTable();
    hb.endDiv();
    hb.endDiv();
    hb.endDiv();
    hb.end();

    return hb.getHTMLText();
}
