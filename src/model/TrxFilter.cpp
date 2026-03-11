/*******************************************************
Copyright (C) 2021-2024 Mark Whalley (mark@ipx.co.uk)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
********************************************************/

// TODO: Consider how we can expand and consolidate this into the Transaction filter dialog

#include "TrxFilter.h"

#include "dialog/AttachmentDialog.h"
#include "report/htmlbuilder.h"

TrxFilter::TrxFilter()
{
    this->clear();
}

void TrxFilter::clear()
{
    m_filter_date = false;
    m_filter_account = false;
    m_filter_payee = false;
    m_filter_category = false;
}

void TrxFilter::setDateRange(const mmDateRange2& date_range)
{
    m_filter_date = true;
    m_start_date = date_range.rangeStartIsoStartN();
    m_end_date = date_range.rangeEndIsoEndN();
}
void TrxFilter::setDateRange(wxDateTime startDate, wxDateTime endDate)
{
    m_filter_date = true;
    if (startDate.FormatISOTime() == "00:00:00")
        m_start_date = startDate.FormatISODate();
    else
        m_start_date = startDate.FormatISOCombined();

    if (!PrefModel::instance().UseTransDateTime())
        endDate = mmDateRange::getDayEnd(endDate);

    m_end_date = endDate.FormatISOCombined();
}

void TrxFilter::setAccountList(wxSharedPtr<wxArrayString> accountList)
{
    if (accountList) {
        m_account_a.clear();
        for (const auto &entry : *accountList) {
            const auto account = AccountModel::instance().get_name_data_n(entry);
            if (account)
                m_account_a.push_back(account->m_id);
        }
        m_filter_account = true;
    }
}

void TrxFilter::setPayeeList(const wxArrayInt64& payeeList)
{
    m_filter_payee = true;
    m_payee_a = payeeList;
}

void TrxFilter::setCategoryList(const wxArrayInt64 &categoryList)
{
    m_filter_category = true;
    m_category_a = categoryList;
}

template<class MODEL, class DATA>
bool TrxFilter::checkCategory(
    const DATA& tran,
    const std::map<int64, typename MODEL::TrxSplitDataA> & splits
) {
    const auto it = splits.find(tran.id());
    if (it == splits.end()) {
        for (auto it2 : m_category_a) {
            if (it2 == tran.m_category_id_n)
                return true;
        }
    }
    else {
        for (const auto& tp_d : it->second) {
            for (auto it2 : m_category_a) {
                if (it2 == tp_d.m_category_id)
                    return true;
            }
        }

    }
    return false;
}

bool TrxFilter::mmIsRecordMatches(
    const TrxData &trx_d,
    const std::map<int64, TrxSplitModel::DataA>& split
) {
    bool ok = true;
    wxString strDate = TrxModel::getTransDateTime(trx_d).FormatISOCombined();
    if (m_filter_account
        && (std::find(m_account_a.begin(), m_account_a.end(), trx_d.m_account_id) == m_account_a.end())
        && (std::find(m_account_a.begin(), m_account_a.end(), trx_d.m_to_account_id_n) == m_account_a.end()))
        ok = false;
    else if (m_filter_date && ((strDate < m_start_date) || (strDate > m_end_date)))
        ok = false;
    else if (m_filter_payee && (std::find(m_payee_a.begin(), m_payee_a.end(), trx_d.m_payee_id_n) == m_payee_a.end()))
        ok = false;
    else if (m_filter_category && !checkCategory<TrxModel>(trx_d, split))
        ok = false;
    return ok;
}

wxString TrxFilter::getHTML()
{
    mmHTMLBuilder hb;
    m_trans.clear();
    const auto splits = TrxSplitModel::instance().get_all_id();
    const auto trxId_glA_m = TagLinkModel::instance().find_refType_mRefId(TrxModel::s_ref_type);
    //TODO: find should be faster
    for (const auto& trx_d : TrxModel::instance().find_all()) {
        if (!mmIsRecordMatches(trx_d, splits)) continue;
        TrxModel::Full_Data full_tran(trx_d, splits, trxId_glA_m);

        full_tran.PAYEENAME = full_tran.real_payee_name(full_tran.m_account_id);
        if (full_tran.has_split()) {
            bool found = true;
            for (const auto& tp_d : full_tran.m_splits) {
                if (m_filter_category) {
                    found = false;

                    for (const auto& it : m_category_a) {
                        if (it == tp_d.m_category_id) {
                            found = true;
                            break;
                        }
                    }
                }

                if (found) {
                    full_tran.CATEGNAME = CategoryModel::instance().full_name(tp_d.m_category_id);
                    full_tran.m_amount = tp_d.m_amount;
                    full_tran.m_notes.Append((trx_d.m_notes.IsEmpty() ? "" : " ") + tp_d.m_notes);
                    m_trans.push_back(full_tran);
                }
            }
        } else
            m_trans.push_back(full_tran);
    }

    std::stable_sort(m_trans.begin(), m_trans.end(), TrxData::SorterByTRANSDATE());

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
    hb.addReportHeader(_t("Transaction Details"), 1, false);

    hb.addDivContainer();
    hb.addTableCellLink("back:",wxString::Format("<< %s", _t("Back")));
    hb.endDiv();
    hb.addDivContainer("shadow");
    hb.startSortTable();
    hb.startThead();
    hb.startTableRow();
    hb.addTableHeaderCell(_t("ID"), "ID text-right");
    hb.addTableHeaderCell(_t("Color"), "Color text-center");
    hb.addTableHeaderCell(_t("Date"), "Date");
    hb.addTableHeaderCell(_t("Number"), "Number");
    hb.addTableHeaderCell(_t("Account"), "Account");
    hb.addTableHeaderCell(_t("Payee"), "Payee");
    hb.addTableHeaderCell(_t("Status"), "Status text-center");
    hb.addTableHeaderCell(_t("Category"), "Category");
    hb.addTableHeaderCell(_t("Type"), "Type");
    hb.addTableHeaderCell(_t("Amount"), "Amount text-right");
    hb.addTableHeaderCell(_t("Notes"), "Notes");
    hb.endTableRow();
    hb.endThead();
    hb.startTbody();
    // Display the data for each row
    for (auto& trx_xd : m_trans) {
        hb.startTableRow();
        hb.addTableCellLink(wxString::Format("trx:%lld", trx_xd.m_id)
            , wxString::Format("%lld", trx_xd.m_id), true);
        hb.addColorMarker(getUDColour(trx_xd.m_color.GetValue()).GetAsString(), true);
        hb.addTableCellDate(trx_xd.TRANSDATE);
        hb.addTableCell(trx_xd.m_number);
        hb.addTableCellLink(wxString::Format("trxid:%lld", trx_xd.m_id)
            , trx_xd.ACCOUNTNAME);
        hb.addTableCell(trx_xd.PAYEENAME);
        hb.addTableCell(trx_xd.m_status.key(), false, true);
        hb.addTableCell(trx_xd.CATEGNAME);
        if (TrxModel::is_foreignAsTransfer(trx_xd))
            hb.addTableCell("< " + wxGetTranslation(trx_xd.m_type.name()));
        else
            hb.addTableCell(wxGetTranslation(trx_xd.m_type.name()));

        const AccountData* acc = AccountModel::instance().get_id_data_n(trx_xd.m_account_id);
        if (acc) {
            const CurrencyData* curr = AccountModel::instance().get_data_currency_p(*acc);
            double flow = TrxModel::account_flow(trx_xd, acc->m_id);
            hb.addCurrencyCell(flow, curr);
        }
        else {
            wxFAIL_MSG("account for transaction not found");
            hb.addEmptyTableCell();
        }

        // Attachments
        wxString AttachmentsLink = "";
        if (AttachmentModel::instance().find_ref_c(TrxModel::s_ref_type, trx_xd.m_id)) {
            AttachmentsLink = wxString::Format(R"(<a href = "attachment:%s|%lld" target="_blank">%s</a>)",
                TrxModel::s_ref_type.name_n(), trx_xd.m_id,
                mmAttachmentManage::GetAttachmentNoteSign());
        }

        //Notes
        hb.addTableCell(AttachmentsLink + trx_xd.m_notes);
        hb.endTableRow();
    }
    hb.endTbody();
    hb.endTable();
    hb.endDiv();

    hb.end();
    return hb.getHTMLText();
}
