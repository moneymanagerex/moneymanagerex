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

#include "filtertrans.h"

#include "dialog/AttachmentDialog.h"
#include "reports/htmlbuilder.h"

mmFilterTransactions::mmFilterTransactions()
{
    this->clear();
}

void mmFilterTransactions::clear()
{
    m_filter_date = false;
    m_filter_account = false;
    m_filter_payee = false;
    m_filter_category = false;
}

void mmFilterTransactions::setDateRange(const DateRange2& date_range)
{
    m_filter_date = true;
    m_start_date = date_range.rangeStartIsoStartN();
    m_end_date = date_range.rangeEndIsoEndN();
}
void mmFilterTransactions::setDateRange(wxDateTime startDate, wxDateTime endDate)
{
    m_filter_date = true;
    if (startDate.FormatISOTime() == "00:00:00")
        m_start_date = startDate.FormatISODate();
    else
        m_start_date = startDate.FormatISOCombined();

    if (!PreferencesModel::instance().UseTransDateTime())
        endDate = mmDateRange::getDayEnd(endDate);

    m_end_date = endDate.FormatISOCombined();
}

void mmFilterTransactions::setAccountList(wxSharedPtr<wxArrayString> accountList)
{
    if (accountList)
    {
        m_account_a.clear();
        for (const auto &entry : *accountList)
        {
            const auto account = AccountModel::instance().get(entry);
            if (account) m_account_a.push_back(account->ACCOUNTID);
        }
        m_filter_account = true;
    }
}

void mmFilterTransactions::setPayeeList(const wxArrayInt64& payeeList)
{
    m_filter_payee = true;
    m_payee_a = payeeList;
}

void mmFilterTransactions::setCategoryList(const wxArrayInt64 &categoryList)
{
    m_filter_category = true;
    m_category_a = categoryList;
}

template<class MODEL, class DATA>
bool mmFilterTransactions::checkCategory(
    const DATA& tran,
    const std::map<int64, typename MODEL::Split_Data_Set> & splits
) {
    const auto it = splits.find(tran.id());
    if (it == splits.end()) {
        for (auto it2 : m_category_a) {
            if (it2 == tran.CATEGID)
                return true;
        }
    }
    else {
        for (const auto& split : it->second) {
            for (auto it2 : m_category_a) {
                if (it2 == split.CATEGID)
                    return true;
            }
        }

    }
    return false;
}

bool mmFilterTransactions::mmIsRecordMatches(
    const TransactionModel::Data &tran,
    const std::map<int64, TransactionSplitModel::Data_Set>& split
) {
    bool ok = true;
    wxString strDate = TransactionModel::getTransDateTime(tran).FormatISOCombined();
    if (m_filter_account
        && (std::find(m_account_a.begin(), m_account_a.end(), tran.ACCOUNTID) == m_account_a.end())
        && (std::find(m_account_a.begin(), m_account_a.end(), tran.TOACCOUNTID) == m_account_a.end()))
        ok = false;
    else if (m_filter_date && ((strDate < m_start_date) || (strDate > m_end_date)))
        ok = false;
    else if (m_filter_payee && (std::find(m_payee_a.begin(), m_payee_a.end(), tran.PAYEEID) == m_payee_a.end()))
        ok = false;
    else if (m_filter_category && !checkCategory<TransactionModel>(tran, split))
        ok = false;
    return ok;
}

wxString mmFilterTransactions::getHTML()
{
    mmHTMLBuilder hb;
    m_trans.clear();
    const auto splits = TransactionSplitModel::instance().get_all();
    const auto tags = TagLinkModel::instance().get_all(TransactionModel::refTypeName);
    for (const auto& tran : TransactionModel::instance().all()) //TODO: find should be faster
    {
        if (!mmIsRecordMatches(tran, splits)) continue;
        TransactionModel::Full_Data full_tran(tran, splits, tags);

        full_tran.PAYEENAME = full_tran.real_payee_name(full_tran.ACCOUNTID);
        if (full_tran.has_split())
        {
            bool found = true;
            for (const auto& split : full_tran.m_splits)
            {
                if (m_filter_category)
                {
                    found = false;

                    for (const auto& it : m_category_a)
                    {
                        if (it == split.CATEGID) {
                            found = true;
                            break;
                        }
                    }
                }

                if (found)
                {
                    full_tran.CATEGNAME = CategoryModel::full_name(split.CATEGID);
                    full_tran.TRANSAMOUNT = split.SPLITTRANSAMOUNT;
                    full_tran.NOTES.Append((tran.NOTES.IsEmpty() ? "" : " ") + split.NOTES);
                    m_trans.push_back(full_tran);
                }
            }
        } else
            m_trans.push_back(full_tran);
    }

    std::stable_sort(m_trans.begin(), m_trans.end(), SorterByTRANSDATE());

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

    const wxString& AttRefType = TransactionModel::refTypeName;
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
    for (auto& transaction : m_trans)
    {
        hb.startTableRow();
        hb.addTableCellLink(wxString::Format("trx:%lld", transaction.TRANSID)
            , wxString::Format("%lld", transaction.TRANSID), true);
        hb.addColorMarker(getUDColour(transaction.COLOR.GetValue()).GetAsString(), true);
        hb.addTableCellDate(transaction.TRANSDATE);
        hb.addTableCell(transaction.TRANSACTIONNUMBER);
        hb.addTableCellLink(wxString::Format("trxid:%lld", transaction.TRANSID)
            , transaction.ACCOUNTNAME);
        hb.addTableCell(transaction.PAYEENAME);
        hb.addTableCell(transaction.STATUS, false, true);
        hb.addTableCell(transaction.CATEGNAME);
        if (TransactionModel::foreignTransactionAsTransfer(transaction))
            hb.addTableCell("< " + wxGetTranslation(transaction.TRANSCODE));
        else
            hb.addTableCell(wxGetTranslation(transaction.TRANSCODE));

        AccountModel::Data* acc;
        const CurrencyModel::Data* curr;
        acc = AccountModel::instance().get(transaction.ACCOUNTID);
        curr = AccountModel::currency(acc);
        if (acc)
        {
            double flow = TransactionModel::account_flow(transaction, acc->ACCOUNTID);
            hb.addCurrencyCell(flow, curr);
        }
        else
        {
            wxFAIL_MSG("account for transaction not found");
            hb.addEmptyTableCell();
        }

        // Attachments
        wxString AttachmentsLink = "";
        if (AttachmentModel::instance().NrAttachments(AttRefType, transaction.TRANSID))
        {
            AttachmentsLink = wxString::Format(R"(<a href = "attachment:%s|%lld" target="_blank">%s</a>)",
                AttRefType, transaction.TRANSID, mmAttachmentManage::GetAttachmentNoteSign());
        }

        //Notes
        hb.addTableCell(AttachmentsLink + transaction.NOTES);
        hb.endTableRow();
    }
    hb.endTbody();
    hb.endTable();
    hb.endDiv();

    hb.end();
    return hb.getHTMLText();
}
