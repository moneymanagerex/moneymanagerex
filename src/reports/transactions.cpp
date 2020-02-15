/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2011, 2012 Nikolay & Stefano Giorgio
 Copyright (C) 2015, 2017 Nikolay Akimov

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
#include "attachmentdialog.h"
#include "constants.h"
#include "htmlbuilder.h"
#include "util.h"
#include "model/allmodel.h"
#include <algorithm>
#include <vector>

mmReportTransactions::mmReportTransactions(int refAccountID, mmFilterTransactionsDialog* transDialog)
    : mmPrintableBase("mmReportTransactions")
    , m_refAccountID(refAccountID)
    , m_transDialog(transDialog)
    , trans_()
{
    Run(m_transDialog);
}

mmReportTransactions::~mmReportTransactions()
{
    // incase the user wants to print a report, we maintain the transaction dialog
    // until we are finished with the report.
    m_transDialog->Destroy();
}

wxString mmReportTransactions::getHTMLText()
{
    mmHTMLBuilder hb;
    hb.init();
    hb.addDivContainer();
    const auto account = Model_Account::instance().get(m_transDialog->getAccountID());
    //wxASSERT(account);
    const wxString transHeading = account
        ? wxString::Format(_("Transaction List for Account: %s"), account->ACCOUNTNAME)
        : _("Transaction List ");

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

    double total = 0;
    bool monoAcc = m_transDialog->getAccountCheckBox() && account;

    const Model_Currency::Data* currency = account
        ? Model_Account::currency(account)
        : Model_Currency::GetBaseCurrency();

    const wxString& AttRefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);

    // Display the data for each row
    for (auto& transaction : trans_)
    {
        hb.startTableRow();
        hb.addTableCellDate(transaction.TRANSDATE);
        hb.addTableCellLink(wxString::Format("trxid:%d", transaction.TRANSID)
            , transaction.ACCOUNTNAME);
        hb.addTableCell(transaction.PAYEENAME);
        hb.addTableCell(transaction.STATUS);
        hb.addTableCell(transaction.CATEGNAME);
        if (Model_Checking::foreignTransactionAsTransfer(transaction))
        {
            hb.addTableCell("< " + wxGetTranslation(transaction.TRANSCODE));
        }
        else
        {
            hb.addTableCell(wxGetTranslation(transaction.TRANSCODE));
        }
        hb.addTableCell(transaction.TRANSACTIONNUMBER);

        // Attachments
        wxString AttachmentsLink = "";
        if (Model_Attachment::instance().NrAttachments(AttRefType, transaction.TRANSID))
        {
            AttachmentsLink = wxString::Format("<a href = \"attachment:%s|%d\">%s</a>",
                AttRefType, transaction.TRANSID, mmAttachmentManage::GetAttachmentNoteSign());
        }

        //Notes
        hb.addTableCell(AttachmentsLink + transaction.NOTES);

        // Get the exchange rate for the account
        if (monoAcc)
        {
            double amount = Model_Checking::balance(transaction, account->ACCOUNTID);
            hb.addCurrencyCell(amount, currency);
            total += amount;
        }
        else
        {
            const auto acc = Model_Account::instance().get(transaction.ACCOUNTID);
            if (acc)
            {
                const Model_Currency::Data* curr = Model_Account::currency(acc);
                double convRate = 1;
                if (curr)
                    convRate = curr->BASECONVRATE;

                double amount = Model_Checking::balance(transaction
                    , transaction.ACCOUNTID) * convRate;
                hb.addCurrencyCell(amount, curr);
                total += amount * convRate;
            }
            else
                hb.addTableCell("");
        }
        hb.endTableRow();
    }
    hb.endTbody();

    // display the total balance.
    const wxString totalStr = Model_Currency::toCurrency(total
        , (monoAcc ? Model_Account::currency(account) : Model_Currency::GetBaseCurrency()));
    const std::vector<wxString> v{ totalStr };
    hb.addTotalRow(_("Total Amount: "), 9, v);

    hb.endTable();

    m_transDialog->getDescription(hb);
    hb.endDiv();
    hb.end();

	return hb.getHTMLText();
}

void mmReportTransactions::Run(mmFilterTransactionsDialog* dlg)
{
    const auto splits = Model_Splittransaction::instance().get_all();
    for (const auto& tran : Model_Checking::instance().all()) //TODO: find should be faster
    {
        if (!dlg->checkAll(tran, m_refAccountID, splits)) continue;
        Model_Checking::Full_Data full_tran(tran, splits);
        full_tran.PAYEENAME = full_tran.real_payee_name(m_refAccountID);
        if (m_transDialog->getCategoryCheckBox() && full_tran.has_split()) 
        {
            full_tran.CATEGNAME.clear();
            full_tran.TRANSAMOUNT = 0;
            for (const auto& split : full_tran.m_splits)
            {
                const wxString split_info = wxString::Format("%s = %s | "
                    , Model_Category::full_name(split.CATEGID, split.SUBCATEGID)
                    , wxString::Format("%.2f", split.SPLITTRANSAMOUNT));
                full_tran.CATEGNAME.Append(split_info);
                if (split.CATEGID != m_transDialog->getCategId() ) continue;
                if (split.SUBCATEGID != m_transDialog->getSubCategId() 
                    && !m_transDialog->getSimilarStatus()) continue;

                full_tran.TRANSAMOUNT += split.SPLITTRANSAMOUNT;
            }
            full_tran.CATEGNAME.RemoveLast(2);
        }

        trans_.push_back(full_tran);
    }
    std::stable_sort(trans_.begin(), trans_.end(), SorterByTRANSDATE());
}
