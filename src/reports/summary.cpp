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
#include "db/assets.h"
#include "mmCurrencyFormatter.h"
#include "model/Model_Account.h"
#include <algorithm>

#define SUMMARY_SORT_BY_NAME		1
#define SUMMARY_SORT_BY_BALANCE		2

mmReportSummary::mmReportSummary(mmCoreDB* core)
: mmPrintableBase(core)
{
	// set initial sort column
	sortColumn_ = SUMMARY_SORT_BY_NAME;
}

// structure for sorting of data
struct data_holder {wxString account_name; wxString link; double balance;};
bool mmSummarySortName (const data_holder& x, const data_holder& y)
{
	return x.account_name < y.account_name;
}
bool mmSummarySortBalance (const data_holder& x, const data_holder& y)
{
	if (x.balance != y.balance) return x.balance < y.balance;
	else return x.account_name < y.account_name;
}

wxString mmReportSummary::getHTMLText()
{
	data_holder line;
    std::vector<data_holder> dataChecking, dataTerm;

    /* Checking */
    double tBalance = 0.0;
    for (const auto& account: Model_Account::instance().all())
    {
        if (Model_Account::type(account) == Model_Account::CHECKING && Model_Account::status(account) == Model_Account::OPEN)
        {
            double bal = Model_Account::balance(account);

            mmCurrency* pCurrencyPtr = core_->accountList_.getCurrencySharedPtr(account.ACCOUNTID); // TODO
            wxASSERT(pCurrencyPtr);
            CurrencyFormatter::instance().loadSettings(*pCurrencyPtr);
            double rate = pCurrencyPtr->baseConv_;

            tBalance += bal * rate;

			line.account_name = account.ACCOUNTNAME;
			line.link = wxString::Format("ACCT:%d", account.ACCOUNTID);
			line.balance = bal;
			dataChecking.push_back(line);
        }
    }

    /* Terms */
    double tTBalance = 0.0;
    for (const auto& account: Model_Account::instance().all())
    {
        if (Model_Account::type(account) == Model_Account::TERM && Model_Account::status(account) == Model_Account::OPEN)
        {
            double bal = Model_Account::balance(account);

            mmCurrency* pCurrencyPtr = core_->accountList_.getCurrencySharedPtr(account.ACCOUNTID);
            wxASSERT(pCurrencyPtr);
            CurrencyFormatter::instance().loadSettings(*pCurrencyPtr);
            double rate = pCurrencyPtr->baseConv_;

            tTBalance += bal * rate;

			line.account_name = account.ACCOUNTNAME;
			line.link = wxString::Format("ACCT:%d", account.ACCOUNTID);
			line.balance = bal;
			dataTerm.push_back(line);
        }
    }

	switch(sortColumn_)
	{
	case SUMMARY_SORT_BY_BALANCE:
		std::stable_sort(dataChecking.begin(), dataChecking.end(), mmSummarySortBalance);
		std::stable_sort(dataTerm.begin(), dataTerm.end(), mmSummarySortBalance);
		break;
	default:
		sortColumn_ = SUMMARY_SORT_BY_NAME;
		std::stable_sort(dataChecking.begin(), dataChecking.end(), mmSummarySortName);
		std::stable_sort(dataTerm.begin(), dataTerm.end(), mmSummarySortName);
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
        hb.addTableCellLink(entry.link, entry.account_name, false, true);
		hb.addMoneyCell(entry.balance);
        hb.endTableRow();
	}

    // all sums below will be in base currency!
    core_->currencyList_.LoadBaseCurrencySettings();

    hb.startTableRow();
    hb.addTotalRow(_("Bank Accounts Total:"), 2, tBalance);
    hb.endTableRow();

    hb.addRowSeparator(2);

    /* Terms */
    for (const auto& entry : dataTerm)
	{
        hb.startTableRow();
        hb.addTableCellLink(entry.link, entry.account_name, false, true);
		hb.addMoneyCell(entry.balance);
        hb.endTableRow();
	}

    // all sums below will be in base currency!
    core_->currencyList_.LoadBaseCurrencySettings();

    if (wxGetApp().m_frame->hasActiveTermAccounts() )
    {
        hb.startTableRow();
        hb.addTotalRow(_("Term Accounts Total:"), 2, tTBalance);
        hb.endTableRow();
        hb.addRowSeparator(2);
    }

    tBalance += tTBalance;

    /* Stocks */

    wxArrayString accounts_type;
    accounts_type.Add(ACCOUNT_TYPE_STOCK);
    wxArrayInt accounts_id = core_->accountList_.getAccountsID(accounts_type);

    double original_val, stockBalance = 0;
    for (size_t i = 0; i < accounts_id.Count(); ++i)
    {
        double base_conv_rate = core_->accountList_.getAccountBaseCurrencyConvRate(accounts_id[i]);
        double amount = mmDBWrapper::getStockInvestmentBalance(core_->db_.get(), accounts_id[i], original_val);
        stockBalance += amount * base_conv_rate;
    }

    hb.startTableRow();
    hb.addTableCell(_("Stocks Total:"));
    hb.addMoneyCell(stockBalance);
    hb.endTableRow();
    hb.addRowSeparator(2);

    /* Assets */
    TAssetList asset_list(core_->db_.get());

    hb.startTableRow();
    hb.addTableCellLink("Assets", _("Assets"), false, true);
    hb.addTableCell(asset_list.GetAssetBalanceCurrencyFormat(), true);
    hb.endTableRow();

    tBalance += stockBalance;
    tBalance += asset_list.GetAssetBalance();

    hb.addRowSeparator(2);

    hb.addTotalRow(_("Total Balance on all Accounts"), 2, tBalance);
    hb.endTable();

    hb.endCenter();
    hb.end();

    return hb.getHTMLText();
}
