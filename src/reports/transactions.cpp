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
#include "reports/htmlbuilder.h"
#include "util.h"
#include "filtertransdialog.h"
#include "Model_Account.h"
#include "Model_Category.h"
#include "Model_CurrencyHistory.h"
#include "Model_Attachment.h"
#include <algorithm>
#include <vector>

mmReportTransactions::mmReportTransactions(int refAccountID
    , mmFilterTransactionsDialog* transDialog)
    : mmPrintableBase("mmReportTransactions")
    , trans_()
    , m_refAccountID(refAccountID)
    , m_transDialog(transDialog)
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
    bool monoAcc = m_transDialog->getAccountCheckBox() && account;
    const wxString transHeading = monoAcc
        ? wxString::Format(_("Transaction List for Account: %s"), account->ACCOUNTNAME)
        : _("Transaction List ");

    hb.addHeader(2, transHeading);

    hb.addDateNow();
    hb.addLineBreak();

    hb.startSortTable();

    hb.startThead();
    // Display the data Headings
    hb.startTableRow();
    hb.addTableHeaderCell(_("ID"));
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

    std::map<int, double> total; //Store transaction amount with original currency
    std::map<int, double> total_in_base_curr; //Store transactions amount daily converted to base currency

    const Model_Currency::Data* currency = account
        ? Model_Account::currency(account)
        : Model_Currency::GetBaseCurrency();

    const wxString& AttRefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);

    // Display the data for each row
    for (auto& transaction : trans_)
    {
        hb.startTableRow();
        hb.addTableCellLink(wxString::Format("trx:%d", transaction.TRANSID)
            , wxString::Format("%i", transaction.TRANSID));
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

        if (monoAcc)
        {
            const double amount = Model_Checking::balance(transaction, account->ACCOUNTID);
            const double convRate = Model_CurrencyHistory::getDayRate(account->CURRENCYID, transaction.TRANSDATE);
            hb.addCurrencyCell(amount, currency);
            total[currency->CURRENCYID] += amount;
            total_in_base_curr[currency->CURRENCYID] += amount * convRate;
        }
        else
        {
            const auto acc = Model_Account::instance().get(transaction.ACCOUNTID);
            if (acc)
            {
                const Model_Currency::Data* curr = Model_Account::currency(acc);
                const double amount = Model_Checking::balance(transaction, transaction.ACCOUNTID);
                const double convRate = Model_CurrencyHistory::getDayRate(curr->CURRENCYID, transaction.TRANSDATE);
                hb.addCurrencyCell(amount, curr);
                total[curr->CURRENCYID] += amount;
                total_in_base_curr[curr->CURRENCYID] += amount * convRate;
            }
            else
                hb.addTableCell("");
        }
        hb.endTableRow();
    }
    hb.endTbody();

    hb.startTfoot();
    // display the total balance.
    double grand_total = 0;
    for (const auto& curr_total : total)
    {
        const auto curr = Model_Currency::instance().get(curr_total.first);
        const wxString totalStr = Model_Currency::toCurrency(curr_total.second, curr);
        grand_total += total_in_base_curr[curr_total.first];
        const std::vector<wxString> v{ totalStr };
        if (total.size() > 1 
            || (curr->CURRENCY_SYMBOL != Model_Currency::GetBaseCurrency()->CURRENCY_SYMBOL))
            hb.addTotalRow(curr->CURRENCY_SYMBOL, 10, v);
    }
    const wxString totalStr = Model_Currency::toCurrency(grand_total, Model_Currency::GetBaseCurrency());
    const std::vector<wxString> v{ totalStr };
    hb.addTotalRow(_("Grand Total:"), 10, v);

    hb.endTfoot();
    hb.endTable();

    m_transDialog->getDescription(hb);
    hb.endDiv();
    hb.end();

    return hb.getHTMLText();
}

void mmReportTransactions::Run(mmFilterTransactionsDialog* dlg)
{
    const auto splits = Model_Splittransaction::instance().get_all();
    auto categ = m_transDialog->getCategId();
    auto subcateg = m_transDialog->getSubCategId();
    bool similar = !m_transDialog->getSimilarStatus();
    bool category = dlg->getCategoryCheckBox();
    for (const auto& tran : Model_Checking::instance().all()) //TODO: find should be faster
    {
        Model_Checking::Full_Data full_tran(m_refAccountID, tran, splits);
        if (!dlg->checkAll(full_tran, m_refAccountID)) continue;
        full_tran.PAYEENAME = full_tran.real_payee_name(m_refAccountID);
        full_tran.TRANSAMOUNT = tran.TRANSAMOUNT;

        if (category && full_tran.has_split())
        {
            const Model_Account::Data* acc = Model_Account::instance().get(tran.ACCOUNTID);
            const Model_Currency::Data* currency = Model_Currency::instance().get(acc->CURRENCYID);

            full_tran.CATEGNAME.clear();
            double total_amount = 0;
            for (const auto& split : full_tran.m_splits)
            {
                const wxString amt = Model_Currency::toCurrency(split.SPLITTRANSAMOUNT, currency);
                const wxString split_info = wxString::Format("%s = %s | "
                    , Model_Category::full_name(split.CATEGID, split.SUBCATEGID)
                    , amt);

                full_tran.CATEGNAME.Append(split_info);

                if (split.CATEGID != categ) continue;
                if (similar && split.SUBCATEGID != subcateg) continue;

                total_amount += split.SPLITTRANSAMOUNT;
            }
            full_tran.CATEGNAME.RemoveLast(2);
            full_tran.TRANSAMOUNT = total_amount;
        }

        trans_.push_back(full_tran);
    }
    std::stable_sort(trans_.begin(), trans_.end(), SorterByTRANSDATE());
}
