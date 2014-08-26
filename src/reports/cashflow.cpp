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
    int years = cashFlowReportType_ == YEARLY ? 10 : 1;// Monthly for 7 years or Daily for 1 year
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

    wxDateTime yearFromNow = today_.Add(wxDateSpan::Years(years));
    forecastVec fvec;

    for (const auto& q1 : Model_Billsdeposits::instance().all())
    {
        wxDateTime nextOccurDate = Model_Billsdeposits::NEXTOCCURRENCEDATE(q1);

        int repeats = q1.REPEATS;
        int numRepeats = q1.NUMOCCURRENCES;
        wxString transType = q1.TRANSCODE;
        double amt = q1.TRANSAMOUNT;
        double toAmt = q1.TOTRANSAMOUNT;

        // DeMultiplex the Auto Executable fields from the db entry: REPEATS
        if (repeats >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute User Acknowlegement required
            repeats -= BD_REPEATS_MULTIPLEX_BASE;
        if (repeats >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute Silent mode
            repeats -= BD_REPEATS_MULTIPLEX_BASE;

        bool processNumRepeats = false;
        if (numRepeats != -1)
            processNumRepeats = true;

        if (repeats == 0)
        {
            numRepeats = 1;
            processNumRepeats = true;
        }

        if (nextOccurDate > yearFromNow)
            continue;

        int accountID = q1.ACCOUNTID;
        int toAccountID = q1.TOACCOUNTID;

        bool isAccountFound = false;
        const Model_Account::Data* account = Model_Account::instance().get(accountID);
        if (account)
        {
            isAccountFound = !(accountArray_ != nullptr && wxNOT_FOUND == accountArray_->Index(account->ACCOUNTNAME)); //linear search
        }

        bool isToAccountFound = false;
        const Model_Account::Data* to_account = Model_Account::instance().get(toAccountID);
        if (to_account)
        {
            isToAccountFound = !(accountArray_ != nullptr && wxNOT_FOUND == accountArray_->Index(to_account->ACCOUNTNAME)); //linear search
        }

        if (!isAccountFound && !isToAccountFound) continue; // skip account

        double convRate = (account ? Model_Account::currency(account)->BASECONVRATE : 1.0);
        double toConvRate = (to_account ? Model_Account::currency(to_account)->BASECONVRATE : 1.0);

        // Process all possible repeating transactions for this BD
        while (1)
        {
            if (nextOccurDate > yearFromNow)
                break;

            if (processNumRepeats)
                numRepeats--;

            mmRepeatForecast rf;
            rf.date = nextOccurDate;
            rf.amount = 0.0;

            switch (Model_Billsdeposits::type(q1))
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

            //            nextOccurDate = nextOccurDate.Add(mmRepeat(repeats));

            if (repeats == 1)
            {
                nextOccurDate = nextOccurDate.Add(wxTimeSpan::Week());
            }
            else if (repeats == 2)
            {
                nextOccurDate = nextOccurDate.Add(wxTimeSpan::Weeks(2));
            }
            else if (repeats == 3)
            {
                nextOccurDate = nextOccurDate.Add(wxDateSpan::Month());
            }
            else if (repeats == 4)
            {
                nextOccurDate = nextOccurDate.Add(wxDateSpan::Months(2));
            }
            else if (repeats == 5)
            {
                nextOccurDate = nextOccurDate.Add(wxDateSpan::Months(3));
            }
            else if (repeats == 6)
            {
                nextOccurDate = nextOccurDate.Add(wxDateSpan::Months(6));
            }
            else if (repeats == 7)
            {
                nextOccurDate = nextOccurDate.Add(wxDateSpan::Year());
            }
            else if (repeats == 8)
            {
                nextOccurDate = nextOccurDate.Add(wxDateSpan::Months(4));
            }
            else if (repeats == 9)
            {
                nextOccurDate = nextOccurDate.Add(wxDateSpan::Weeks(4));
            }
            else if (repeats == 10)
            {
                nextOccurDate = nextOccurDate.Add(wxDateSpan::Days(1));
            }
            else if (repeats == 11) // repeat in numRepeats Days (Once only)
            {
                if (numRepeats > 0)
                {
                    nextOccurDate = nextOccurDate.Add(wxDateSpan::Days(numRepeats));
                    numRepeats = -1;
                }
                else break;
            }
            else if (repeats == 12) // repeat in numRepeats Months (Once only)
            {
                if (numRepeats > 0)
                {
                    nextOccurDate = nextOccurDate.Add(wxDateSpan::Months(numRepeats));
                    numRepeats = -1;
                }
                else break;
            }
            else if (repeats == 13) // repeat every numRepeats Days
            {
                if (numRepeats > 0)
                {
                    nextOccurDate = nextOccurDate.Add(wxDateSpan::Days(numRepeats));
                }
                else break;
            }
            else if (repeats == 14) // repeat every numRepeats Months
            {
                if (numRepeats > 0)
                {
                    nextOccurDate = nextOccurDate.Add(wxDateSpan::Months(numRepeats));
                }
                else break;
            }
            else if ((repeats == 15) || (repeats == 16))
            {
                nextOccurDate = nextOccurDate.Add(wxDateSpan::Month());
                nextOccurDate = nextOccurDate.SetToLastMonthDay(nextOccurDate.GetMonth(), nextOccurDate.GetYear());
                if (repeats == 16) // last weekday of month
                {
                    if (nextOccurDate.GetWeekDay() == wxDateTime::Sun || nextOccurDate.GetWeekDay() == wxDateTime::Sat)
                        nextOccurDate.SetToPrevWeekDay(wxDateTime::Fri);
                }
            }
            else break;
        } // end while
    } //end query

    const wxDateTime& dtBegin = today_;
    for (int idx = 0; idx < (int) forecastVector.size(); idx++)
    {
        wxDateTime dtEnd = cashFlowReportType_ == YEARLY ? dtBegin.Add(wxDateSpan::Months(idx)) : dtBegin.Add(wxDateSpan::Days(idx));

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
}
