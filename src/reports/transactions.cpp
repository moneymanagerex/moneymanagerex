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

#define TRANS_SORT_BY_DATE      1
#define TRANS_SORT_BY_ACCOUNT   2
#define TRANS_SORT_BY_PAYEE     3
#define TRANS_SORT_BY_STATUS    4
#define TRANS_SORT_BY_CATEGORY  5
#define TRANS_SORT_BY_TYPE      6
#define TRANS_SORT_BY_AMOUNT    7
#define TRANS_SORT_BY_NUMBER    8
#define TRANS_SORT_BY_NOTE      9

mmReportTransactions::mmReportTransactions(const std::vector<mmBankTransaction>& trans,
    mmCoreDB* core, int refAccountID, mmFilterTransactionsDialog* transDialog)
: mmPrintableBase(core)
, trans_(trans)
, refAccountID_(refAccountID)
, transDialog_(transDialog)
{
    // set initial sort column
    sortColumn_ = TRANS_SORT_BY_DATE;
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
    // structure for sorting of data
    struct data_holder {wxDateTime date; wxString account; wxString link; wxString payee; wxString status; wxString categ; wxString type; double amount; wxString num; wxString note;} line;
    std::vector<data_holder> data;

    double total = 0;

    for (auto& transaction: trans_)
    {
        transaction.updateTransactionData(refAccountID_, total);

        line.date = transaction.date_;
        Model_Account::Data* account = Model_Account::instance().get(transaction.accountID_);
        line.account = account ? account->ACCOUNTNAME : "";
        line.link = wxString::Format("TRXID:%d", transaction.transactionID());
        Model_Payee::Data* payee = Model_Payee::instance().get(transaction.payeeID_);
        line.payee = ( (payee) ? payee->PAYEENAME : "" );
        line.status = transaction.status_;
        line.categ = transaction.fullCatStr_;
        line.type = transaction.transType_;
        // Get the exchange rate for the selected account
        double dbRate = core_->accountList_.getAccountBaseCurrencyConvRate(transaction.accountID_);
        line.amount = transaction.value(refAccountID_) * dbRate;
        line.num = transaction.transNum_;
        line.note = transaction.notes_;
        data.push_back(line);
    }

    switch (sortColumn_)
    {
    case TRANS_SORT_BY_ACCOUNT:
        std::stable_sort(data.begin(), data.end()
            , [] (const data_holder& x, const data_holder& y)
            {
                if (x.account != y.account) return x.account < y.account;
                else return x.date < y.date;
            }
        );
        break;
    case TRANS_SORT_BY_PAYEE:
        std::stable_sort(data.begin(), data.end()
            , [] (const data_holder& x, const data_holder& y)
            {
                if (x.payee != y.payee) return x.payee < y.payee;
                else if (x.date != y.date) return x.date < y.date;
                else return x.account < y.account;
            }
        );
        break;
    case TRANS_SORT_BY_STATUS:
        std::stable_sort(data.begin(), data.end()
            , [] (const data_holder& x, const data_holder& y)
            {
                if (x.status != y.status) return x.status < y.status;
                else if (x.date != y.date) return x.date < y.date;
                else return x.account < y.account;
            }
        );
        break;
    case TRANS_SORT_BY_CATEGORY:
        std::stable_sort(data.begin(), data.end()
            , [] (const data_holder& x, const data_holder& y)
            {
                if (x.categ != y.categ) return x.categ < y.categ;
                else if (x.date != y.date) return x.date < y.date;
                else return x.account < y.account;
            }
        );
        break;
    case TRANS_SORT_BY_TYPE:
        std::stable_sort(data.begin(), data.end()
            , [] (const data_holder& x, const data_holder& y)
            {
                if (x.type != y.type) return x.type < y.type;
                else if (x.date != y.date) return x.date < y.date;
                else return x.account < y.account;
            }
        );
        break;
    case TRANS_SORT_BY_AMOUNT:
        std::stable_sort(data.begin(), data.end()
            , [] (const data_holder& x, const data_holder& y)
            {
                if (x.amount != y.amount) return x.amount < y.amount;
                else if (x.date != y.date) return x.date < y.date;
                else return x.account < y.account;
            }
        );
        break;
    case TRANS_SORT_BY_NUMBER:
        std::stable_sort(data.begin(), data.end()
            , [] (const data_holder& x, const data_holder& y)
            {
                if (x.num != y.num) return x.num < y.num;
                else if (x.date != y.date) return x.date < y.date;
                else return x.account < y.account;
            }
        );
        break;
    case TRANS_SORT_BY_NOTE:
        std::stable_sort(data.begin(), data.end()
            , [] (const data_holder& x, const data_holder& y)
            {
                if (x.note != y.note) return x.note < y.note;
                else if (x.date != y.date) return x.date < y.date;
                else return x.account < y.account;
            }
        );
        break;
    default:
        sortColumn_ = TRANS_SORT_BY_DATE;
        std::stable_sort(data.begin(), data.end()
            , [] (const data_holder& x, const data_holder& y)
            {
                if (x.date != y.date) return x.date < y.date;
                else return x.account < y.account;
            }
        );
    }

    mmHTMLBuilder hb;
    hb.init();

    wxString transHeading = _("Transaction List ");
    if (refAccountID_ > -1)
    {
        transHeading = wxString::Format(_("Transaction List for Account: %s")
            ,core_->accountList_.GetAccountName(refAccountID_));
    }
    hb.addHeader(2, transHeading);

    hb.addDateNow();
    hb.addLineBreak();

    hb.startTable();
    hb.startTable("95%");

    // Display the data Headings
    hb.startTableRow();
    if(TRANS_SORT_BY_DATE == sortColumn_)
        hb.addTableHeaderCell(_("Date"));
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", TRANS_SORT_BY_DATE), _("Date"));
    if(TRANS_SORT_BY_ACCOUNT == sortColumn_)
        hb.addTableHeaderCell(_("Account"));
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", TRANS_SORT_BY_ACCOUNT), _("Account"));
    if(TRANS_SORT_BY_PAYEE == sortColumn_)
        hb.addTableHeaderCell(_("Payee"));
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", TRANS_SORT_BY_PAYEE), _("Payee"));
    if(TRANS_SORT_BY_STATUS == sortColumn_)
        hb.addTableHeaderCell(_("Status"));
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", TRANS_SORT_BY_STATUS), _("Status"));
    if(TRANS_SORT_BY_CATEGORY == sortColumn_)
        hb.addTableHeaderCell(_("Category"));
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", TRANS_SORT_BY_CATEGORY), _("Category"));
    if(TRANS_SORT_BY_TYPE == sortColumn_)
        hb.addTableHeaderCell(_("Type"));
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", TRANS_SORT_BY_TYPE), _("Type"));
    if(TRANS_SORT_BY_AMOUNT == sortColumn_)
        hb.addTableHeaderCell(_("Amount"), true);
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", TRANS_SORT_BY_AMOUNT), _("Amount"), true);
    if(TRANS_SORT_BY_NUMBER == sortColumn_)
        hb.addTableHeaderCell(_("Number"));
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", TRANS_SORT_BY_NUMBER), _("Number"));
    if(TRANS_SORT_BY_NOTE == sortColumn_)
        hb.addTableHeaderCell(_("Notes"));
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", TRANS_SORT_BY_NOTE), _("Notes"));
    hb.endTableRow();

    // Display the data for each row
    for (const auto& entry : data)
    {
        hb.startTableRow();
        hb.addTableCell(entry.date);
        hb.addTableCellLink(entry.link, entry.account);
        hb.addTableCell(entry.payee);
        hb.addTableCell(entry.status);
        hb.addTableCell(entry.categ, false, true);
        hb.addTableCell(wxGetTranslation(entry.type));
        hb.addMoneyCell(entry.amount);
        hb.addTableCell(entry.num);
        hb.addTableCell(entry.note, false, true);
        hb.endTableRow();
    }

    // work out the total balance for all the data at base rate
    core_->currencyList_.LoadBaseCurrencySettings();

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
    if ( transDialog_->getPayeeCheckBox())
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
