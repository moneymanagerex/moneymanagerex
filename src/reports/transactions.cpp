/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2011, 2012 Nikolay & Stefano Giorgio

 This program is free software; you can redistribute transcation and/or modify
 transcation under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that transcation will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ********************************************************/

#include "transactions.h"
#include "constants.h"
#include "htmlbuilder.h"
#include "util.h"
#include "model/Model_Payee.h"
#include "model/Model_Category.h"
#include <algorithm>

mmReportTransactions::mmReportTransactions(int refAccountID, mmFilterTransactionsDialog* transDialog)
    : refAccountID_(refAccountID)
    , transDialog_(transDialog)
    , ignoreDate_(false)
    , trans_()
{
    Run(transDialog_);
}

mmReportTransactions::~mmReportTransactions()
{
    // incase the user wants to print a report, we maintain the transaction dialog
    // until we are finished with the report.
    transDialog_->Destroy();
}

wxString mmReportTransactions::getHTMLText()
{
    mmHTMLBuilder hb;
    hb.init();

    wxString transHeading = _("Transaction List ");
    const Model_Account::Data* account = Model_Account::instance().get(refAccountID_);
    if (account)
        transHeading = wxString::Format(_("Transaction List for Account: %s"), account->ACCOUNTNAME);

    hb.addHeader(2, transHeading);

    hb.addDateNow();
    hb.addLineBreak();

    hb.startSortTable();

    hb.startThead();
    // Display the data Headings
    hb.startTableRow();
    hb.addTableHeaderCell(_("Date"));
    hb.addTableHeaderCell(_("Account"));
    hb.addTableHeaderCell(_("Payee"));
    hb.addTableHeaderCell(_("Status"));
    hb.addTableHeaderCell(_("Category"));
    hb.addTableHeaderCell(_("Type"));
    hb.addTableHeaderCell(_("Number"));
    hb.addTableHeaderCell(_("Notes"));
    hb.addTableHeaderCell(_("Amount"), true);
    hb.endTableRow();
    hb.endThead();

    hb.startTbody();

    // Display the data for each row
    double total = 0;
    for (auto& transaction : trans_)
    {
        hb.startTableRow();
        hb.addTableCell(mmGetDateForDisplay(mmGetStorageStringAsDate(transaction.TRANSDATE)));
        hb.addTableCellLink(wxString::Format("trxid:%d", transaction.TRANSID), transaction.ACCOUNTNAME);
        hb.addTableCell(transaction.PAYEENAME);
        hb.addTableCell(transaction.STATUS);
        hb.addTableCell(transaction.CATEGNAME);
        hb.addTableCell(wxGetTranslation(transaction.TRANSCODE));
        hb.addTableCell(transaction.TRANSACTIONNUMBER);
        hb.addTableCell(transaction.NOTES);
        // Get the exchange rate for the account
        Model_Account::Data* account = Model_Account::instance().get(transaction.ACCOUNTID);
        if (account)
        {
            double convRate = 1;
            const Model_Currency::Data* currency = Model_Account::currency(account);
            if (currency)
                convRate = currency->BASECONVRATE;
            int accountId = transaction.ACCOUNTID;
            if (transDialog_->getAccountCheckBox())
                accountId = transDialog_->getAccountID();
            double amount = Model_Checking::balance(transaction, accountId) * convRate;
            hb.addCurrencyCell(amount);
            total += amount;
        }
        else
            hb.addTableCell("");

        hb.endTableRow();
    }
    hb.endTbody();

    // display the total balance.
    hb.addTotalRow(_("Total Amount: "), 9, total);

    hb.endTable();

    transDialog_->getDescription(hb);

    hb.end();

    return hb.getHTMLText();
}

void mmReportTransactions::Run(mmFilterTransactionsDialog* dlg)
{
    const auto splits = Model_Splittransaction::instance().get_all();
    for (const auto& tran : Model_Checking::instance().all()) //TODO: find_or should be faster
    {
        if (!dlg->checkAll(tran, refAccountID_, splits)) continue;
        Model_Checking::Full_Data full_tran(tran, splits);
        full_tran.PAYEENAME = full_tran.real_payee_name(refAccountID_);
        if (transDialog_->getCategoryCheckBox() && full_tran.has_split()) {
            full_tran.CATEGNAME.clear();
            full_tran.TRANSAMOUNT = 0;
            for (const auto& split : full_tran.m_splits)
            {
                const wxString split_info = wxString::Format("%s = %s | "
                    , Model_Category::full_name(split.CATEGID, split.SUBCATEGID)
                    , wxString::Format("%.2f", split.SPLITTRANSAMOUNT));
                full_tran.CATEGNAME.Append(split_info);
                if (split.CATEGID != transDialog_->getCategId() ) continue;
                if (split.SUBCATEGID != transDialog_->getSubCategId() && !transDialog_->getSimilarStatus()) continue;

                full_tran.TRANSAMOUNT += split.SPLITTRANSAMOUNT;
            }
            full_tran.CATEGNAME.RemoveLast(2);
        }

        trans_.push_back(full_tran);
    }
}
