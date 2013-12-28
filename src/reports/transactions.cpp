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
#include <algorithm>

mmReportTransactions::mmReportTransactions(const Model_Checking::Full_Data_Set& trans,
    int refAccountID, mmFilterTransactionsDialog* transDialog)
    : mmPrintableBase(DATE)
    , trans_(trans)
    , refAccountID_(refAccountID)
    , transDialog_(transDialog)
    , sortby_(DATE)
    , ignoreDate_(false)
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
    if(DATE == sortColumn_)
        hb.addTableHeaderCell(_("Date"));
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", DATE), _("Date"));
    if(ACCOUNT == sortColumn_)
        hb.addTableHeaderCell(_("Account"));
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", ACCOUNT), _("Account"));
    if(PAYEE == sortColumn_)
        hb.addTableHeaderCell(_("Payee"));
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", PAYEE), _("Payee"));
    if(STATUS == sortColumn_)
        hb.addTableHeaderCell(_("Status"));
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", STATUS), _("Status"));
    if(CATEGORY == sortColumn_)
        hb.addTableHeaderCell(_("Category"));
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", CATEGORY), _("Category"));
    if(TYPE == sortColumn_)
        hb.addTableHeaderCell(_("Type"));
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", TYPE), _("Type"));
    if(AMOUNT == sortColumn_)
        hb.addTableHeaderCell(_("Amount"), true);
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", AMOUNT), _("Amount"), true);
    if(NUMBER == sortColumn_)
        hb.addTableHeaderCell(_("Number"));
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", NUMBER), _("Number"));
    if(NOTE == sortColumn_)
        hb.addTableHeaderCell(_("Notes"));
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", NOTE), _("Notes"));
    hb.endTableRow();

    // Display the data for each row
    double total = 0;
    for (auto& transaction : trans_)
    {
        hb.startTableRow();
        hb.addTableCell(mmGetDateForDisplay(mmGetStorageStringAsDate(transaction.TRANSDATE)));

        hb.addTableCellLink(wxString::Format("TRXID:%d", transaction.TRANSID), transaction.ACCOUNTNAME);

        hb.addTableCell(transaction.PAYEENAME);

        hb.addTableCell(transaction.STATUS);

        hb.addTableCell(transaction.CATEGNAME, false, true);

        hb.addTableCell(wxGetTranslation(transaction.TRANSCODE));
        // Get the exchange rate for the account
        Model_Account::Data* account = Model_Account::instance().get(transaction.ACCOUNTID);
        const Model_Currency::Data* currency = Model_Account::currency(account);
        if (currency)
        {
            int accountId = transaction.ACCOUNTID;
            if (transDialog_->getAccountCheckBox())
                accountId = transDialog_->getAccountID();
            double amount = Model_Checking::balance(transaction, accountId) * currency->BASECONVRATE;
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
    if (transDialog_->getCategoryCheckBox())
    {
        filterDetails << "<b>" << _("Category:") << " </b>" << transDialog_->userCategoryStr()
        << (transDialog_->getSimilarCategoryStatus() ? wxString(" (") << _("Include Similar") << ")" : "")
        << "<br>";
    }
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
