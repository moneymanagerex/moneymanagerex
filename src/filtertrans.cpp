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

#include "attachmentdialog.h"
#include "../reports/htmlbuilder.h"

mmFilterTransactions::mmFilterTransactions()
{
    this->clear();
}

void mmFilterTransactions::clear()
{
    m_dateFilter = false;
    m_accountFilter = false;
    m_payeeFilter = false;
    m_categoryFilter = false;
}

void mmFilterTransactions::setDateRange(wxDateTime startDate, wxDateTime endDate)
{
    m_dateFilter = true;
    if (startDate.FormatISOTime() == "00:00:00")
        m_startDate = startDate.FormatISODate();
    else
        m_startDate = startDate.FormatISOCombined();

    if (!Option::instance().UseTransDateTime())
        endDate = mmDateRange::getDayEnd(endDate);

    m_endDate = endDate.FormatISOCombined();
}

void mmFilterTransactions::setAccountList(wxSharedPtr<wxArrayString> accountList)
{
    if (accountList)
    {
        m_accountList.clear();
        for (const auto &entry : *accountList)
        {
            const auto account = Model_Account::instance().get(entry);
            if (account) m_accountList.push_back(account->ACCOUNTID);
        }
        m_accountFilter = true;
    }
}

void mmFilterTransactions::setPayeeList(const wxArrayInt64& payeeList)
{
    m_payeeFilter = true;
    m_payeeList = payeeList;
}

void mmFilterTransactions::setCategoryList(const wxArrayInt64 &categoryList)
{
    m_categoryFilter = true;
    m_categoryList = categoryList;
}

template<class MODEL, class DATA>
bool mmFilterTransactions::checkCategory(const DATA& tran, const std::map<int64, typename MODEL::Split_Data_Set> & splits)
{
    const auto it = splits.find(tran.id());
    if (it == splits.end())
    {
        for (auto it2 : m_categoryList)
        {
            if (it2 == tran.CATEGID)
                return true;
        }
    }
    else
    {
        for (const auto& split : it->second)
        {
            for (auto it2 : m_categoryList)
            {
                if (it2 == split.CATEGID)
                    return true;
            }
        }

    }
    return false;
}

bool mmFilterTransactions::mmIsRecordMatches(const Model_Checking::Data &tran
    , const std::map<int64, Model_Splittransaction::Data_Set>& split)
{
    bool ok = true;
    wxString strDate = Model_Checking::TRANSDATE(tran).FormatISOCombined();
    if (m_accountFilter
        && (std::find(m_accountList.begin(), m_accountList.end(), tran.ACCOUNTID) == m_accountList.end())
        && (std::find(m_accountList.begin(), m_accountList.end(), tran.TOACCOUNTID) == m_accountList.end()))
        ok = false;
    else if (m_dateFilter && ((strDate < m_startDate) || (strDate > m_endDate)))
        ok = false;
    else if (m_payeeFilter && (std::find(m_payeeList.begin(), m_payeeList.end(), tran.PAYEEID) == m_payeeList.end()))
        ok = false;
    else if (m_categoryFilter && !checkCategory<Model_Checking>(tran, split))
        ok = false;
    return ok;
}

wxString mmFilterTransactions::getHTML()
{
    mmHTMLBuilder hb;
    m_trans.clear();
    const auto splits = Model_Splittransaction::instance().get_all();
    const auto tags = Model_Taglink::instance().get_all(Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION));
    for (const auto& tran : Model_Checking::instance().all()) //TODO: find should be faster
    {
        if (!mmIsRecordMatches(tran, splits)) continue;
        Model_Checking::Full_Data full_tran(tran, splits, tags);

        full_tran.PAYEENAME = full_tran.real_payee_name(full_tran.ACCOUNTID);
        if (full_tran.has_split())
        {
            bool found = true;
            for (const auto& split : full_tran.m_splits)
            {
                if (m_categoryFilter)
                {
                    found = false;

                    for (const auto& it : m_categoryList)
                    {
                        if (it == split.CATEGID) {
                            found = true;
                            break;
                        }
                    }
                }

                if (found)
                {
                    full_tran.CATEGNAME = Model_Category::full_name(split.CATEGID);
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
    hb.addReportHeader(_("Transaction Details"), 1, false);

    const wxString& AttRefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
    hb.addDivContainer();
    hb.addTableCellLink("back:",wxString::Format("<< %s", _("Back")));
    hb.endDiv();
    hb.addDivContainer("shadow");
    hb.startSortTable();
    hb.startThead();
    hb.startTableRow();
    hb.addTableHeaderCell(_("ID"), "ID text-right");
    hb.addTableHeaderCell(_("Color"), "Color text-center");
    hb.addTableHeaderCell(_("Date"), "Date");
    hb.addTableHeaderCell(_("Number"), "Number");
    hb.addTableHeaderCell(_("Account"), "Account");
    hb.addTableHeaderCell(_("Payee"), "Payee");
    hb.addTableHeaderCell(_("Status"), "Status text-center");
    hb.addTableHeaderCell(_("Category"), "Category");
    hb.addTableHeaderCell(_("Type"), "Type");
    hb.addTableHeaderCell(_("Amount"), "Amount text-right");
    hb.addTableHeaderCell(_("Notes"), "Notes");
    hb.endTableRow();
    hb.endThead();
    hb.startTbody();
    // Display the data for each row
    for (auto& transaction : m_trans)
    {
        hb.startTableRow();
        hb.addTableCellLink(wxString::Format("trx:%d", transaction.TRANSID)
            , wxString::Format("%i", transaction.TRANSID), true);
        hb.addColorMarker(getUDColour(transaction.COLOR.GetValue()).GetAsString(), true);
        hb.addTableCellDate(transaction.TRANSDATE);
        hb.addTableCell(transaction.TRANSACTIONNUMBER);
        hb.addTableCellLink(wxString::Format("trxid:%d", transaction.TRANSID)
            , transaction.ACCOUNTNAME);
        hb.addTableCell(transaction.PAYEENAME);
        hb.addTableCell(transaction.STATUS, false, true);
        hb.addTableCell(transaction.CATEGNAME);
        if (Model_Checking::foreignTransactionAsTransfer(transaction))
            hb.addTableCell("< " + wxGetTranslation(transaction.TRANSCODE));
        else
            hb.addTableCell(wxGetTranslation(transaction.TRANSCODE));

        Model_Account::Data* acc;
        const Model_Currency::Data* curr;
        acc = Model_Account::instance().get(transaction.ACCOUNTID);
        curr = Model_Account::currency(acc);
        if (acc)
        {
            double flow = Model_Checking::account_flow(transaction, acc->ACCOUNTID);
            hb.addCurrencyCell(flow, curr);
        }
        else
        {
            wxFAIL_MSG("account for transaction not found");
            hb.addEmptyTableCell();
        }

        // Attachments
        wxString AttachmentsLink = "";
        if (Model_Attachment::instance().NrAttachments(AttRefType, transaction.TRANSID))
        {
            AttachmentsLink = wxString::Format(R"(<a href = "attachment:%s|%d" target="_blank">%s</a>)",
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
