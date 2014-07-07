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
    : mmPrintableBase(DATE)
    , refAccountID_(refAccountID)
    , transDialog_(transDialog)
    , sortby_(DATE)
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
    switch (sortColumn_)
    {
    case ACCOUNT:
        std::stable_sort(trans_.begin(), trans_.end(), SorterByACCOUNTNAME());
        break;
    case PAYEE:
        std::stable_sort(trans_.begin(), trans_.end(), SorterByPAYEENAME());
        break;
    case STATUS:
        std::stable_sort(trans_.begin(), trans_.end(), SorterBySTATUS());
        break;
    case CATEGORY:
        std::stable_sort(trans_.begin(), trans_.end(), SorterByCATEGNAME());
        break;
    case TYPE:
        std::stable_sort(trans_.begin(), trans_.end(), SorterByTRANSCODE());
        break;
    case AMOUNT:
        std::stable_sort(trans_.begin(), trans_.end(), SorterByTRANSAMOUNT());
        break;
    case NUMBER:
        std::stable_sort(trans_.begin(), trans_.end(), SorterByTRANSACTIONNUMBER());
        break;
    case NOTE:
        std::stable_sort(trans_.begin(), trans_.end(), SorterByNOTES());
        break;
    default: // DATE
        std::stable_sort(trans_.begin(), trans_.end(), SorterByTRANSDATE());
    }

    mmHTMLBuilder hb;
    hb.init();

    wxString transHeading = _("Transaction List ");
    const Model_Account::Data* account = Model_Account::instance().get(refAccountID_);
    if (account)
        transHeading = wxString::Format(_("Transaction List for Account: %s"), account->ACCOUNTNAME);

    hb.addHeader(2, transHeading);

    hb.addDateNow();
    hb.addLineBreak();

    hb.startTable();
    hb.startTable("95%");

    // Display the data Headings
    hb.startTableRow();
    if(DATE == sortColumn_)
        hb.addTableHeaderCell(_("Date"));
    else
        hb.addTableHeaderCellLink(wxString::Format("sort:%d", DATE), _("Date"));
    if(ACCOUNT == sortColumn_)
        hb.addTableHeaderCell(_("Account"));
    else
        hb.addTableHeaderCellLink(wxString::Format("sort:%d", ACCOUNT), _("Account"));
    if(PAYEE == sortColumn_)
        hb.addTableHeaderCell(_("Payee"));
    else
        hb.addTableHeaderCellLink(wxString::Format("sort:%d", PAYEE), _("Payee"));
    if(STATUS == sortColumn_)
        hb.addTableHeaderCell(_("Status"));
    else
        hb.addTableHeaderCellLink(wxString::Format("sort:%d", STATUS), _("Status"));
    if(CATEGORY == sortColumn_)
        hb.addTableHeaderCell(_("Category"));
    else
        hb.addTableHeaderCellLink(wxString::Format("sort:%d", CATEGORY), _("Category"));
    if(TYPE == sortColumn_)
        hb.addTableHeaderCell(_("Type"));
    else
        hb.addTableHeaderCellLink(wxString::Format("sort:%d", TYPE), _("Type"));
    if(AMOUNT == sortColumn_)
        hb.addTableHeaderCell(_("Amount"), true);
    else
        hb.addTableHeaderCellLink(wxString::Format("sort:%d", AMOUNT), _("Amount"), true);
    if(NUMBER == sortColumn_)
        hb.addTableHeaderCell(_("Number"));
    else
        hb.addTableHeaderCellLink(wxString::Format("sort:%d", NUMBER), _("Number"));
    if(NOTE == sortColumn_)
        hb.addTableHeaderCell(_("Notes"));
    else
        hb.addTableHeaderCellLink(wxString::Format("sort:%d", NOTE), _("Notes"));
    hb.endTableRow();

    // Display the data for each row
    double total = 0;
    for (auto& transaction : trans_)
    {
        hb.startTableRow();
        hb.addTableCell(mmGetDateForDisplay(mmGetStorageStringAsDate(transaction.TRANSDATE)));

        hb.addTableCellLink(wxString::Format("trxid:%d", transaction.TRANSID), transaction.ACCOUNTNAME);

        hb.addTableCell(transaction.PAYEENAME);

        hb.addTableCell(transaction.STATUS);

        hb.addTableCell(transaction.CATEGNAME, false, true);

        hb.addTableCell(wxGetTranslation(transaction.TRANSCODE));

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

        hb.addTableCell(transaction.TRANSACTIONNUMBER);
        hb.addTableCell(transaction.NOTES, false, true);
        hb.endTableRow();
    }

    // display the total balance.
    hb.addRowSeparator(9);
    hb.addTotalRow(_("Total Amount: "), 7, total);

    hb.endTable();
    hb.endCenter();

    transDialog_->getDescription(hb);

    hb.end();

    return hb.getHTMLText();
}

void mmReportTransactions::Run(mmFilterTransactionsDialog* dlg)
{
    const auto splits = Model_Splittransaction::instance().get_all();
    for (const auto& tran : Model_Checking::instance().all()) //TODO: find_or should be faster
    {
        if (!dlg->checkAll(tran, refAccountID_)) continue;
        Model_Checking::Full_Data full_tran(tran, splits);
        full_tran.PAYEENAME = full_tran.real_payee_name(refAccountID_);
        if (transDialog_->getCategoryCheckBox() && full_tran.has_split()) {
            full_tran.CATEGNAME.clear();
            full_tran.TRANSAMOUNT = 0;
            for (const auto& split : Model_Splittransaction::instance().find(Model_Splittransaction::TRANSID(full_tran.TRANSID)))
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
