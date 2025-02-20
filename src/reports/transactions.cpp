/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2011, 2012 Stefano Giorgio
 Copyright (C) 2011, 2012, 2015, 2017, 2021 Nikolay Akimov
 Copyright (C) 2021 - 2023 Mark Whalley (mark@ipx.co.uk)

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
#include <numeric>
#include <vector>
#include <float.h>

mmReportTransactions::mmReportTransactions(wxSharedPtr<mmFilterTransactionsDialog>& transDialog)
    : mmPrintableBase("Transaction Report")
    , trans_()
    , m_transDialog(transDialog)
{
}

mmReportTransactions::~mmReportTransactions()
{
    if (m_transDialog)
        m_transDialog->Destroy();
}

void mmReportTransactions::displayTotals(std::map<int64, double> total, std::map<int64, double> total_in_base_curr, int noOfCols)
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
    hb.addTotalRow(_t("Grand Total:"), noOfCols, v);
}

void mmReportTransactions::UDFCFormatHelper(Model_CustomField::TYPE_ID type, int64 ref, wxString data, double val, int scale)
{
    if (type == Model_CustomField::TYPE_ID_DECIMAL || type == Model_CustomField::TYPE_ID_INTEGER)
        hb.addMoneyCell(val, scale);
    else if (ref != -1)
    {
        if (type == Model_CustomField::TYPE_ID_BOOLEAN && !data.empty())
        {
            bool v = wxString("TRUE|true|1").Contains(data);
            hb.addTableCell(v ? "&check;" : "&cross;", false, true);
        } else
            hb.addTableCell(type == Model_CustomField::TYPE_ID_DATE && !data.empty() ? mmGetDateTimeForDisplay(data) : data);
    }
}

wxString mmReportTransactions::getHTMLText()
{
    Run(m_transDialog);

    wxArrayInt64 selected_accounts = m_transDialog->mmGetAccountsID();
    wxString accounts_label = _t("All Accounts");
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
    hb.DisplayFooter(_t("Accounts: ") + accounts_label);

    m_noOfCols = (m_transDialog->mmIsHideColumnsChecked()) ? m_transDialog->mmGetHideColumnsID().GetCount() : 11;
    const wxString& AttRefType = Model_Attachment::REFTYPE_STR_TRANSACTION;
    const int groupBy = m_transDialog->mmGetGroupBy();
    const int chart = m_transDialog->mmGetChart();
    wxString lastSortLabel = "";

    std::map<int64, double> total; //Store transaction amount with original currency
    std::map<int64, double> total_in_base_curr; //Store transactions amount daily converted to base currency
    std::map<int64, double> grand_total; //Grand - Store transaction amount with original currency
    std::map<int64, double> grand_total_in_base_curr; //Grad - Store transactions amount daily converted to base currency
    std::map<int64, double> grand_total_extrans; //Grand - Store transaction amount with original currency - excluding TRANSFERS
    std::map<int64, double> grand_total_in_base_curr_extrans; //Grand - Store transactions amount daily converted to base currency - excluding TRANSFERS
    std::map<wxString, double> values_chart; // Store grouped values for chart

    const wxString refType = Model_Attachment::REFTYPE_STR_TRANSACTION;
    static wxArrayString udfc_fields = Model_CustomField::UDFC_FIELDS();
    Model_CustomField::TYPE_ID udfc_type[5];
    int udfc_scale[5];
    for (int i = 0; i < 5; i++) {
        // note: udfc_fields starts with ""
        wxString field = udfc_fields[i+1];
        udfc_type[i] = Model_CustomField::getUDFCType(refType, field);
        udfc_scale[i] = Model_CustomField::getDigitScale(
            Model_CustomField::getUDFCProperties(refType, field)
        );
    }

    // Display the data for each row
    for (auto& transaction : trans_)
    {
        if (!transaction.DELETEDTIME.IsEmpty()) continue;

        wxString sortLabel = "ALL";
        if (groupBy == mmFilterTransactionsDialog::GROUPBY_ACCOUNT)
            sortLabel = transaction.ACCOUNTNAME;
        else if (groupBy == mmFilterTransactionsDialog::GROUPBY_PAYEE)
            sortLabel = transaction.PAYEENAME;
        else if (groupBy == mmFilterTransactionsDialog::GROUPBY_CATEGORY)
            sortLabel = transaction.CATEGNAME;
        else if (groupBy == mmFilterTransactionsDialog::GROUPBY_TYPE)
            sortLabel = wxGetTranslation(transaction.TRANSCODE);
        else if (groupBy == mmFilterTransactionsDialog::GROUPBY_DAY)
            sortLabel = mmGetDateTimeForDisplay(transaction.TRANSDATE);
        else if (groupBy == mmFilterTransactionsDialog::GROUPBY_MONTH)
            sortLabel = Model_Checking::TRANSDATE(transaction).Format("%Y-%m");
        else if (groupBy == mmFilterTransactionsDialog::GROUPBY_YEAR)
            sortLabel = Model_Checking::TRANSDATE(transaction).Format("%Y");

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

                if (chart > -1)
                {
                    double value_chart = std::accumulate(total_in_base_curr.begin(), total_in_base_curr.end(), static_cast<double>(0),
                                                         [](const double previous, decltype(*total_in_base_curr.begin()) p) { return previous + p.second; });
                    values_chart[lastSortLabel] += value_chart;
                }
                total.clear();
                total_in_base_curr.clear();
            }
            hb.addDivContainer("shadow");
            if (groupBy > -1)
                hb.addHeader(3, sortLabel);
            hb.startSortTable();
            hb.startThead();
            hb.startTableRow();
            if (showColumnById(mmFilterTransactionsDialog::COL_ID))
                hb.addTableHeaderCell(_t("ID"), "ID text-right");
            if (showColumnById(mmFilterTransactionsDialog::COL_COLOR))
                hb.addTableHeaderCell(_t("Color"), "Color text-center");
            if (showColumnById(mmFilterTransactionsDialog::COL_DATE))
                hb.addTableHeaderCell(_t("Date"), "Date");
            if (showColumnById(mmFilterTransactionsDialog::COL_TIME))
                hb.addTableHeaderCell(_t("Time"), "Time");
            if (showColumnById(mmFilterTransactionsDialog::COL_NUMBER))
                hb.addTableHeaderCell(_t("Number"), "Number");
            if (showColumnById(mmFilterTransactionsDialog::COL_ACCOUNT))
                hb.addTableHeaderCell(_t("Account"), "Account");
            if (showColumnById(mmFilterTransactionsDialog::COL_PAYEE))
                hb.addTableHeaderCell(_t("Payee"), "Payee");
            if (showColumnById(mmFilterTransactionsDialog::COL_STATUS))
                hb.addTableHeaderCell(_t("Status"), "Status text-center");
            if (showColumnById(mmFilterTransactionsDialog::COL_CATEGORY))
                hb.addTableHeaderCell(_t("Category"), "Category");
            if (showColumnById(mmFilterTransactionsDialog::COL_TAGS))
                hb.addTableHeaderCell(_t("Tags"), "Tags");
            if (showColumnById(mmFilterTransactionsDialog::COL_TYPE))
                hb.addTableHeaderCell(_t("Type"), "Type");
            if (showColumnById(mmFilterTransactionsDialog::COL_AMOUNT))
                hb.addTableHeaderCell(_t("Amount"), "Amount text-right");
            if (showColumnById(mmFilterTransactionsDialog::COL_RATE))
                hb.addTableHeaderCell(_t("FX Rate"), "Rate text-right");
            if (showColumnById(mmFilterTransactionsDialog::COL_NOTES))
                hb.addTableHeaderCell(_t("Notes"), "Notes");
            const auto& ref_type = Model_Attachment::REFTYPE_STR_TRANSACTION;
            int colNo = mmFilterTransactionsDialog::COL_UDFC01;
            for (const auto& udfc_entry : Model_CustomField::UDFC_FIELDS())
            {
                if (udfc_entry.empty()) continue;
                const auto& name = Model_CustomField::getUDFCName(ref_type, udfc_entry);
                if (showColumnById(colNo++) && name != udfc_entry)
                {
                    wxString nameCSS = name;
                    switch (Model_CustomField::getUDFCType(ref_type, udfc_entry)) {
                    case Model_CustomField::TYPE_ID_DECIMAL:
                    case Model_CustomField::TYPE_ID_INTEGER:
                        nameCSS.Append(" text-right");
                        break;
                    case Model_CustomField::TYPE_ID_BOOLEAN:
                        nameCSS.Append(" text-center");
                        break;
                    default: break;
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
        if ((Model_Checking::type_id(transaction) == Model_Checking::TYPE_ID_TRANSFER) &&
            (allAccounts ||
                (std::find(selected_accounts.begin(), selected_accounts.end(), transaction.ACCOUNTID) != selected_accounts.end()
                    && std::find(selected_accounts.begin(), selected_accounts.end(), transaction.TOACCOUNTID) != selected_accounts.end())))
            noOfTrans = 2;

        bool is_time_used = Option::instance().UseTransDateTime();
        const wxString mask = is_time_used ? "%Y-%m-%dT%H:%M:%S" : "%Y-%m-%d";

        auto custom_fields_data = Model_CustomFieldData::instance().get_all(Model_Attachment::REFTYPE_ID_TRANSACTION);
        while (noOfTrans--)
        {
            hb.startTableRow();
            {
                /*  if ((Model_Checking::type_id(transaction) == Model_Checking::TYPE_ID_TRANSFER)
                    && m_transDialog->getTypeCheckBox() && */
                if (showColumnById(mmFilterTransactionsDialog::COL_ID))
                {
                    hb.addTableCellLink(wxString::Format("trx:%lld", transaction.TRANSID)
                        , transaction.displayID, true);
                }
                if (showColumnById(mmFilterTransactionsDialog::COL_COLOR))
                    hb.addColorMarker(getUDColour(transaction.COLOR.GetValue()).GetAsString(), true);
                if (showColumnById(mmFilterTransactionsDialog::COL_DATE))
                {
                    wxDateTime dt;
                    dt.ParseFormat(transaction.TRANSDATE, mask) || dt.ParseDate(transaction.TRANSDATE);
                    hb.addTableCellDate(dt.FormatISODate());
                }
                if (showColumnById(mmFilterTransactionsDialog::COL_TIME))
                    hb.addTableCell(mmGetTimeForDisplay(transaction.TRANSDATE));
                if (showColumnById(mmFilterTransactionsDialog::COL_NUMBER))
                    hb.addTableCell(transaction.TRANSACTIONNUMBER);
                if (showColumnById(mmFilterTransactionsDialog::COL_ACCOUNT))
                {
                    hb.addTableCellLink(wxString::Format("trxid:%lld", transaction.TRANSID)
                        , noOfTrans ? transaction.TOACCOUNTNAME : transaction.ACCOUNTNAME);
                }
                if (showColumnById(mmFilterTransactionsDialog::COL_PAYEE))
                    hb.addTableCell(noOfTrans ? "< " + transaction.ACCOUNTNAME : transaction.PAYEENAME);
                if (showColumnById(mmFilterTransactionsDialog::COL_STATUS))
                    hb.addTableCell(transaction.STATUS, false, true);
                if (showColumnById(mmFilterTransactionsDialog::COL_CATEGORY))
                    hb.addTableCell(transaction.CATEGNAME);
                // Tags
                if (showColumnById(mmFilterTransactionsDialog::COL_TAGS))
                    hb.addTableCell(transaction.TAGNAMES);
                if (showColumnById(mmFilterTransactionsDialog::COL_TYPE))
                {
                    if (Model_Checking::foreignTransactionAsTransfer(transaction))
                        hb.addTableCell("< " + wxGetTranslation(transaction.TRANSCODE));
                    else
                        hb.addTableCell(wxGetTranslation(transaction.TRANSCODE));
                }

                Model_Account::Data* acc;
                acc = Model_Account::instance().get(transaction.ACCOUNTID);

                if (acc)
                {
                    const Model_Currency::Data* curr = Model_Account::currency(acc);
                    double flow = Model_Checking::account_flow(transaction, acc->ACCOUNTID);
                    if (noOfTrans || (!allAccounts && (std::find(selected_accounts.begin(), selected_accounts.end(), transaction.ACCOUNTID) == selected_accounts.end())))
                        flow = -flow;
                    const double convRate = Model_CurrencyHistory::getDayRate(curr->CURRENCYID, transaction.TRANSDATE);
                    if (showColumnById(mmFilterTransactionsDialog::COL_AMOUNT))
                    {
                        if (Model_Checking::status_id(transaction.STATUS) == Model_Checking::STATUS_ID_VOID) {
                            double void_flow = Model_Checking::type_id(transaction.TRANSCODE) == Model_Checking::TYPE_ID_DEPOSIT ? transaction.TRANSAMOUNT : -transaction.TRANSAMOUNT;
                            hb.addCurrencyCell(void_flow, curr, -1, true);
                        }
                        else if (transaction.DELETEDTIME.IsEmpty())
                            hb.addCurrencyCell(flow, curr);
                    }
                    total[curr->CURRENCYID] += flow;
                    grand_total[curr->CURRENCYID] += flow;
                    total_in_base_curr[curr->CURRENCYID] += flow * convRate;
                    grand_total_in_base_curr[curr->CURRENCYID] += flow * convRate;
                    if (Model_Checking::type_id(transaction) != Model_Checking::TYPE_ID_TRANSFER)
                    {
                        grand_total_extrans[curr->CURRENCYID] += flow;
                        grand_total_in_base_curr_extrans[curr->CURRENCYID] += flow * convRate;
                    }
                    if (chart > -1 && groupBy == -1)
                    {
                        values_chart[transaction.TRANSID.ToString()] += (flow * convRate);
                    }
                }
                else
                {
                    wxFAIL_MSG("account for transaction not found");
                    if (showColumnById(mmFilterTransactionsDialog::COL_AMOUNT))
                        hb.addEmptyTableCell();
                }

                // Exchange Rate
                if (showColumnById(mmFilterTransactionsDialog::COL_RATE))
                {
                    if ((Model_Checking::type_id(transaction) == Model_Checking::TYPE_ID_TRANSFER)
                        && (transaction.TRANSAMOUNT != transaction.TOTRANSAMOUNT))
                        hb.addMoneyCell(transaction.TOTRANSAMOUNT / transaction.TRANSAMOUNT);
                    else
                        hb.addEmptyTableCell();
                }

                // Attachments
                wxString AttachmentsLink = "";
                if (Model_Attachment::instance().NrAttachments(AttRefType, transaction.TRANSID))
                {
                    AttachmentsLink = wxString::Format(R"(<a href = "attachment:%s|%lld" target="_blank">%s</a>)",
                        AttRefType, transaction.TRANSID, mmAttachmentManage::GetAttachmentNoteSign());
                }

                // Notes
                if (showColumnById(mmFilterTransactionsDialog::COL_NOTES))
                    hb.addTableCell(AttachmentsLink + transaction.NOTES);

                // Custom Fields

                int64 udfc_id[5];
                for (int i = 0; i < 5; i++) {
                    wxString field = udfc_fields[i+1];
                    udfc_id[i] = Model_CustomField::getUDFCID(refType, field);
                    transaction.UDFC_value[i] = -DBL_MAX;
                }

                if (custom_fields_data.find(transaction.TRANSID) != custom_fields_data.end()) {
                    const auto& udfcs = custom_fields_data.at(transaction.TRANSID);
                    for (const auto& udfc : udfcs) {
                        for (int i = 0; i < 5; i++) {
                            if (udfc.FIELDID == udfc_id[i]) {
                                transaction.UDFC_content[i] = udfc.CONTENT;
                                transaction.UDFC_value[i] = cleanseNumberStringToDouble(
                                    udfc.CONTENT, udfc_scale[i] > 0
                                );
                                break;
                            }
                        }
                    }
                }

                for (int i = 0; i < 5; i++) {
                    if (showColumnById(mmFilterTransactionsDialog::COL_UDFC01 + i))
                        UDFCFormatHelper(
                            udfc_type[i], udfc_id[i],
                            transaction.UDFC_content[i],
                            transaction.UDFC_value[i],
                            udfc_scale[i]
                        );
                }
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
        if (chart > -1)
        {
            double value_chart = std::accumulate(total_in_base_curr.begin(), total_in_base_curr.end(), 0,
                                                 [](const double previous, decltype(*total_in_base_curr.begin()) p) { return previous + p.second; });
            values_chart[lastSortLabel] += value_chart;
        }
    }
    hb.endTbody();
    hb.endTable();
    hb.endDiv();

    // Totals box
    hb.addDivContainer("shadow");
    {
        hb.startTable();
        {
            // display the total balance 
            hb.startThead();
            {
                hb.startTableRow();
                {
                    hb.addTableHeaderCell(_t("All Transactions: Withdrawals, Deposits, and Transfers"));
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
                    hb.addTableHeaderCell(_t("All Transactions excluding Transfers"));
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

    // Chart
    if (chart > -1 && values_chart.size() > 0)
    {
        GraphData gd;
        GraphSeries gs;
        for (const auto& kv : values_chart)
        {
            gd.labels.push_back(kv.first);
            gs.values.push_back(kv.second);
        }
        gd.series.push_back(gs);
        //gd.colors = { mmThemeMetaColour(meta::COLOR_REPORT_DELTA) };
        gd.type = static_cast<GraphData::GraphType>(chart);
        hb.addChart(gd);

        // Statistics
        hb.addDivContainer("shadowGraph");
        {
            hb.startTable();
            {
                hb.startThead();
                {
                    hb.startTableRow();
                    {
                        hb.addTableHeaderCell(_t("Statistics"));
                        hb.addTableHeaderCell("");
                    }
                    hb.endTableRow();
                }
                hb.endThead();
                hb.startTbody();
                {
                    auto statsMin = std::min_element
                    (values_chart.begin(), values_chart.end(),
                    [](const std::pair<wxString, double>& p1, const std::pair<wxString, double>& p2) {
                            return p1.second < p2.second;
                        }
                    );
                    auto statsMax = std::max_element
                    (values_chart.begin(), values_chart.end(),
                    [](const std::pair<wxString, double>& p1, const std::pair<wxString, double>& p2) {
                            return p1.second < p2.second;
                        }
                    );
                    double statsAvg = std::accumulate(values_chart.begin(), values_chart.end(), 0,
                        [](const double previous, decltype(*values_chart.begin()) p) { return previous + p.second; });
                    statsAvg = values_chart.size() > 0 ? statsAvg / values_chart.size() : 0;
                    hb.addTotalRow(_t("Minimum") + " >> " + statsMin->first, 2,
                        std::vector<wxString>{ Model_Currency::toCurrency(statsMin->second, Model_Currency::GetBaseCurrency()) });
                    hb.addTotalRow(_t("Maximum") + " >> " + statsMax->first, 2,
                        std::vector<wxString>{ Model_Currency::toCurrency(statsMax->second, Model_Currency::GetBaseCurrency()) });
                    hb.addTotalRow(_t("Average"), 2,
                        std::vector<wxString>{ Model_Currency::toCurrency(statsAvg, Model_Currency::GetBaseCurrency()) });
                }
                hb.endTbody();
            }
            hb.endTable();
        }
        hb.endDiv();

        // TODO: Fix with proper HTML code reordering instead of jQuery trick
        hb.addText("<script>$('.shadowGraph').insertAfter($('.shadowTitle'));</script>\n");
    }

    // Filters recap
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
    const auto tags = Model_Taglink::instance().get_all(Model_Attachment::REFTYPE_STR_TRANSACTION);
    bool combine_splits = dlg.get()->mmIsCombineSplitsChecked();
    const wxString splitRefType = Model_Attachment::REFTYPE_STR_TRANSACTIONSPLIT;
    for (const auto& tran : Model_Checking::instance().all())
    {
        Model_Checking::Full_Data full_tran(tran, splits, tags);

        full_tran.PAYEENAME = full_tran.real_payee_name(full_tran.ACCOUNTID);
        if (full_tran.has_split())
        {
            Model_Checking::Full_Data single_tran = full_tran;
            single_tran.TRANSAMOUNT = 0;
            int splitIndex = 1;
            bool match = false;
            wxString tranTagnames = full_tran.TAGNAMES;
            for (const auto& split : full_tran.m_splits)
            {
                full_tran.displayID = (wxString::Format("%lld", tran.TRANSID) + "." + wxString::Format("%i", splitIndex++));
                full_tran.CATEGID = split.CATEGID;
                full_tran.CATEGNAME = Model_Category::full_name(split.CATEGID);
                full_tran.TRANSAMOUNT = split.SPLITTRANSAMOUNT;
                full_tran.NOTES = tran.NOTES;
                full_tran.TAGNAMES = tranTagnames;
                Model_Checking::Data splitWithTxnNotes = full_tran;
                Model_Checking::Data splitWithSplitNotes = full_tran;
                splitWithSplitNotes.NOTES = split.NOTES;
                if (dlg.get()->mmIsSplitRecordMatches<Model_Splittransaction>(split)
                    && (dlg.get()->mmIsRecordMatches<Model_Checking>(splitWithSplitNotes, true)
                        || dlg.get()->mmIsRecordMatches<Model_Checking>(splitWithTxnNotes, true)))
                {
                    match = true;
                    full_tran.NOTES.Append((tran.NOTES.IsEmpty() ? "" : " ") + split.NOTES);

                    wxString tagnames;
                    for (const auto& tag : Model_Taglink::instance().get(splitRefType, split.SPLITTRANSID))
                        tagnames.Append(tag.first + " ");
                    if (!tagnames.IsEmpty())
                        full_tran.TAGNAMES.Append((full_tran.TAGNAMES.IsEmpty() ? "" : ", ") + tagnames.Trim());

                    if (!combine_splits) trans_.push_back(full_tran);
                    else single_tran.TRANSAMOUNT += full_tran.TRANSAMOUNT;
                }
            }
            if (match && combine_splits) trans_.push_back(single_tran);
        }
        else if (dlg.get()->mmIsRecordMatches<Model_Checking>(tran)) trans_.push_back(full_tran);
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
    case mmFilterTransactionsDialog::GROUPBY_DAY:
        std::stable_sort(trans_.begin(), trans_.end(), SorterByTRANSDATE());
        break;
    case mmFilterTransactionsDialog::GROUPBY_MONTH:
        std::stable_sort(trans_.begin(), trans_.end(), SorterByTRANSDATE());
        break;
    case mmFilterTransactionsDialog::GROUPBY_YEAR:
        std::stable_sort(trans_.begin(), trans_.end(), SorterByTRANSDATE());
        break;
    }
}

bool mmReportTransactions::showColumnById(int num)
{
    if (num == mmFilterTransactionsDialog::COL_TIME && !Option::instance().UseTransDateTime())
        return false;

    if (m_transDialog->mmIsHideColumnsChecked())
    {
        wxArrayInt columns = m_transDialog->mmGetHideColumnsID();
        return columns.Index(num) == wxNOT_FOUND;
    }
    return true;
}
