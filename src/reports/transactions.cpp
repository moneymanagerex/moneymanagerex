/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2011, 2012 Stefano Giorgio
 Copyright (C) 2011, 2012, 2015, 2017, 2021 Nikolay Akimov
 Copyright (C) 2021, 2022 Mark Whalley (mark@ipx.co.uk)

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
#include <float.h>

mmReportTransactions::mmReportTransactions(wxSharedPtr<mmFilterTransactionsDialog>& transDialog)
    : mmPrintableBase("Transaction Report")
    , m_transDialog(transDialog)
    , trans_()
{
}

mmReportTransactions::~mmReportTransactions()
{
    if (m_transDialog)
        m_transDialog->Destroy();
}

void mmReportTransactions::displayTotals(std::map<int, double> total, std::map<int, double> total_in_base_curr, int noOfCols)
{
    double grand_total = 0;
    for (const auto& curr_total : total)
    {
        const auto curr = Model_Currency::instance().get(curr_total.first);
        const bool isBaseCurr = (curr->CURRENCY_SYMBOL == Model_Currency::GetBaseCurrency()->CURRENCY_SYMBOL);
        grand_total += total_in_base_curr[curr_total.first];
        if (total.size() > 1 || !isBaseCurr)
        {
            const wxString totalStr_curr = isBaseCurr ? "" : Model_Currency::toCurrency(curr_total.second, curr);
            const wxString totalStr = Model_Currency::toCurrency(total_in_base_curr[curr_total.first], Model_Currency::GetBaseCurrency());
            const std::vector<wxString> v{ totalStr_curr,  totalStr };
            hb.addTotalRow(curr->CURRENCY_SYMBOL, noOfCols, v);
        }
    }
    const wxString totalStr = Model_Currency::toCurrency(grand_total, Model_Currency::GetBaseCurrency());
    const std::vector<wxString> v{ "", totalStr };
    hb.addTotalRow(_("Grand Total:"), noOfCols, v);
}

void mmReportTransactions::UDFCFormatHelper(Model_CustomField::FIELDTYPE type, int ref, wxString data, double val, int scale)
{
    if (type == Model_CustomField::FIELDTYPE::DECIMAL || type == Model_CustomField::FIELDTYPE::INTEGER)
        hb.addMoneyCell(val, scale);
    else if (ref != -1)
    {
        if (type == Model_CustomField::FIELDTYPE::BOOLEAN && !data.empty())
        {
            bool v = wxString("TRUE|true|1").Contains(data);
            hb.addTableCell(v ? "&check;" : "&cross;", false, true);
        } else
            hb.addTableCell(type == Model_CustomField::FIELDTYPE::DATE && !data.empty() ? mmGetDateForDisplay(data) : data);
    }
}

wxString mmReportTransactions::getHTMLText()
{
    Run(m_transDialog);

    wxArrayInt selected_accounts = m_transDialog->mmGetAccountsID();
    wxString accounts_label = _("All Accounts");
    bool allAccounts = true;
    if (m_transDialog->mmIsAccountChecked() && !m_transDialog->mmGetAccountsID().empty()) {
        accounts_label.clear();
        allAccounts = false;
        for (const auto& acc : selected_accounts) {
            Model_Account::Data* a = Model_Account::instance().get(acc);
            accounts_label += (accounts_label.empty() ? "" : ", ") + a->ACCOUNTNAME;
        }
    }

    const wxString extra_style = R"(
table {
  width: 100%;
}
  th.ID, th.Status {
  width: 3%;
}
  th.Color, th.Date, th.Number, th.Type {
  width: 5%;
}
  th.Amount {
  width: 8%;
}
  th.Account, th.Category, th.Payee {
  width: 10%;
}
)";

    hb.init(false, extra_style);
    wxString label = m_transDialog->mmGetLabelString();
     hb.addReportHeader(wxString::Format("%s %s%s", getReportTitle(), !label.IsEmpty() ? ": " : "", label), 
            ((m_transDialog->mmIsRangeChecked()) ? m_transDialog->mmGetStartDay() : 1),
            ((m_transDialog->mmIsRangeChecked()) ? m_transDialog->mmIsFutureIgnored() : false ));
    wxDateTime start,end;
    start.ParseISODate(m_transDialog->mmGetBeginDate());
    end.ParseISODate(m_transDialog->mmGetEndDate());
    hb.DisplayDateHeading(start, end
        , m_transDialog->mmIsRangeChecked() || m_transDialog->mmIsDateRangeChecked());
    hb.DisplayFooter(_("Accounts: ") + accounts_label);

    m_noOfCols = (m_transDialog->mmIsHideColumnsChecked()) ? m_transDialog->mmGetHideColumnsID().GetCount() : 11;
    const wxString& AttRefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
    const int groupBy = m_transDialog->mmGetGroupBy();
    wxString lastSortLabel = "";

    std::map<int, double> total; //Store transaction amount with original currency
    std::map<int, double> total_in_base_curr; //Store transactions amount daily converted to base currency
    std::map<int, double> grand_total; //Grand - Store transaction amount with original currency
    std::map<int, double> grand_total_in_base_curr; //Grad - Store transactions amount daily converted to base currency
    std::map<int, double> grand_total_extrans; //Grand - Store transaction amount with original currency - excluding TRANSFERS
    std::map<int, double> grand_total_in_base_curr_extrans; //Grand - Store transactions amount daily converted to base currency - excluding TRANSFERS

    const wxString RefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
    Model_CustomField::FIELDTYPE UDFC01_Type = Model_CustomField::getUDFCType(RefType, "UDFC01");
    Model_CustomField::FIELDTYPE UDFC02_Type = Model_CustomField::getUDFCType(RefType, "UDFC02");
    Model_CustomField::FIELDTYPE UDFC03_Type = Model_CustomField::getUDFCType(RefType, "UDFC03");
    Model_CustomField::FIELDTYPE UDFC04_Type = Model_CustomField::getUDFCType(RefType, "UDFC04");
    Model_CustomField::FIELDTYPE UDFC05_Type = Model_CustomField::getUDFCType(RefType, "UDFC05");
    int UDFC01_Scale = Model_CustomField::getDigitScale(Model_CustomField::getUDFCProperties(RefType, "UDFC01"));
    int UDFC02_Scale = Model_CustomField::getDigitScale(Model_CustomField::getUDFCProperties(RefType, "UDFC02"));
    int UDFC03_Scale = Model_CustomField::getDigitScale(Model_CustomField::getUDFCProperties(RefType, "UDFC03"));
    int UDFC04_Scale = Model_CustomField::getDigitScale(Model_CustomField::getUDFCProperties(RefType, "UDFC04"));
    int UDFC05_Scale = Model_CustomField::getDigitScale(Model_CustomField::getUDFCProperties(RefType, "UDFC05"));
    // Display the data for each row
    for (auto& transaction : trans_)
    {
        wxString sortLabel = "ALL";
        if (groupBy == mmFilterTransactionsDialog::GROUPBY_ACCOUNT)
            sortLabel = transaction.ACCOUNTNAME;
        else if (groupBy == mmFilterTransactionsDialog::GROUPBY_PAYEE)
            sortLabel = transaction.PAYEENAME;
        else if (groupBy == mmFilterTransactionsDialog::GROUPBY_CATEGORY)
            sortLabel = transaction.CATEGNAME;
        else if (groupBy == mmFilterTransactionsDialog::GROUPBY_TYPE)
            sortLabel = wxGetTranslation(transaction.TRANSCODE);

        if (sortLabel != lastSortLabel)
        {
            if (lastSortLabel != "") 
            {
                hb.endTbody();
                hb.endTable();
                hb.startTable();
                hb.startTbody();
                displayTotals(total, total_in_base_curr, m_noOfCols); 
                hb.endTbody();
                hb.endTable();
                hb.endDiv();
                total.clear();
                total_in_base_curr.clear();
            }
            hb.addDivContainer("shadow");
            if (groupBy > -1) 
                hb.addHeader(2, sortLabel);
            hb.startSortTable();
                hb.startThead();
                    hb.startTableRow();
                        if (showColumnById(0)) hb.addTableHeaderCell(_("ID"), "ID text-right");
                        if (showColumnById(1)) hb.addTableHeaderCell(_("Color"), "Color text-center");
                        if (showColumnById(2)) hb.addTableHeaderCell(_("Date"), "Date");
                        if (showColumnById(3)) hb.addTableHeaderCell(_("Number"), "Number");
                        if (showColumnById(4)) hb.addTableHeaderCell(_("Account"), "Account");
                        if (showColumnById(5)) hb.addTableHeaderCell(_("Payee"), "Payee");
                        if (showColumnById(6)) hb.addTableHeaderCell(_("Status"), "Status text-center");
                        if (showColumnById(7)) hb.addTableHeaderCell(_("Category"), "Category");
                        if (showColumnById(8)) hb.addTableHeaderCell(_("Type"), "Type");
                        if (showColumnById(9)) hb.addTableHeaderCell(_("Amount"), "Amount text-right");
                        if (showColumnById(10)) hb.addTableHeaderCell(_("Notes"), "Notes");
                        const auto& ref_type = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
                        int colNo = 11;
                        for (const auto& udfc_entry : Model_CustomField::UDFC_FIELDS())
                        {
                            if (udfc_entry.empty()) continue;
                            const auto& name = Model_CustomField::getUDFCName(ref_type, udfc_entry);
                            if (showColumnById(colNo++) && name != udfc_entry)
                            {
                                wxString nameCSS = name;
                                switch (Model_CustomField::getUDFCType(ref_type, udfc_entry)) {
                                case Model_CustomField::FIELDTYPE::DECIMAL:
                                case Model_CustomField::FIELDTYPE::INTEGER:
                                    nameCSS.Append(" text-right");
                                    break;
                                case Model_CustomField::FIELDTYPE::BOOLEAN:
                                    nameCSS.Append(" text-center");
                                    break;
                                }
                                hb.addTableHeaderCell(name, nameCSS);
                            }
                        }
                    hb.endTableRow();
                hb.endThead();
            hb.startTbody();
        }
        lastSortLabel = sortLabel;
    
        // If a transfer between two accounts in the list of accounts being reported then we
        // should report both the transfer in and transfer out, i.e. two transactions
        int noOfTrans = 1; 
        if ((Model_Checking::type(transaction) == Model_Checking::TRANSFER) &&
            (allAccounts ||
            ((selected_accounts.Index(transaction.ACCOUNTID) != wxNOT_FOUND)
            && (selected_accounts.Index(transaction.TOACCOUNTID) != wxNOT_FOUND))))
                noOfTrans = 2;

        auto custom_fields_data = Model_CustomFieldData::instance().get_all(Model_Attachment::TRANSACTION);
        while (noOfTrans--)
        {
            hb.startTableRow();
            {
                /*  if ((Model_Checking::type(transaction) == Model_Checking::TRANSFER)
                    && m_transDialog->getTypeCheckBox() && */
                if (showColumnById(0)) {
                    hb.addTableCellLink(wxString::Format("trx:%d", transaction.TRANSID)
                        , wxString::Format("%i", transaction.TRANSID), true);
                }
                if (showColumnById(1)) hb.addColorMarker(getUDColour(transaction.FOLLOWUPID).GetAsString(), true);
                if (showColumnById(2)) hb.addTableCellDate(transaction.TRANSDATE);
                if (showColumnById(3)) hb.addTableCell(transaction.TRANSACTIONNUMBER);
                if (showColumnById(4)) {
                    hb.addTableCellLink(wxString::Format("trxid:%d", transaction.TRANSID)
                        , noOfTrans ? transaction.TOACCOUNTNAME : transaction.ACCOUNTNAME);
                }
                if (showColumnById(5)) hb.addTableCell(noOfTrans ? "< " + transaction.ACCOUNTNAME : transaction.PAYEENAME);
                if (showColumnById(6)) hb.addTableCell(transaction.STATUS, false, true);
                if (showColumnById(7)) hb.addTableCell(transaction.CATEGNAME);
                if (showColumnById(8))
                {
                    if (Model_Checking::foreignTransactionAsTransfer(transaction))
                        hb.addTableCell("< " + wxGetTranslation(transaction.TRANSCODE));
                    else
                        hb.addTableCell(wxGetTranslation(transaction.TRANSCODE));
                }

                Model_Account::Data* acc;
                const Model_Currency::Data* curr;
                acc = Model_Account::instance().get(transaction.ACCOUNTID);
                curr = Model_Account::currency(acc);

                if (acc)
                {
                    double amount = Model_Checking::balance(transaction, acc->ACCOUNTID);
                    if (noOfTrans || (!allAccounts && (selected_accounts.Index(transaction.ACCOUNTID) == wxNOT_FOUND)))
                        amount = -amount;
                    const double convRate = Model_CurrencyHistory::getDayRate(curr->CURRENCYID, transaction.TRANSDATE);
                    if (showColumnById(9)) 
                        if (Model_Checking::status(transaction.STATUS) == Model_Checking::VOID_)
                            hb.addCurrencyCell(Model_Checking::amount(transaction, acc->ACCOUNTID), curr, -1, true);                            
                        else 
                            hb.addCurrencyCell(amount, curr);
                    total[curr->CURRENCYID] += amount;
                    grand_total[curr->CURRENCYID] += amount;
                    total_in_base_curr[curr->CURRENCYID] += amount * convRate;
                    grand_total_in_base_curr[curr->CURRENCYID] += amount * convRate;
                    if (Model_Checking::type(transaction) != Model_Checking::TRANSFER)
                    {
                        grand_total_extrans[curr->CURRENCYID] += amount;
                        grand_total_in_base_curr_extrans[curr->CURRENCYID] += amount * convRate;
                    }
                }
                else
                {
                    wxFAIL_MSG("account for transaction not found");
                    if (showColumnById(9)) hb.addEmptyTableCell();
                }

                // Attachments
                wxString AttachmentsLink = "";
                if (Model_Attachment::instance().NrAttachments(AttRefType, transaction.TRANSID))
                {
                    AttachmentsLink = wxString::Format(R"(<a href = "attachment:%s|%d" target="_blank">%s</a>)",
                        AttRefType, transaction.TRANSID, mmAttachmentManage::GetAttachmentNoteSign());
                }

                // Notes
                if (showColumnById(10)) hb.addTableCell(AttachmentsLink + transaction.NOTES);

                // Custom Fields

                const auto matrix = Model_CustomField::getMatrix(Model_Attachment::TRANSACTION);
                int udfc01_ref_id = matrix.at("UDFC01");
                int udfc02_ref_id = matrix.at("UDFC02");
                int udfc03_ref_id = matrix.at("UDFC03");
                int udfc04_ref_id = matrix.at("UDFC04");
                int udfc05_ref_id = matrix.at("UDFC05");

                transaction.UDFC01_val = -DBL_MAX;
                transaction.UDFC02_val = -DBL_MAX;
                transaction.UDFC03_val = -DBL_MAX;
                transaction.UDFC04_val = -DBL_MAX;
                transaction.UDFC05_val = -DBL_MAX;

                if (custom_fields_data.find(transaction.TRANSID) != custom_fields_data.end()) {
                    const auto& udfcs = custom_fields_data.at(transaction.TRANSID);
                    for (const auto& udfc : udfcs)
                    {
                        if (udfc.FIELDID == udfc01_ref_id) {
                            transaction.UDFC01 = udfc.CONTENT;
                            transaction.UDFC01_val = cleanseNumberStringToDouble(udfc.CONTENT, UDFC01_Scale > 0);
                        }
                        else if (udfc.FIELDID == udfc02_ref_id) {
                            transaction.UDFC02 = udfc.CONTENT;
                            transaction.UDFC02_val = cleanseNumberStringToDouble(udfc.CONTENT, UDFC02_Scale > 0);
                        }
                        else if (udfc.FIELDID == udfc03_ref_id) {
                            transaction.UDFC03 = udfc.CONTENT;
                            transaction.UDFC03_val = cleanseNumberStringToDouble(udfc.CONTENT, UDFC03_Scale > 0);
                        }
                        else if (udfc.FIELDID == udfc04_ref_id) {
                            transaction.UDFC04 = udfc.CONTENT;
                            transaction.UDFC04_val = cleanseNumberStringToDouble(udfc.CONTENT, UDFC04_Scale > 0);
                        }
                        else if (udfc.FIELDID == udfc05_ref_id) {
                            transaction.UDFC05 = udfc.CONTENT;
                            transaction.UDFC05_val = cleanseNumberStringToDouble(udfc.CONTENT, UDFC05_Scale > 0);
                        }
                    }
                }

                if (showColumnById(11))
                    UDFCFormatHelper(UDFC01_Type, udfc01_ref_id, transaction.UDFC01, transaction.UDFC01_val, UDFC01_Scale);
                if (showColumnById(12))
                    UDFCFormatHelper(UDFC02_Type, udfc02_ref_id, transaction.UDFC02, transaction.UDFC02_val, UDFC02_Scale);
                 if (showColumnById(13))
                    UDFCFormatHelper(UDFC03_Type, udfc03_ref_id, transaction.UDFC03, transaction.UDFC03_val, UDFC03_Scale);
                 if (showColumnById(14))
                    UDFCFormatHelper(UDFC04_Type, udfc04_ref_id, transaction.UDFC04, transaction.UDFC04_val, UDFC04_Scale);
                 if (showColumnById(15))
                    UDFCFormatHelper(UDFC05_Type, udfc05_ref_id, transaction.UDFC05, transaction.UDFC05_val, UDFC05_Scale);
            }
            hb.endTableRow();
        }
    }
    if (groupBy > -1)
    {
        hb.endTbody();
        hb.endTable();
        hb.startTable();
        hb.startTbody();
        displayTotals(total, total_in_base_curr, m_noOfCols);
    }  
    hb.endTbody();
    hb.endTable();
    hb.endDiv();

    hb.addDivContainer("shadow");
    {
        hb.startTable();
        {
            // display the total balance 
            hb.startThead();
            {
                hb.startTableRow();
                {
                    hb.addTableHeaderCell(_("All Transactions: Withdrawals, Deposits, and Transfers"));
                    hb.addTableHeaderCell("");
                    hb.addTableHeaderCell("");
                }
                hb.endTableRow();
            }
            hb.endThead();
            hb.startTbody();
            {
                displayTotals(grand_total, grand_total_in_base_curr, 2);
            }
            hb.endTbody();

            // display the total balance (excluding TRANSFERS)
            hb.startThead();
            {
                hb.startTableRow();
                {
                    hb.addTableHeaderCell(_("All Transactions excluding Transfers"));
                    hb.addTableHeaderCell("");
                    hb.addTableHeaderCell("");
                }
                hb.endTableRow();
            }
            hb.endThead();
            hb.startTbody();
            {
                displayTotals(grand_total_extrans, grand_total_in_base_curr_extrans, 2);
            }
            hb.endTbody();
        }
        hb.endTable();
    }
    hb.endDiv();
    hb.addDivContainer("shadow");
    {
        m_transDialog->mmGetDescription(hb);
    }
    hb.endDiv();
    hb.end();

    return hb.getHTMLText();
}

void mmReportTransactions::Run(wxSharedPtr<mmFilterTransactionsDialog>& dlg)
{
    trans_.clear();
    const auto splits = Model_Splittransaction::instance().get_all();
    for (const auto& tran : Model_Checking::instance().all())
    {
        if (!dlg.get()->mmIsRecordMatches(tran, splits)) continue;
        Model_Checking::Full_Data full_tran(tran, splits);

        full_tran.PAYEENAME = full_tran.real_payee_name(full_tran.ACCOUNTID);
        if (full_tran.has_split())
        {
            bool catFilter = dlg.get()->mmIsCategoryChecked();
            bool subCatFilter = dlg.get()->mmIsCategorySubCatChecked();
            const auto& value = dlg.get()->mmGetCategoryPattern();
            wxRegEx pattern("^(" + value + ")$", wxRE_ICASE | wxRE_ADVANCED);

            for (const auto& split : full_tran.m_splits)
            {
                const auto& categ = Model_Category::full_name(split.CATEGID, (!subCatFilter ? split.SUBCATEGID : -1));

                if (!catFilter || pattern.Matches(categ)) {
                    full_tran.CATEGNAME = Model_Category::full_name(split.CATEGID, split.SUBCATEGID);
                    full_tran.TRANSAMOUNT = split.SPLITTRANSAMOUNT;
                    trans_.push_back(full_tran);
                }
            }
        }
        else
            trans_.push_back(full_tran);
    }

    std::stable_sort(trans_.begin(), trans_.end(), SorterByTRANSDATE());
    switch (dlg.get()->mmGetGroupBy())
    {
        case mmFilterTransactionsDialog::GROUPBY_ACCOUNT:
            std::stable_sort(trans_.begin(), trans_.end(), SorterByACCOUNTNAME());
            break;
        case mmFilterTransactionsDialog::GROUPBY_PAYEE:
            std::stable_sort(trans_.begin(), trans_.end(), SorterByPAYEENAME());
            break;   
        case mmFilterTransactionsDialog::GROUPBY_CATEGORY:
            std::stable_sort(trans_.begin(), trans_.end(), SorterByCATEGNAME());
            break;   
        case mmFilterTransactionsDialog::GROUPBY_TYPE:
            std::stable_sort(trans_.begin(), trans_.end(), SorterByTRANSCODE());
            break;   
    }
}

bool mmReportTransactions::showColumnById(int num)
{
    if (m_transDialog->mmIsHideColumnsChecked())
    {
        wxArrayInt columns = m_transDialog->mmGetHideColumnsID();
        return columns.Index(num) == wxNOT_FOUND;
    }
    return true;
}
