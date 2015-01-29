/*******************************************************
Copyright (C) 2006-2012

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
#include "htmlbuilder.h"
#include "model/Model_Account.h"
#include "model/Model_Billsdeposits.h"

static const wxString COLORS [] = {
    ""
    , "rgba(240, 248, 255, 0.5)"
};

mmReportCashFlow::mmReportCashFlow(int cashflowreporttype)
    : mmPrintableBaseSpecificAccounts(_("Cash Flow"))
    , activeTermAccounts_(false)
    , activeBankAccounts_(false)
    , cashFlowReportType_(cashflowreporttype)
    , m_cashflowSpecificAccounts(false)
    , today_(wxDateTime::Today())
    , colorId_(0)
{}

mmReportCashFlow::~mmReportCashFlow()
{
}

void mmReportCashFlow::activateTermAccounts() 
{
    activeTermAccounts_ = true;
}

void mmReportCashFlow::activateBankAccounts() 
{
    activeBankAccounts_ = true;
}

wxString mmReportCashFlow::getHTMLText()
{
    return this->getHTMLText_i();
}

void mmReportCashFlow::getStats(double& tInitialBalance, std::vector<ValueTrio>& forecastVector)
{
    int years = cashFlowReportType_ == YEARLY ? 10 : 1;// Monthly for 10 years or Daily for 1 year
    std::map<wxDateTime, double> daily_balance;

    for (const auto& account : Model_Account::instance().all())
    {
        if (Model_Account::status(account) == Model_Account::CLOSED
            || Model_Account::type(account) == Model_Account::INVESTMENT) continue;

        if (accountArray_)
        {
            if (wxNOT_FOUND == accountArray_->Index(account.ACCOUNTNAME)) continue;
        }
        else
        {
            if (! activeTermAccounts_ && Model_Account::type(account) == Model_Account::TERM) continue;
            if (! activeBankAccounts_ && (Model_Account::type(account) == Model_Account::CHECKING || Model_Account::type(account) == Model_Account::CREDIT_CARD)) continue;
        }

        const Model_Currency::Data* currency = Model_Account::currency(account);
        tInitialBalance += account.INITIALBAL * currency->BASECONVRATE;

        for (const auto& tran : Model_Account::transaction(account))
        {
            daily_balance[Model_Checking::TRANSDATE(tran)] += Model_Checking::balance(tran, account.ACCOUNTID) * currency->BASECONVRATE;
        }
    }

    // We now know the total balance on the account
    // Start by walking through the repeating transaction list

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

        int accountID = entry.ACCOUNTID;
        int toAccountID = entry.TOACCOUNTID;

        const Model_Account::Data* account = Model_Account::instance().get(accountID);
        bool isAccountFound = account && !(accountArray_ != nullptr 
            && wxNOT_FOUND == accountArray_->Index(account->ACCOUNTNAME)); //linear search

        const Model_Account::Data* to_account = Model_Account::instance().get(toAccountID);
        bool isToAccountFound = to_account && !(accountArray_ != nullptr 
            && wxNOT_FOUND == accountArray_->Index(to_account->ACCOUNTNAME)); //linear search

        if (!isAccountFound && !isToAccountFound) continue; // skip account

        // Determine if we need to process this account
        if (!m_cashflowSpecificAccounts)
        {
            if (Model_Account::status(account) == Model_Account::CLOSED
                || Model_Account::type(account) == Model_Account::INVESTMENT) continue;
            if (!activeTermAccounts_ && Model_Account::type(account) == Model_Account::TERM) continue;
            if (!activeBankAccounts_ && (Model_Account::type(account) == Model_Account::CHECKING
                || Model_Account::type(account) == Model_Account::CREDIT_CARD)) continue;
        }

        double convRate = (account ? Model_Account::currency(account)->BASECONVRATE : 1.0);
        double toConvRate = (to_account ? Model_Account::currency(to_account)->BASECONVRATE : 1.0);

        // Process all possible repeating transactions for this BD
        while (1)
        {
            if (nextOccurDate > yearFromNow) break;
            if (processNumRepeats) numRepeats--;

            mmRepeatForecast rf;
            rf.date = nextOccurDate;
            rf.amount = 0.0;

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
                    rf.amount += toAmt * toConvRate;
                break;
            default:
                break;
            }

            fvec.push_back(rf);

            if (processNumRepeats && (numRepeats <= 0)) 
                break;

            nextOccurDate = Model_Billsdeposits::nextOccurDate(repeatsType, numRepeats, nextOccurDate);

            if (processNumRepeats) numRepeats--;
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
        wxDateTime dtEnd = cashFlowReportType_ == YEARLY 
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
        forecastVector[idx].label = mmGetDateForDisplay(dtEnd);
    }
}

wxString mmReportCashFlow::getHTMLText_i()
{
    int years = cashFlowReportType_ == YEARLY ? 10 : 1;// Monthly for 10 years or Daily for 1 year
    double tInitialBalance = 0.0;

    // Now we have a vector of dates and amounts over next year
    int forecastItemsNum = cashFlowReportType_ == YEARLY ? 12 * years : 366 * years;
    std::vector<ValueTrio> forecastVector(forecastItemsNum, {"", "", 0.0});
    getStats(tInitialBalance, forecastVector);

    mmHTMLBuilder hb;
    hb.init();
    hb.addDivContainer();

    wxString headerMsg = wxString::Format (_("Cash Flow Forecast for %i Years Ahead"), years);
    hb.addHeader(2, headerMsg );
    headerMsg = _("Accounts: ");
    if (accountArray_ == nullptr) 
    {
        if (activeBankAccounts_ && activeTermAccounts_)
            headerMsg << _("All Accounts");
        else if (activeBankAccounts_)
            headerMsg << _("All Bank Accounts");
        else if (activeTermAccounts_)
            headerMsg << _("All Term Accounts");
    } 
    else 
    {
        int arrIdx = 0;
 
        if ( (int)accountArray_->size() == 0 )
            headerMsg << "?";

        if ( !accountArray_->empty() )
        {
            headerMsg << accountArray_->Item(arrIdx);
            arrIdx ++;
        }
        while ( arrIdx < (int)accountArray_->size() )
        {
            headerMsg << ", " << accountArray_->Item(arrIdx);
            arrIdx ++;
        }

    }

    hb.addHeader(2, headerMsg);
    hb.addDateNow();
    hb.addLineBreak();

    //hb.addLineChart(forecastVector, "test");
    hb.addDivRow();
    hb.addDivCol8();
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
        wxDateTime dtEnd = cashFlowReportType_ == YEARLY
            ? today_.Add(wxDateSpan::Months(idx)) : today_.Add(wxDateSpan::Days(idx));

        // Add a separator for each year/month in daily cash flow report
        if (cashFlowReportType_ == YEARLY)
        {
            colorId_ = dtEnd.GetYear() % 2;
        }
        else
        {
            const wxDateTime firstDayOfTheMonth = wxDateTime(dtEnd).SetDay(1);
            if (dtEnd == firstDayOfTheMonth) colorId_ = ++colorId_ % 2;
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

//-----------------------------------------------------------------------------
mmReportCashFlowAllAccounts::mmReportCashFlowAllAccounts()
: mmReportCashFlow(0)
{
    this->activateBankAccounts();
    this->activateTermAccounts();
}

mmReportCashFlowBankAccounts::mmReportCashFlowBankAccounts()
: mmReportCashFlow(0)
{
    this->activateBankAccounts();
}

mmReportCashFlowTermAccounts::mmReportCashFlowTermAccounts()
: mmReportCashFlow(0)
{
    this->activateTermAccounts();
}

//-----------------------------------------------------------------------------
mmReportCashFlowSpecificAccounts::mmReportCashFlowSpecificAccounts()
: mmReportCashFlow(0)
{
    this->cashFlowReportType_ = YEARLY;
    this->m_cashflowSpecificAccounts = true;
}

wxString mmReportCashFlowSpecificAccounts::getHTMLText()
{
    this->getSpecificAccounts();
    return this->getHTMLText_i();
}

//-----------------------------------------------------------------------------
mmReportDailyCashFlowSpecificAccounts::mmReportDailyCashFlowSpecificAccounts()
: mmReportCashFlowSpecificAccounts()
{
    this->cashFlowReportType_ = DAILY;
    this->m_cashflowSpecificAccounts = true;
}
