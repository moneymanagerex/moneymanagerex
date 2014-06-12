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

#include "summary.h"
#include "constants.h"
#include "htmlbuilder.h"
#include "model/Model_Account.h"
#include "model/Model_Currency.h"
#include "model/Model_Asset.h"
#include "model/Model_Stock.h"
#include <algorithm>

#if 0
#define SUMMARY_SORT_BY_NAME        1
#define SUMMARY_SORT_BY_BALANCE     2

mmReportSummary::mmReportSummary()
: mmPrintableBase(SUMMARY_SORT_BY_NAME)
, tBalance_(0.0)
, tTBalance_(0.0)
, stockBalance_(0.0)
, asset_balance_(0.0)
, totalBalance_(0.0)
{
}

bool mmSummarySortBalance(const summary_data_holder& x, const summary_data_holder& y)
{
    if (x.balance != y.balance) return x.balance < y.balance;
    else return x.name < y.name;
}

wxString mmReportSummary::version()
{
    return "$Rev: 6082 $";
}

void  mmReportSummary::RefreshData()
{
    dataChecking_.clear();
    dataTerm_.clear();

    summary_data_holder line;

    /* Checking */
    tBalance_ = 0.0;
    for (const auto& account: Model_Account::instance().all(Model_Account::COL_ACCOUNTNAME))
    {
        if (Model_Account::type(account) == Model_Account::CHECKING && Model_Account::status(account) == Model_Account::OPEN)
        {
            double bal = Model_Account::balance(account);

            Model_Currency::Data* currency = Model_Account::currency(account);
            tBalance_ += bal * currency->BASECONVRATE;

            line.name = account.ACCOUNTNAME;
            line.link = wxString::Format("ACCT:%d", account.ACCOUNTID);
            line.balance = bal;
            dataChecking_.push_back(line);
        }
    }

    /* Terms */
    tTBalance_ = 0.0;
    for (const auto& account: Model_Account::instance().all(Model_Account::COL_ACCOUNTNAME))
    {
        if (Model_Account::type(account) == Model_Account::TERM && Model_Account::status(account) == Model_Account::OPEN)
        {
            double bal = Model_Account::balance(account);

            Model_Currency::Data* currency = Model_Account::currency(account);
            tTBalance_ += bal * currency->BASECONVRATE;

            line.name = account.ACCOUNTNAME;
            line.link = wxString::Format("ACCT:%d", account.ACCOUNTID);
            line.balance = bal;
            dataTerm_.push_back(line);
        }
    }

    /* Stocks */
    stockBalance_ = 0.0;
    for (const auto& account : Model_Account::instance().all())
    {
        if (Model_Account::type(account) == Model_Account::INVESTMENT && Model_Account::status(account) == Model_Account::OPEN)
        {
            Model_Currency::Data* currency = Model_Account::currency(account);
            stockBalance_ += currency->BASECONVRATE * Model_Account::investment_balance(account).first;
        }
    }

    /* Assets */
    asset_balance_ = Model_Asset::instance().balance();

    totalBalance_ = tBalance_ + tTBalance_ + stockBalance_ + asset_balance_;
}

wxString mmReportSummary::getHTMLText()
{
    std::vector<summary_data_holder> sortedDataChecking(dataChecking_);
    std::vector<summary_data_holder> sortedDataTerm(dataTerm_);

    // List is presorted by account name
    if (SUMMARY_SORT_BY_BALANCE == sortColumn_)
    {
        std::stable_sort(sortedDataChecking.begin(), sortedDataChecking.end(), mmSummarySortBalance);
        std::stable_sort(sortedDataTerm.begin(), sortedDataTerm.end(), mmSummarySortBalance);
    }

    mmHTMLBuilder hb;
    hb.init();
    hb.addHeader(2, _("Summary of Accounts"));
    hb.addDateNow();
    hb.addLineBreak();

    hb.startCenter();

    hb.startTable("50%");
    hb.startTableRow();
    if(SUMMARY_SORT_BY_NAME == sortColumn_)
        hb.addTableHeaderCell(_("Account Name"));
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", SUMMARY_SORT_BY_NAME), _("Account Name"));
    if(SUMMARY_SORT_BY_BALANCE == sortColumn_)
        hb.addTableHeaderCell(_("Balance"), true);
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", SUMMARY_SORT_BY_BALANCE), _("Balance"), true);
    hb.endTableRow();

    /* Checking */
    for (const auto& entry : sortedDataChecking)
    {
        hb.startTableRow();
        hb.addTableCellLink(entry.link, entry.name, false, true);
        hb.addMoneyCell(entry.balance);
        hb.endTableRow();
    }

    hb.startTableRow();
    hb.addTotalRow(_("Bank Accounts Total:"), 2, tBalance_);
    hb.endTableRow();

    hb.addRowSeparator(2);

    /* Terms */
    for (const auto& entry : sortedDataTerm)
    {
        hb.startTableRow();
        hb.addTableCellLink(entry.link, entry.name, false, true);
        hb.addMoneyCell(entry.balance);
        hb.endTableRow();
    }

    if (Model_Account::hasActiveTermAccount())
    {
        hb.startTableRow();
        hb.addTotalRow(_("Term Accounts Total:"), 2, tTBalance_);
        hb.endTableRow();
        hb.addRowSeparator(2);
    }

    hb.startTableRow();
    hb.addTableCell(_("Stocks Total:"));
    hb.addMoneyCell(stockBalance_);
    hb.endTableRow();
    hb.addRowSeparator(2);

    hb.startTableRow();
    hb.addTableCellLink("Assets:", _("Assets"), false, true);
    hb.addMoneyCell(asset_balance_);
    hb.endTableRow();

    hb.addRowSeparator(2);

    hb.addTotalRow(_("Total Balance on all Accounts"), 2, totalBalance_);
    hb.endTable();

    hb.endCenter();
    hb.end();

    return hb.getHTMLText();
}
#endif

mmReportSummaryByDate::mmReportSummaryByDate(mmGUIFrame* frame, int mode)
: mmPrintableBase()
, frame_(frame)
, mode_(mode)
{}

wxString mmReportSummaryByDate::getHTMLText()
{
    int             i = 0, j;
    double          balancePerDay[5];
    mmHTMLBuilder   hb;
    wxString        datePrec;
    wxDate			date, dateStart = wxDate::Now(), dateEnd = wxDate::Now();
    wxDateSpan      span;
    std::vector<balanceMap> balanceMapVec(Model_Account::instance().all().size());
    std::vector<std::map<wxDate, double>::const_iterator>   arIt(balanceMapVec.size());
    std::vector<double> arBalance(balanceMapVec.size());
    std::vector<wxString>   totBalanceData;

    hb.init();
    hb.addHeader(2, wxString::Format(_("Account Balance - %s"), mode_==0 ? _("Monthly Report"):_("Yearly Report")));
    hb.addDateNow();
    hb.addLineBreak();

    hb.startCenter();

    hb.startTable("90%");
    hb.startTableRow();
    hb.addTableHeaderCell(_("Date"));
    hb.addTableHeaderCell(_("Bank Account"), true);
    hb.addTableHeaderCell(_("Credit Card Account"), true);
    hb.addTableHeaderCell(_("Term Accounts"), true);
    hb.addTableHeaderCell(_("Stocks"), true);
    hb.addTableHeaderCell(_("Balance"), true);
    hb.endTableRow();

    for (const auto& account: Model_Account::instance().all())
    {
        if (Model_Account::type(account) == Model_Account::CHECKING || Model_Account::type(account) == Model_Account::CREDIT_CARD || Model_Account::type(account) == Model_Account::TERM)
        {
            //  in balanceMapVec ci sono i totali dei movimenti giorno per giorno
            const Model_Currency::Data* currency = Model_Account::currency(account);
            for (const auto& tran: Model_Account::transaction(account))
                balanceMapVec[i][Model_Checking::TRANSDATE(tran)] += Model_Checking::balance(tran, account.ACCOUNTID) * currency->BASECONVRATE;
            if ((Model_Account::type(account) == Model_Account::CHECKING || Model_Account::type(account) == Model_Account::CREDIT_CARD) && balanceMapVec[i].size())
            {
                date = balanceMapVec[i].begin()->first;
                if (date.IsEarlierThan(dateStart))
                    dateStart = date;
            }
            arBalance[i] = account.INITIALBAL * currency->BASECONVRATE;
        }
        i++;
    }

    if (mode_ == 0)
    {
        dateEnd -= wxDateSpan::Months(1);
        dateEnd.SetToLastMonthDay(dateEnd.GetMonth(), dateEnd.GetYear());
        span = wxDateSpan::Months(1);
    }
    else if (mode_ == 1)
    {
        dateEnd.Set(31, wxDateTime::Dec, wxDateTime::Now().GetYear(), 23, 59, 59);
        span = wxDateSpan::Years(1);
    }
    else
        wxASSERT(0);

    date = dateEnd;
    while (date.IsLaterThan(dateStart))
        date -= span;
    dateStart = date;
//dateStart.Set(31,wxDateTime::Dec,2013);

    i = 0;
    for (const auto& acctMap: balanceMapVec)
        arIt[i++] = acctMap.begin();

    while (dateStart <= dateEnd)
    {
        if (mode_ == 0 )
            dateStart.SetToLastMonthDay(dateStart.GetMonth(), dateStart.GetYear());

        i = 0;
        for (auto& account: Model_Account::instance().all())
        {
            if (Model_Account::type(account) == Model_Account::CHECKING || Model_Account::type(account) == Model_Account::CREDIT_CARD || Model_Account::type(account) == Model_Account::TERM)
            {
                for (; arIt[i] != balanceMapVec[i].end(); ++arIt[i])
                {
                    if (arIt[i]->first.IsLaterThan(dateStart))
                        break;
                    arBalance[i] += arIt[i]->second;
                }
            }
            else if (Model_Account::type(account) == Model_Account::INVESTMENT)
                arBalance[i] = GetDailyBalanceAt(&account, dateStart);
            i++;
        }

        totBalanceData.push_back(dateStart.FormatDate());
        for (j=0; j<5; j++)
            balancePerDay[j] = 0.0;
        for (j=0; j<4; j++)
        {
            i = 0;
            for (const auto& account: Model_Account::instance().all())
            {
                if ((j == 0 && Model_Account::type(account) == Model_Account::CHECKING) ||
                    (j == 1 && Model_Account::type(account) == Model_Account::CREDIT_CARD) ||
                    (j == 2 && Model_Account::type(account) == Model_Account::TERM) ||
                    (j == 3 && Model_Account::type(account) == Model_Account::INVESTMENT))
                {
                    balancePerDay[j] += arBalance[i];
                }
                i++;
            }
            totBalanceData.push_back(Model_Currency::toCurrency(balancePerDay[j]));
            balancePerDay[4] += balancePerDay[j];
        }
        totBalanceData.push_back(Model_Currency::toCurrency(balancePerDay[j]));

        dateStart += span;
    }

    for (i=totBalanceData.size()-6; i>=0; i-=6)
    {
        if (!datePrec.IsEmpty() && datePrec.Right(4) != totBalanceData[i].Right(4))
            hb.addRowSeparator(6);
        hb.startTableRow();
        hb.addTableCell(totBalanceData[i], false, true);
        hb.addTableCell(totBalanceData[i+1], true);
        hb.addTableCell(totBalanceData[i+2], true);
        hb.addTableCell(totBalanceData[i+3], true);
        hb.addTableCell(totBalanceData[i+4], true);
        hb.addTableCell(totBalanceData[i+5], true, false, true);
        hb.endTableRow();
        datePrec = totBalanceData[i];
    }

    hb.endTable();
    hb.endCenter();
    hb.end();

    return hb.getHTMLText();
}

double mmReportSummaryByDate::GetDailyBalanceAt(const Model_Account::Data *account, wxDateTime date)
{
    double	convRate = 1.0;

    if (account)
    {
        Model_Currency::Data* currency = Model_Account::currency(account);
        convRate = currency->BASECONVRATE;
    }

    return Model_Stock::instance().getDailyBalanceAt(account, date) * convRate;
}