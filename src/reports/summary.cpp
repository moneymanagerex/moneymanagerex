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
#include "mmex.h"
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
{
}

// structure for sorting of data
struct data_holder {wxString name; wxString link; double balance;};
bool mmSummarySortBalance (const data_holder& x, const data_holder& y)
{
    if (x.balance != y.balance) return x.balance < y.balance;
    else return x.name < y.name;
}

wxString mmReportSummary::getHTMLText()
{
    data_holder line;
    std::vector<data_holder> dataChecking, dataTerm;

    /* Checking */
    double tBalance = 0.0;
    for (const auto& account: Model_Account::instance().all(Model_Account::COL_ACCOUNTNAME))
    {
        if (Model_Account::type(account) == Model_Account::CHECKING && Model_Account::status(account) == Model_Account::OPEN)
        {
            double bal = Model_Account::balance(account);

            Model_Currency::Data* currency = Model_Account::currency(account);
            tBalance += bal * currency->BASECONVRATE;

            line.name = account.ACCOUNTNAME;
            line.link = wxString::Format("ACCT:%d", account.ACCOUNTID);
            line.balance = bal;
            dataChecking.push_back(line);
        }
    }

    /* Terms */
    double tTBalance = 0.0;
    for (const auto& account: Model_Account::instance().all(Model_Account::COL_ACCOUNTNAME))
    {
        if (Model_Account::type(account) == Model_Account::TERM && Model_Account::status(account) == Model_Account::OPEN)
        {
            double bal = Model_Account::balance(account);

            Model_Currency::Data* currency = Model_Account::currency(account);
            tTBalance += bal * currency->BASECONVRATE;

            line.name = account.ACCOUNTNAME;
            line.link = wxString::Format("ACCT:%d", account.ACCOUNTID);
            line.balance = bal;
            dataTerm.push_back(line);
        }
    }

    if (SUMMARY_SORT_BY_BALANCE == sortColumn_)
    {
        std::stable_sort(dataChecking.begin(), dataChecking.end(), mmSummarySortBalance);
        std::stable_sort(dataTerm.begin(), dataTerm.end(), mmSummarySortBalance);
    }
    else
    {
        // List is presorted by account name
        sortColumn_ = SUMMARY_SORT_BY_NAME;
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
    for (const auto& entry : dataChecking)
    {
        hb.startTableRow();
        hb.addTableCellLink(entry.link, entry.name, false, true);
        hb.addMoneyCell(entry.balance);
        hb.endTableRow();
    }

    hb.startTableRow();
    hb.addTotalRow(_("Bank Accounts Total:"), 2, tBalance);
    hb.endTableRow();

    hb.addRowSeparator(2);

    /* Terms */
    for (const auto& entry : dataTerm)
    {
        hb.startTableRow();
        hb.addTableCellLink(entry.link, entry.name, false, true);
        hb.addMoneyCell(entry.balance);
        hb.endTableRow();
    }

    if (wxGetApp().m_frame->hasActiveTermAccounts() )
    {
        hb.startTableRow();
        hb.addTotalRow(_("Term Accounts Total:"), 2, tTBalance);
        hb.endTableRow();
        hb.addRowSeparator(2);
    }

    tBalance += tTBalance;

    /* Stocks */
    double stockBalance = 0.0;
    for (const auto& account: Model_Account::instance().all())
    {
        if (Model_Account::type(account) != Model_Account::INVESTMENT) continue;
        Model_Currency::Data* currency = Model_Account::currency(account);
        stockBalance += currency->BASECONVRATE * Model_Account::investment_balance(account).second;
    }

    hb.startTableRow();
    hb.addTableCell(_("Stocks Total:"));
    hb.addMoneyCell(stockBalance);
    hb.endTableRow();
    hb.addRowSeparator(2);

    /* Assets */
    double asset_balance = Model_Asset::instance().balance();

    hb.startTableRow();
    hb.addTableCellLink("Assets", _("Assets"), false, true);
    hb.addMoneyCell(asset_balance);
    hb.endTableRow();

    tBalance += stockBalance;
    tBalance += asset_balance; 

    hb.addRowSeparator(2);

    hb.addTotalRow(_("Total Balance on all Accounts"), 2, tBalance);
    hb.endTable();

    hb.endCenter();
    hb.end();

    return hb.getHTMLText();
}
