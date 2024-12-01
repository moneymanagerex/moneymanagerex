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
    _dateFilter = false;
    _accountFilter = false;
    _payeeFilter = false;
    _categoryFilter = false;
}

void mmFilterTransactions::setDateRange(wxDateTime startDate, wxDateTime endDate)
{
    _dateFilter = true;
    if (startDate.FormatISOTime() == "00:00:00")
        _startDate = startDate.FormatISODate();
    else
        _startDate = startDate.FormatISOCombined();

    if (!Option::instance().UseTransDateTime())
        endDate = mmDateRange::getDayEnd(endDate);

    _endDate = endDate.FormatISOCombined();
}

void mmFilterTransactions::setAccountList(wxSharedPtr<wxArrayString> accountList)
{
    if (accountList)
    {
        _accountList.Clear();
        for (const auto &entry : *accountList)
        {
            const auto account = Model_Account::instance().get(entry);
            if (account) _accountList.Add(account->ACCOUNTID);
        }
        _accountFilter = true;
    }
}

void mmFilterTransactions::setPayeeList(const wxArrayInt payeeList)
{
    _payeeFilter = true;
    _payeeList = payeeList;
}

void mmFilterTransactions::setCategoryList(const std::vector<int> &categoryList)
{
    _categoryFilter = true;
    _categoryList = categoryList;
}

template<class MODEL, class DATA>
bool mmFilterTransactions::checkCategory(const DATA& tran, const std::map<int, typename MODEL::Split_Data_Set> & splits)
{
    const auto it = splits.find(tran.id());
    if (it == splits.end())
    {
        for (int it2 : _categoryList)
        {
            if (it2 == tran.CATEGID)
                return true;
        }
    }
    else
    {
        for (const auto& split : it->second)
        {
            for (int it2 : _categoryList)
            {
                if (it2 == split.CATEGID)
                    return true;
            }
        }

    }
    return false;
}

bool mmFilterTransactions::mmIsRecordMatches(const Model_Checking::Data &tran
    , const std::map<int, Model_Splittransaction::Data_Set>& split)
{
    bool ok = true;
    wxString strDate = Model_Checking::TRANSDATE(tran).FormatISOCombined();
    if (_accountFilter
        && (_accountList.Index(tran.ACCOUNTID) == wxNOT_FOUND)
        && (_accountList.Index(tran.TOACCOUNTID) == wxNOT_FOUND))
        ok = false;
    else if (_dateFilter && ((strDate < _startDate) || (strDate > _endDate)))
        ok = false;
    else if (_payeeFilter && (_payeeList.Index(tran.PAYEEID) == wxNOT_FOUND))
        ok = false;
    else if (_categoryFilter && !checkCategory<Model_Checking>(tran, split))
        ok = false;
    return ok;
}

wxString mmFilterTransactions::getHTML()
{
    mmHTMLBuilder hb;
    _trans.clear();
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
                if (_categoryFilter)
                {
                    found = false;

                    for (const auto& it : _categoryList)
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
                    _trans.push_back(full_tran);
                }
            }
        } else
            _trans.push_back(full_tran);
    }

    std::stable_sort(_trans.begin(), _trans.end(), SorterByTRANSDATE());

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
    for (auto& transaction : _trans)
    {
        hb.startTableRow();
        hb.addTableCellLink(wxString::Format("trx:%d", transaction.TRANSID)
            , wxString::Format("%i", transaction.TRANSID), true);
        hb.addColorMarker(getUDColour(transaction.COLOR).GetAsString(), true);
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
            double amount = Model_Checking::balance(transaction, acc->ACCOUNTID);
            hb.addCurrencyCell(amount, curr);
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