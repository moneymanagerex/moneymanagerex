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

#include <algorithm>
#include <numeric>
#include <vector>
#include <float.h>

#include "base/constants.h"
#include "util/_util.h"

#include "model/_all.h"
#include "dialog/AttachmentDialog.h"

#include "htmlbuilder.h"
#include "TrxReport.h"

TrxReport::TrxReport(wxSharedPtr<TrxFilterDialog>& transDialog)
    : ReportBase("Transaction Report")
    , trx_xa()
    , m_transDialog(transDialog)
{
}

TrxReport::~TrxReport()
{
    if (m_transDialog)
        m_transDialog->Destroy();
}

void TrxReport::displayTotals(const std::map<int64, double>& total, std::map<int64, double>& total_in_base_curr, int noOfCols)
{
    double grand_total = 0;
    for (const auto& [curr_id, curr_total]: total)
    {
        const CurrencyData* curr = CurrencyModel::instance().get_id_data_n(curr_id);
        const bool isBaseCurr = (curr->m_symbol == CurrencyModel::instance().get_base_data_n()->m_symbol);
        grand_total += total_in_base_curr[curr_id];
        if (total.size() > 1 || !isBaseCurr)
        {
            const wxString totalStr_curr = isBaseCurr ? "" : CurrencyModel::instance().toCurrency(curr_total, curr);
            const wxString totalStr = CurrencyModel::instance().toCurrency(total_in_base_curr[curr_id], CurrencyModel::instance().get_base_data_n());
            hb.addTotalRow(curr->m_symbol, noOfCols, { totalStr_curr,  totalStr });
        }
    }
    const wxString totalStr = CurrencyModel::instance().toCurrency(grand_total, CurrencyModel::instance().get_base_data_n());
    const std::vector<wxString> v{ "", totalStr };
    hb.addTotalRow(_t("Grand Total:"), noOfCols, v);
}

void TrxReport::UDFCFormatHelper(FieldTypeN type, int64 ref, wxString data, double val, int scale)
{
    if (type.id_n() == FieldTypeN::e_decimal || type.id_n() == FieldTypeN::e_integer) {
        hb.addMoneyCell(val, scale);
    }
    else if (ref != -1) {
        if (type.id_n() == FieldTypeN::e_boolean && !data.empty()) {
            bool v = wxString("TRUE|true|1").Contains(data);
            hb.addTableCell(v ? "&check;" : "&cross;", false, true);
        }
        else {
            hb.addTableCell(type.id_n() == FieldTypeN::e_date && !data.empty()
                ? mmGetDateTimeForDisplay(data)
                : data
            );
        }
    }
}

wxString TrxReport::getHTMLText()
{
    Run(m_transDialog);

    wxArrayInt64 selected_accounts = m_transDialog->mmGetAccountsID();
    wxString accounts_label = _t("All Accounts");
    bool allAccounts = true;
    if (m_transDialog->mmIsAccountChecked() && !m_transDialog->mmGetAccountsID().empty()) {
        accounts_label.clear();
        allAccounts = false;
        for (const auto& acc : selected_accounts) {
            const AccountData* a = AccountModel::instance().get_id_data_n(acc);
            accounts_label += (accounts_label.empty() ? "" : ", ") + a->m_name;
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
    hb.addReportHeader(
        wxString::Format(
            "%s %s%s",
            getTitle(),
            !label.IsEmpty() ? ": " : "",
            label
        ),
        ((m_transDialog->mmIsRangeChecked()) ? m_transDialog->mmGetStartDay() : 1),
        ((m_transDialog->mmIsRangeChecked()) ? m_transDialog->mmIsFutureIgnored() : false )
    );
    wxDateTime start,end;
    start.ParseISODate(m_transDialog->mmGetBeginDate());
    end.ParseISODate(m_transDialog->mmGetEndDate());
    hb.displayDateHeading(start, end
        , m_transDialog->mmIsRangeChecked() || m_transDialog->mmIsDateRangeChecked());
    hb.displayFooter(_t("Accounts: ") + accounts_label);

    m_noOfCols = (m_transDialog->mmIsHideColumnsChecked()) ? m_transDialog->mmGetHideColumnsID().GetCount() : 11;
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

    static wxArrayString udfc_fields = FieldModel::UDFC_FIELDS();
    FieldTypeN udfc_type[5];
    int udfc_scale[5];
    for (int i = 0; i < 5; i++) {
        // note: udfc_fields starts with ""
        wxString field = udfc_fields[i+1];
        udfc_type[i] = FieldModel::instance().get_udfc_type_n(TrxModel::s_ref_type, field);
        udfc_scale[i] = FieldModel::getDigitScale(
            FieldModel::instance().get_udfc_properties_n(TrxModel::s_ref_type, field)
        );
    }

    // Display the data for each row
    for (auto& trx_dx : trx_xa) {
        if (trx_dx.is_deleted())
            continue;

        wxString sortLabel = "ALL";
        if (groupBy == TrxFilterDialog::GROUPBY_ACCOUNT)
            sortLabel = trx_dx.ACCOUNTNAME;
        else if (groupBy == TrxFilterDialog::GROUPBY_PAYEE)
            sortLabel = trx_dx.PAYEENAME;
        else if (groupBy == TrxFilterDialog::GROUPBY_CATEGORY)
            sortLabel = trx_dx.CATEGNAME;
        else if (groupBy == TrxFilterDialog::GROUPBY_TYPE)
            sortLabel = wxGetTranslation(trx_dx.m_type.name());
        else if (groupBy == TrxFilterDialog::GROUPBY_DAY)
            sortLabel = mmGetDateTimeForDisplay(trx_dx.m_date_time.isoDateTime());
        else if (groupBy == TrxFilterDialog::GROUPBY_MONTH)
            sortLabel = trx_dx.m_date_time.getDateTime().Format("%Y-%m");
        else if (groupBy == TrxFilterDialog::GROUPBY_YEAR)
            sortLabel = trx_dx.m_date_time.getDateTime().Format("%Y");

        if (sortLabel != lastSortLabel) {
            if (lastSortLabel != "") {
                hb.endTbody();
                hb.endTable();
                hb.startTable();
                hb.startTbody();
                displayTotals(total, total_in_base_curr, m_noOfCols);
                hb.endTbody();
                hb.endTable();
                hb.endDiv();

                if (chart > -1) {
                    double value_chart = std::accumulate(
                        total_in_base_curr.begin(), total_in_base_curr.end(),
                        static_cast<double>(0),
                        [](double previous, const auto& p) { return previous + p.second; }
                    );
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
            if (showColumnById(TrxFilterDialog::COL_ID))
                hb.addTableHeaderCell(_t("ID"), "ID text-right");
            if (showColumnById(TrxFilterDialog::COL_COLOR))
                hb.addTableHeaderCell(_t("Color"), "Color text-center");
            if (showColumnById(TrxFilterDialog::COL_DATE))
                hb.addTableHeaderCell(_t("Date"), "Date");
            if (showColumnById(TrxFilterDialog::COL_TIME))
                hb.addTableHeaderCell(_t("Time"), "Time");
            if (showColumnById(TrxFilterDialog::COL_NUMBER))
                hb.addTableHeaderCell(_t("Number"), "Number");
            if (showColumnById(TrxFilterDialog::COL_ACCOUNT))
                hb.addTableHeaderCell(_t("Account"), "Account");
            if (showColumnById(TrxFilterDialog::COL_PAYEE))
                hb.addTableHeaderCell(_t("Payee"), "Payee");
            if (showColumnById(TrxFilterDialog::COL_STATUS))
                hb.addTableHeaderCell(_t("Status"), "Status text-center");
            if (showColumnById(TrxFilterDialog::COL_CATEGORY))
                hb.addTableHeaderCell(_t("Category"), "Category");
            if (showColumnById(TrxFilterDialog::COL_TAGS))
                hb.addTableHeaderCell(_t("Tags"), "Tags");
            if (showColumnById(TrxFilterDialog::COL_TYPE))
                hb.addTableHeaderCell(_t("Type"), "Type");
            if (showColumnById(TrxFilterDialog::COL_AMOUNT))
                hb.addTableHeaderCell(_t("Amount"), "Amount text-right");
            if (showColumnById(TrxFilterDialog::COL_RATE))
                hb.addTableHeaderCell(_t("FX Rate"), "Rate text-right");
            if (showColumnById(TrxFilterDialog::COL_NOTES))
                hb.addTableHeaderCell(_t("Notes"), "Notes");
            int colNo = TrxFilterDialog::COL_UDFC01;
            for (const auto& ucfd : FieldModel::UDFC_FIELDS()) {
                if (ucfd.empty())
                    continue;
                const auto& name = FieldModel::instance().get_udfc_name_n(TrxModel::s_ref_type, ucfd);
                if (showColumnById(colNo++) && name != ucfd) {
                    wxString nameCSS = name;
                    switch (FieldModel::instance().get_udfc_type_n(TrxModel::s_ref_type, ucfd).id_n()) {
                    case FieldTypeN::e_decimal:
                    case FieldTypeN::e_integer:
                        nameCSS.Append(" text-right");
                        break;
                    case FieldTypeN::e_boolean:
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
        if (trx_dx.is_transfer() && (allAccounts || (
            std::find(selected_accounts.begin(), selected_accounts.end(),
                trx_dx.m_account_id
            ) != selected_accounts.end() &&
            std::find(selected_accounts.begin(), selected_accounts.end(),
                trx_dx.m_to_account_id_n
            ) != selected_accounts.end()
        )))
            noOfTrans = 2;

        auto trxId_fvA_m = FieldValueModel::instance().find_refType_mRefId(
            TrxModel::s_ref_type
        );
        while (noOfTrans--) {
            hb.startTableRow();
            {
                /*  if ((TrxModel::type_id(trx_dx) == TrxModel::TYPE_ID_TRANSFER)
                    && m_transDialog->getTypeCheckBox() && */
                if (showColumnById(TrxFilterDialog::COL_ID)) {
                    hb.addTableCellLink(
                        wxString::Format("trx:%lld", trx_dx.m_id),
                        trx_dx.displayID, true
                    );
                }
                if (showColumnById(TrxFilterDialog::COL_COLOR))
                    hb.addColorMarker(getUDColour(trx_dx.m_color.GetValue()).GetAsString(), true);
                if (showColumnById(TrxFilterDialog::COL_DATE)) {
                    hb.addTableCellDate(trx_dx.m_date().isoDate());
                }
                if (showColumnById(TrxFilterDialog::COL_TIME))
                    hb.addTableCell(mmGetTimeForDisplay(trx_dx.m_date_time.isoDateTime()));
                if (showColumnById(TrxFilterDialog::COL_NUMBER))
                    hb.addTableCell(trx_dx.m_number);
                if (showColumnById(TrxFilterDialog::COL_ACCOUNT)) {
                    hb.addTableCellLink(wxString::Format("trxid:%lld", trx_dx.m_id)
                        , noOfTrans ? trx_dx.TOACCOUNTNAME : trx_dx.ACCOUNTNAME);
                }
                if (showColumnById(TrxFilterDialog::COL_PAYEE))
                    hb.addTableCell(noOfTrans ? "< " + trx_dx.ACCOUNTNAME : trx_dx.PAYEENAME);
                if (showColumnById(TrxFilterDialog::COL_STATUS))
                    hb.addTableCell(trx_dx.m_status.key(), false, true);
                if (showColumnById(TrxFilterDialog::COL_CATEGORY))
                    hb.addTableCell(trx_dx.CATEGNAME);
                // Tags
                if (showColumnById(TrxFilterDialog::COL_TAGS))
                    hb.addTableCell(trx_dx.TAGNAMES);
                if (showColumnById(TrxFilterDialog::COL_TYPE)) {
                    if (TrxModel::is_foreignAsTransfer(trx_dx))
                        hb.addTableCell("< " + wxGetTranslation(trx_dx.m_type.name()));
                    else
                        hb.addTableCell(wxGetTranslation(trx_dx.m_type.name()));
                }

                const AccountData* account_n = AccountModel::instance().get_id_data_n(trx_dx.m_account_id);

                if (account_n) {
                    const CurrencyData* currency_n = AccountModel::instance().get_data_currency_p(*account_n);
                    double flow = trx_dx.account_flow(account_n->m_id);
                    if (noOfTrans || (!allAccounts && (std::find(selected_accounts.begin(), selected_accounts.end(), trx_dx.m_account_id) == selected_accounts.end())))
                        flow = -flow;
                    const double convRate = CurrencyHistoryModel::instance().get_id_date_rate(
                        currency_n->m_id,
                        trx_dx.m_date()
                    );
                    if (showColumnById(TrxFilterDialog::COL_AMOUNT)) {
                        if (trx_dx.is_void()) {
                            double void_flow = trx_dx.is_deposit() ? trx_dx.m_amount : -trx_dx.m_amount;
                            hb.addCurrencyCell(void_flow, currency_n, -1, true);
                        }
                        else if (!trx_dx.is_deleted())
                            hb.addCurrencyCell(flow, currency_n);
                    }
                    total[currency_n->m_id] += flow;
                    grand_total[currency_n->m_id] += flow;
                    total_in_base_curr[currency_n->m_id] += flow * convRate;
                    grand_total_in_base_curr[currency_n->m_id] += flow * convRate;
                    if (!trx_dx.is_transfer()) {
                        grand_total_extrans[currency_n->m_id] += flow;
                        grand_total_in_base_curr_extrans[currency_n->m_id] += flow * convRate;
                    }
                    if (chart > -1 && groupBy == -1) {
                        values_chart[trx_dx.m_id.ToString()] += (flow * convRate);
                    }
                }
                else {
                    wxFAIL_MSG("account for trx_dx not found");
                    if (showColumnById(TrxFilterDialog::COL_AMOUNT))
                        hb.addEmptyTableCell();
                }

                // Exchange Rate
                if (showColumnById(TrxFilterDialog::COL_RATE)) {
                    if (trx_dx.is_transfer() && trx_dx.m_amount != trx_dx.m_to_amount) {
                        hb.addMoneyCell(trx_dx.m_to_amount / trx_dx.m_amount);
                    }
                    else {
                        hb.addEmptyTableCell();
                    }
                }

                // Attachments
                wxString AttachmentsLink = "";
                if (AttachmentModel::instance().find_ref_c(
                    TrxModel::s_ref_type, trx_dx.m_id
                )) {
                    AttachmentsLink = wxString::Format(R"(<a href = "attachment:%s|%lld" target="_blank">%s</a>)",
                        TrxModel::s_ref_type.name_n(), trx_dx.m_id,
                        mmAttachmentManage::GetAttachmentNoteSign()
                    );
                }

                // Notes
                if (showColumnById(TrxFilterDialog::COL_NOTES))
                    hb.addTableCell(AttachmentsLink + trx_dx.m_notes);

                // Custom Fields

                int64 udfc_id[5];
                for (int i = 0; i < 5; i++) {
                    wxString field = udfc_fields[i+1];
                    udfc_id[i] = FieldModel::instance().get_udfc_id_n(TrxModel::s_ref_type, field);
                    trx_dx.UDFC_value[i] = -DBL_MAX;
                }

                if (trxId_fvA_m.find(trx_dx.m_id) != trxId_fvA_m.end()) {
                    for (const auto& fv_d : trxId_fvA_m.at(trx_dx.m_id)) {
                        for (int i = 0; i < 5; i++) {
                            if (fv_d.m_field_id == udfc_id[i]) {
                                trx_dx.UDFC_content[i] = fv_d.m_content;
                                trx_dx.UDFC_value[i] = cleanseNumberStringToDouble(
                                    fv_d.m_content, udfc_scale[i] > 0
                                );
                                break;
                            }
                        }
                    }
                }

                for (int i = 0; i < 5; i++) {
                    if (showColumnById(TrxFilterDialog::COL_UDFC01 + i))
                        UDFCFormatHelper(
                            udfc_type[i], udfc_id[i],
                            trx_dx.UDFC_content[i],
                            trx_dx.UDFC_value[i],
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
            double value_chart = std::accumulate(total_in_base_curr.begin(), total_in_base_curr.end(), 0.0,
                                                 [](double previous, const auto& p) { return previous + p.second; });
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
        for (const auto& [k, v] : values_chart)
        {
            gd.labels.push_back(k);
            gs.values.push_back(v);
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
                    [](const auto& p1, const auto& p2) {
                        return p1.second < p2.second;
                    });
                    auto statsMax = std::max_element
                    (values_chart.begin(), values_chart.end(),
                    [](const auto& p1, const auto& p2) {
                        return p1.second < p2.second;
                    });
                    double statsAvg = std::accumulate(values_chart.begin(), values_chart.end(), 0,
                        [](double previous, const auto & p) { return previous + p.second; });
                    statsAvg = values_chart.size() > 0 ? statsAvg / values_chart.size() : 0;
                    hb.addTotalRow(_t("Minimum") + " >> " + statsMin->first, 2,
                        std::vector<wxString>{ CurrencyModel::instance().toCurrency(statsMin->second, CurrencyModel::instance().get_base_data_n()) });
                    hb.addTotalRow(_t("Maximum") + " >> " + statsMax->first, 2,
                        std::vector<wxString>{ CurrencyModel::instance().toCurrency(statsMax->second, CurrencyModel::instance().get_base_data_n()) });
                    hb.addTotalRow(_t("Average"), 2,
                        std::vector<wxString>{ CurrencyModel::instance().toCurrency(statsAvg, CurrencyModel::instance().get_base_data_n()) });
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

void TrxReport::Run(wxSharedPtr<TrxFilterDialog>& dlg)
{
    trx_xa.clear();
    const auto trxId_tpA_m = TrxSplitModel::instance().find_all_mTrxId();
    const auto trxId_glA_m = TagLinkModel::instance().find_refType_mRefId(
        TrxModel::s_ref_type
    );
    bool combine_splits = dlg.get()->mmIsCombineSplitsChecked();
    for (const auto& trx_d : TrxModel::instance().find_all()) {
        TrxModel::DataExt trx_dx(trx_d, trxId_tpA_m, trxId_glA_m);
        trx_dx.PAYEENAME = trx_dx.real_payee_name(trx_dx.m_account_id);
        if (trx_dx.has_split()) {
            TrxModel::DataExt single_tran = trx_dx;
            single_tran.m_amount = 0;
            int splitIndex = 1;
            bool match = false;
            wxString tranTagnames = trx_dx.TAGNAMES;
            for (const auto& tp_d : trx_dx.m_tp_a) {
                trx_dx.displayID       = wxString::Format("%lld", trx_d.m_id) + "." +
                    wxString::Format("%i", splitIndex++);
                trx_dx.m_category_id_n = tp_d.m_category_id;
                trx_dx.CATEGNAME       = CategoryModel::instance().get_id_fullname(tp_d.m_category_id);
                trx_dx.m_amount        = tp_d.m_amount;
                trx_dx.m_notes         = trx_d.m_notes;
                trx_dx.TAGNAMES        = tranTagnames;

                TrxData trx_trx_d = trx_dx;
                TrxData trx_tp_d = trx_dx;
                trx_tp_d.m_notes = tp_d.m_notes;
                if (dlg.get()->mmIsSplitRecordMatches<TrxSplitModel>(tp_d) && (
                    dlg.get()->mmIsRecordMatches<TrxModel>(trx_tp_d, true) ||
                    dlg.get()->mmIsRecordMatches<TrxModel>(trx_trx_d, true)
                )) {
                    match = true;
                    trx_dx.m_notes.Append((trx_d.m_notes.IsEmpty() ? "" : " ") + tp_d.m_notes);

                    wxString tagnames;
                    for (const auto& [tag_name, _] : TagLinkModel::instance().find_ref_mTagName(
                        TrxSplitModel::s_ref_type, tp_d.m_id
                    )) {
                        tagnames.Append(tag_name + " ");
                    }
                    if (!tagnames.IsEmpty())
                        trx_dx.TAGNAMES.Append(
                            (trx_dx.TAGNAMES.IsEmpty() ? "" : ", ") +
                            tagnames.Trim()
                        );

                    if (!combine_splits)
                        trx_xa.push_back(trx_dx);
                    else
                        single_tran.m_amount += trx_dx.m_amount;
                }
            }
            if (match && combine_splits)
                trx_xa.push_back(single_tran);
        }
        else if (dlg.get()->mmIsRecordMatches<TrxModel>(trx_d))
            trx_xa.push_back(trx_dx);
    }

    std::stable_sort(trx_xa.begin(), trx_xa.end(), TrxData::SorterByDateTime());
    switch (dlg.get()->mmGetGroupBy())
    {
    case TrxFilterDialog::GROUPBY_ACCOUNT:
        std::stable_sort(trx_xa.begin(), trx_xa.end(), TrxModel::SorterByACCOUNTNAME());
        break;
    case TrxFilterDialog::GROUPBY_PAYEE:
        std::stable_sort(trx_xa.begin(), trx_xa.end(), TrxModel::SorterByPAYEENAME());
        break;
    case TrxFilterDialog::GROUPBY_CATEGORY:
        std::stable_sort(trx_xa.begin(), trx_xa.end(), TrxModel::SorterByCATEGNAME());
        break;
    case TrxFilterDialog::GROUPBY_TYPE:
        std::stable_sort(trx_xa.begin(), trx_xa.end(), TrxData::SorterByType());
        break;
    case TrxFilterDialog::GROUPBY_DAY:
        std::stable_sort(trx_xa.begin(), trx_xa.end(), TrxData::SorterByDateTime());
        break;
    case TrxFilterDialog::GROUPBY_MONTH:
        std::stable_sort(trx_xa.begin(), trx_xa.end(), TrxData::SorterByDateTime());
        break;
    case TrxFilterDialog::GROUPBY_YEAR:
        std::stable_sort(trx_xa.begin(), trx_xa.end(), TrxData::SorterByDateTime());
        break;
    }
}

bool TrxReport::showColumnById(int num)
{
    if (num == TrxFilterDialog::COL_TIME && !PrefModel::instance().UseTransDateTime())
        return false;

    if (m_transDialog->mmIsHideColumnsChecked()) {
        wxArrayInt columns = m_transDialog->mmGetHideColumnsID();
        return columns.Index(num) == wxNOT_FOUND;
    }
    return true;
}
