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

mmReportSummary::mmReportSummary(mmCoreDB* core)
: mmPrintableBase(core)
{}

wxString mmReportSummary::getHTMLText()
{
    mmHTMLBuilder hb;
    hb.init();
    hb.addHeader(2, _("Summary of Accounts"));
    hb.addDateNow();
    hb.addLineBreak();

    double tBalance = 0.0;

    hb.startCenter();

    hb.startTable("50%");
    hb.startTableRow();
    hb.addTableHeaderCell(_("Account Name"));
    hb.addTableHeaderCell(_("Balance"));
    hb.endTableRow();

    /* Checking */
    for (const auto& account: core_->accountList_.accounts_)
    {
        if (account->acctType_ == ACCOUNT_TYPE_BANK && account->status_ == mmAccount::MMEX_Open)
        {
            double bal = account->initialBalance_ + core_->bTransactionList_.getBalance(account->id_, mmIniOptions::instance().ignoreFutureTransactions_);

            mmCurrency* pCurrencyPtr = core_->accountList_.getCurrencySharedPtr(account->id_);
            wxASSERT(pCurrencyPtr);
            CurrencyFormatter::instance().loadSettings(*pCurrencyPtr);
            double rate = pCurrencyPtr->baseConv_;

            tBalance += bal * rate;

            hb.startTableRow();
            hb.addTableCellLink(wxString::Format("ACCT:%d", account->id_),account->name_, false, true);
			hb.addMoneyCell(bal);
            hb.endTableRow();
        }
    }

    // all sums below will be in base currency!
    core_->currencyList_.LoadBaseCurrencySettings();

    hb.startTableRow();
    hb.addTotalRow(_("Bank Accounts Total:"), 2, tBalance);
    hb.endTableRow();

    hb.addRowSeparator(2);

    /* Terms */
    double tTBalance = 0.0;

    for (const auto& account: core_->accountList_.accounts_)
    {
        if (account->status_== mmAccount::MMEX_Open && account->acctType_ == ACCOUNT_TYPE_TERM)
        {
            double bal = account->initialBalance_ + core_->bTransactionList_.getBalance(account->id_, mmIniOptions::instance().ignoreFutureTransactions_);

            mmCurrency* pCurrencyPtr = core_->accountList_.getCurrencySharedPtr(account->id_);
            wxASSERT(pCurrencyPtr);
            CurrencyFormatter::instance().loadSettings(*pCurrencyPtr);
            double rate = pCurrencyPtr->baseConv_;

            tTBalance += bal * rate;

            hb.startTableRow();
            hb.addTableCellLink(wxString::Format("ACCT:%d", account->id_),account->name_, false, true);
            hb.addMoneyCell(bal);
            hb.endTableRow();
        }
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
