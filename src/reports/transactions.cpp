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
#include "model/Model_Account.h"
#include <algorithm>

mmReportTransactions::mmReportTransactions(const std::vector<mmBankTransaction*>& trans,
    int refAccountID, mmFilterTransactionsDialog* transDialog)
: mmPrintableBase(mmBankTransaction::DATE)
, trans_(trans)
, refAccountID_(refAccountID)
, transDialog_(transDialog)
{
}

mmReportTransactions::~mmReportTransactions()
{
    // incase the user wants to print a report, we maintain the transaction dialog
    // until we are finished with the report.
    transDialog_->Destroy();
}

wxString addFilterDetailes(wxString sHeader, wxString sValue)
{
    wxString sData;
    sData << "<b>" << sHeader << " </b>" << sValue << "<br>";
    return sData;
}

wxString mmReportTransactions::getHTMLText()
{
    for (auto& transaction: trans_)
        transaction->sortby_ = (mmBankTransaction::SORT)sortColumn_;
    std::stable_sort (trans_.begin(), trans_.end());

    mmHTMLBuilder hb;
    hb.init();

    wxString transHeading = _("Transaction List ");
    if (refAccountID_ > -1)
    {
        const Model_Account::Data* account = Model_Account::instance().get(refAccountID_);
        transHeading = wxString::Format(_("Transaction List for Account: %s"), account->ACCOUNTNAME);
    }
    hb.addHeader(2, transHeading);

    hb.addDateNow();
    hb.addLineBreak();

    hb.startTable();
    hb.startTable("95%");

    // Display the data Headings
    hb.startTableRow();
    if(mmBankTransaction::DATE == sortColumn_)
        hb.addTableHeaderCell(_("Date"));
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", mmBankTransaction::DATE), _("Date"));
    if(mmBankTransaction::ACCOUNT == sortColumn_)
        hb.addTableHeaderCell(_("Account"));
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", mmBankTransaction::ACCOUNT), _("Account"));
    if(mmBankTransaction::PAYEE == sortColumn_)
        hb.addTableHeaderCell(_("Payee"));
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", mmBankTransaction::PAYEE), _("Payee"));
    if(mmBankTransaction::STATUS == sortColumn_)
        hb.addTableHeaderCell(_("Status"));
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", mmBankTransaction::STATUS), _("Status"));
    if(mmBankTransaction::CATEGORY == sortColumn_)
        hb.addTableHeaderCell(_("Category"));
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", mmBankTransaction::CATEGORY), _("Category"));
    if(mmBankTransaction::TYPE == sortColumn_)
        hb.addTableHeaderCell(_("Type"));
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", mmBankTransaction::TYPE), _("Type"));
    if(mmBankTransaction::AMOUNT == sortColumn_)
        hb.addTableHeaderCell(_("Amount"), true);
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", mmBankTransaction::AMOUNT), _("Amount"), true);
    if(mmBankTransaction::NUMBER == sortColumn_)
        hb.addTableHeaderCell(_("Number"));
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", mmBankTransaction::NUMBER), _("Number"));
    if(mmBankTransaction::NOTE == sortColumn_)
        hb.addTableHeaderCell(_("Notes"));
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", mmBankTransaction::NOTE), _("Notes"));
    hb.endTableRow();

    // Display the data for each row
    double total = 0;
    for (auto& transaction: trans_)
    {
        transaction->updateTransactionData(refAccountID_, total);

        hb.startTableRow();
        hb.addTableCell(transaction->date_);
        Model_Account::Data* account = Model_Account::instance().get(transaction->accountID_);
        hb.addTableCellLink(wxString::Format("TRXID:%d", transaction->transactionID()), ( account ? account->ACCOUNTNAME : ""));
        Model_Payee::Data* payee = (transaction->transType_ != TRANS_TYPE_TRANSFER_STR ? Model_Payee::instance().get(transaction->payeeID_) : NULL);
        hb.addTableCell( payee ? payee->PAYEENAME : "" );
        hb.addTableCell(transaction->status_);
        hb.addTableCell(transaction->fullCatStr_, false, true);
        hb.addTableCell(wxGetTranslation(transaction->transType_));
        // Get the exchange rate for the selected account
        const Model_Currency::Data* currency = Model_Account::currency(account);
        double dbRate = currency->BASECONVRATE;
        hb.addMoneyCell(transaction->value(refAccountID_) * dbRate);
        hb.addTableCell(transaction->transNum_);
        hb.addTableCell(transaction->notes_, false, true);
        hb.endTableRow();
    }

    // display the total balance.
    hb.addRowSeparator(9);
    hb.addTotalRow(_("Total Amount: "), 7, total);

    hb.endTable();
    hb.endCenter();

    // Extract the parameters from the transaction dialog and add them to the report.
    wxString filterDetails;

    if ( transDialog_->getAccountCheckBox())
        filterDetails << addFilterDetailes(_("Account:"), transDialog_->getAccountName());

    //Date range
    if ( transDialog_->getDateRangeCheckBox())
        filterDetails << addFilterDetailes(_("Date Range:"), transDialog_->userDateRangeStr());

    //Payees
    if ( transDialog_->checkPayeeCheckBox())
        filterDetails << addFilterDetailes(_("Payee:"), transDialog_->userPayeeStr());

    //Category
    if ( transDialog_->getCategoryCheckBox())
        filterDetails << "<b>" << _("Category:") << " </b>" <<transDialog_->userCategoryStr()
        << (transDialog_->getExpandStatus() ? wxString(" <b> ") << _("Subcategory:") << " </b>" << _("Any"): "")
        << "<br>";
    //Status
    if ( transDialog_->getStatusCheckBox())
        filterDetails << addFilterDetailes(_("Status:"), transDialog_->userStatusStr());
    //Type
    if ( transDialog_->getTypeCheckBox() )
        filterDetails << addFilterDetailes(_("Type:"), transDialog_->userTypeStr());
    //Amount Range
    if ( transDialog_->getAmountRangeCheckBoxMin() || transDialog_->getAmountRangeCheckBoxMax())
        filterDetails << addFilterDetailes(_("Amount Range:"), transDialog_->userAmountRangeStr());
    //Number
    if ( transDialog_->getNumberCheckBox())
        filterDetails << addFilterDetailes(_("Number:"), transDialog_->getNumber());
    //Notes
    if ( transDialog_->getNotesCheckBox())
        filterDetails << addFilterDetailes(_("Notes:"), transDialog_->getNotes());

    if ( !filterDetails.IsEmpty())
    {
        hb.addHorizontalLine();
        filterDetails.Prepend( wxString()<< "<b>" << _("Filtering Details: ") << "</b><br>");
        hb.addParaText(filterDetails );
    }

    hb.end();

    return hb.getHTMLText();
}
