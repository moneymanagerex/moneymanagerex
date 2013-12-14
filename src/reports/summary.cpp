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
#include <algorithm>

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
    return "$Rev$";
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
        if (Model_Account::type(account) != Model_Account::INVESTMENT) continue;
        Model_Currency::Data* currency = Model_Account::currency(account);
        stockBalance_ += currency->BASECONVRATE * Model_Account::investment_balance(account).first;
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
    hb.addTableCellLink("Assets", _("Assets"), false, true);
    hb.addMoneyCell(asset_balance_);
    hb.endTableRow();

    hb.addRowSeparator(2);

    hb.addTotalRow(_("Total Balance on all Accounts"), 2, totalBalance_);
    hb.endTable();

    hb.endCenter();
    hb.end();

    return hb.getHTMLText();
}
