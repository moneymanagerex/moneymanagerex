/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2014 - 2021 Nikolay Akimov
 Copyright (C) 2021-2022 Mark Whalley (mark@ipx.co.uk)

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

#include "filtertransdialog.h"
#include "mmcheckingpanel.h"
#include "mmchecking_list.h"
#include "fusedtransaction.h"
#include "paths.h"
#include "constants.h"
#include "images_list.h"
#include "util.h"
#include "mmex.h"
#include "mmframe.h"
#include "mmTips.h"
#include "mmSimpleDialogs.h"
#include "splittransactionsdialog.h"
#include "transdialog.h"
#include "validators.h"
#include "transactionsupdatedialog.h"
#include "attachmentdialog.h"
#include "model/allmodel.h"
#include "sharetransactiondialog.h"
#include "assetdialog.h"
#include "billsdepositsdialog.h"
#include <wx/clipbrd.h>
#include <float.h>

//----------------------------------------------------------------------------

#include <wx/srchctrl.h>
#include <algorithm>
#include <wx/sound.h>
//----------------------------------------------------------------------------

const std::vector<std::pair<mmCheckingPanel::FILTER_ID, wxString> > mmCheckingPanel::FILTER_CHOICES =
{
    { mmCheckingPanel::FILTER_ID_ALL,           wxString(wxTRANSLATE("View All Transactions")) },
    { mmCheckingPanel::FILTER_ID_TODAY,          wxString(wxTRANSLATE("View Today")) },
    { mmCheckingPanel::FILTER_ID_CURRENTMONTH,   wxString(wxTRANSLATE("View Current Month")) },
    { mmCheckingPanel::FILTER_ID_LAST30,         wxString(wxTRANSLATE("View Last 30 days")) },
    { mmCheckingPanel::FILTER_ID_LAST90,         wxString(wxTRANSLATE("View Last 90 days")) },
    { mmCheckingPanel::FILTER_ID_LASTMONTH,      wxString(wxTRANSLATE("View Last Month")) },
    { mmCheckingPanel::FILTER_ID_LAST3MONTHS,    wxString(wxTRANSLATE("View Last 3 Months")) },
    { mmCheckingPanel::FILTER_ID_LAST12MONTHS,   wxString(wxTRANSLATE("View Last 12 Months")) },
    { mmCheckingPanel::FILTER_ID_CURRENTYEAR,    wxString(wxTRANSLATE("View Current Year")) },
    { mmCheckingPanel::FILTER_ID_CURRENTFINYEAR, wxString(wxTRANSLATE("View Current Financial Year")) },
    { mmCheckingPanel::FILTER_ID_LASTYEAR,       wxString(wxTRANSLATE("View Last Year")) },
    { mmCheckingPanel::FILTER_ID_LASTFINYEAR,    wxString(wxTRANSLATE("View Last Financial Year")) },
    { mmCheckingPanel::FILTER_ID_STATEMENTDATE,  wxString(wxTRANSLATE("View Since Statement Date")) },
    { mmCheckingPanel::FILTER_ID_DIALOG,         wxString::FromUTF8(wxTRANSLATE("View Transaction Report…")) }
};

wxArrayString mmCheckingPanel::FILTER_STR = filter_str_all();
const wxString mmCheckingPanel::FILTER_STR_ALL    = FILTER_STR[FILTER_ID_ALL];
const wxString mmCheckingPanel::FILTER_STR_DIALOG = FILTER_STR[FILTER_ID_DIALOG];

wxArrayString mmCheckingPanel::filter_str_all()
{
    wxArrayString items;
    int i = 0;
    for (const auto& item: FILTER_CHOICES)
    {
        wxASSERT_MSG(item.first == i++, "Wrong order in mmCheckingPanel::FILTER_CHOICES");
        items.Add(item.second);
    }
    return items;
}
//----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(mmCheckingPanel, wxPanel)
    EVT_BUTTON(wxID_NEW,           mmCheckingPanel::OnNewTransaction)
    EVT_BUTTON(wxID_EDIT,          mmCheckingPanel::OnEditTransaction)
    EVT_BUTTON(wxID_REMOVE,        mmCheckingPanel::OnDeleteTransaction)
    EVT_BUTTON(wxID_DUPLICATE,     mmCheckingPanel::OnDuplicateTransaction)
    EVT_BUTTON(wxID_PASTE,         mmCheckingPanel::OnEnterScheduled)
    EVT_BUTTON(wxID_IGNORE,        mmCheckingPanel::OnSkipScheduled)
    EVT_BUTTON(wxID_UNDELETE,      mmCheckingPanel::OnRestoreTransaction)
    EVT_BUTTON(wxID_FILE,          mmCheckingPanel::OnOpenAttachment)
    EVT_BUTTON(ID_TRX_FILTER,      mmCheckingPanel::OnMouseLeftDown)
    //EVT_CHECKBOX(ID_TRX_SCHEDULED, mmCheckingPanel::OnScheduled)
    EVT_TOGGLEBUTTON(ID_TRX_SCHEDULED, mmCheckingPanel::OnScheduled)
    EVT_SEARCHCTRL_SEARCH_BTN(wxID_FIND, mmCheckingPanel::OnSearchTxtEntered)
    EVT_MENU_RANGE(wxID_HIGHEST + FILTER_ID_ALL, wxID_HIGHEST + FILTER_ID_MAX
        , mmCheckingPanel::OnViewPopupSelected)
    EVT_MENU_RANGE(Model_Checking::TYPE_ID_WITHDRAWAL, Model_Checking::TYPE_ID_TRANSFER
        , mmCheckingPanel::OnNewTransaction)
wxEND_EVENT_TABLE()
//----------------------------------------------------------------------------

mmCheckingPanel::mmCheckingPanel(wxWindow *parent, mmGUIFrame *frame, int64 accountID, int id)
    : m_AccountID(accountID)
    , isAllAccounts_((-1 == accountID) ? true : false)
    , isTrash_((-2 == accountID) ? true : false)
    , m_frame(frame)
{
    Create(parent, id);
    Fit();
}
//----------------------------------------------------------------------------

/*
    We cannon use OnClose() event because this class deletes
    via DestroyChildren() of its parent.
*/
mmCheckingPanel::~mmCheckingPanel()
{
}

bool mmCheckingPanel::Create(
    wxWindow* parent,
    wxWindowID winid, const wxPoint& pos,
    const wxSize& size, long style, const wxString& name
)
{
    if (isAllAccounts_ || isTrash_) {
        m_currency = Model_Currency::GetBaseCurrency();
    }
    else {
        m_account = Model_Account::instance().get(m_AccountID);
        m_currency = Model_Account::currency(m_account);
    }

    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    if (!wxPanel::Create(parent, winid, pos, size, style, name)) return false;

    this->windowsFreezeThaw();
    CreateControls();

    initFilterChoices();
    updateFilterState();

    if (m_transFilterActive) {
        const wxString& def_view = wxString::Format(R"({ "FILTER": "%s" })", Model_Setting::instance().ViewTransactions());
        wxString json = Model_Infotable::instance().GetStringInfo(wxString::Format("CHECK_FILTER_ID_ADV_%lld", m_AccountID), def_view);
        m_trans_filter_dlg = new mmFilterTransactionsDialog(parent, m_AccountID, false, json);
        m_bitmapTransFilter->SetToolTip(m_trans_filter_dlg->mmGetDescriptionToolTip());
    }

    RefreshList();
    this->windowsFreezeThaw();

    Model_Usage::instance().pageview(this);
    return true;
}

void mmCheckingPanel::sortTable()
{
    m_listCtrlAccount->sortTable();
}

void mmCheckingPanel::filterTable()
{
    m_listCtrlAccount->m_trans.clear();

    m_account_balance = !isAllAccounts_ && !isTrash_ && m_account ? m_account->INITIALBAL : 0.0;
    m_account_recbalance = m_account_balance;
    m_show_reconciled = false;
    m_account_flow = 0.0;

    const wxString transRefType = Model_Attachment::REFTYPE_STR_TRANSACTION;
    const wxString splitRefType = Model_Attachment::REFTYPE_STR_TRANSACTIONSPLIT;
    const wxString billsRefType = Model_Attachment::REFTYPE_STR_BILLSDEPOSIT;
    const wxString billsplitRefType = Model_Attachment::REFTYPE_STR_BILLSDEPOSITSPLIT;

    Model_CustomField::TYPE_ID UDFC01_Type = Model_CustomField::getUDFCType(transRefType, "UDFC01");
    Model_CustomField::TYPE_ID UDFC02_Type = Model_CustomField::getUDFCType(transRefType, "UDFC02");
    Model_CustomField::TYPE_ID UDFC03_Type = Model_CustomField::getUDFCType(transRefType, "UDFC03");
    Model_CustomField::TYPE_ID UDFC04_Type = Model_CustomField::getUDFCType(transRefType, "UDFC04");
    Model_CustomField::TYPE_ID UDFC05_Type = Model_CustomField::getUDFCType(transRefType, "UDFC05");
    int UDFC01_Scale = Model_CustomField::getDigitScale(Model_CustomField::getUDFCProperties(transRefType, "UDFC01"));
    int UDFC02_Scale = Model_CustomField::getDigitScale(Model_CustomField::getUDFCProperties(transRefType, "UDFC02"));
    int UDFC03_Scale = Model_CustomField::getDigitScale(Model_CustomField::getUDFCProperties(transRefType, "UDFC03"));
    int UDFC04_Scale = Model_CustomField::getDigitScale(Model_CustomField::getUDFCProperties(transRefType, "UDFC04"));
    int UDFC05_Scale = Model_CustomField::getDigitScale(Model_CustomField::getUDFCProperties(transRefType, "UDFC05"));

    auto trans_fields_data = Model_CustomFieldData::instance().get_all(Model_Attachment::REFTYPE_ID_TRANSACTION);
    const auto matrix = Model_CustomField::getMatrix(Model_Attachment::REFTYPE_ID_TRANSACTION);
    int64 udfc01_ref_id = matrix.at("UDFC01");
    int64 udfc02_ref_id = matrix.at("UDFC02");
    int64 udfc03_ref_id = matrix.at("UDFC03");
    int64 udfc04_ref_id = matrix.at("UDFC04");
    int64 udfc05_ref_id = matrix.at("UDFC05");

    bool ignore_future = Option::instance().getIgnoreFutureTransactions();
    const wxString today_date = Option::instance().UseTransDateTime() ?
        wxDateTime::Now().FormatISOCombined() :
        wxDateTime(23, 59, 59, 999).FormatISOCombined();

    const auto trans_splits = Model_Splittransaction::instance().get_all();
    const auto trans_tags = Model_Taglink::instance().get_all(transRefType);
    const auto trans_attachments = Model_Attachment::instance().get_all(
        Model_Attachment::REFTYPE_ID_TRANSACTION);
    const auto trans = (isAllAccounts_ || isTrash_) ?
        Model_Checking::instance().all() :
        Model_Account::transaction(this->m_account);

    std::map<int64, Model_Budgetsplittransaction::Data_Set> bills_splits;
    std::map<int64, Model_Taglink::Data_Set> bills_tags;
    std::map<int64, Model_Attachment::Data_Set> bills_attachments;
    Model_Billsdeposits::Data_Set bills;
    typedef std::tuple<int /* i */, wxString /* date */, int /* repeat_num */> bills_index_t;
    std::vector<bills_index_t> bills_index;
    if (m_scheduled_allowed && m_scheduled_selected)
    {
        bills_splits = Model_Budgetsplittransaction::instance().get_all();
        bills_tags = Model_Taglink::instance().get_all(billsRefType);
        bills_attachments = Model_Attachment::instance().get_all(
            Model_Attachment::REFTYPE_ID_BILLSDEPOSIT);
        bills = (isAllAccounts_ || isTrash_) ?
            Model_Billsdeposits::instance().all() :
            Model_Account::billsdeposits(this->m_account);
        for (unsigned int i = 0; i < bills.size(); ++i)
        {
            int limit = 1000;  // this is enough for daily repetitions for one year
            auto dates = Model_Billsdeposits::unroll(bills[i], m_end_date, limit);
            for (unsigned int repeat_num = 1; repeat_num <= dates.size(); ++repeat_num)
                bills_index.push_back({i, dates[repeat_num-1], repeat_num});
        }
        std::stable_sort(bills_index.begin(), bills_index.end(),
            [](const bills_index_t& a, const bills_index_t& b) -> bool {
                return std::get<1>(a) < std::get<1>(b);
            });
    }

    auto trans_it = trans.begin();
    auto bills_it = bills_index.begin();
    while (trans_it != trans.end() || bills_it != bills_index.end())
    {
        int bill_i;
        wxString tran_date;
        int repeat_num = 0;
        Model_Checking::Data bill_tran;
        const Model_Checking::Data* tran = nullptr;

        if (trans_it != trans.end())
            tran_date = Model_Checking::TRANSDATE(*trans_it).FormatISOCombined();
        if (trans_it != trans.end() && (bills_it == bills_index.end() || tran_date <= std::get<1>(*bills_it)))
        {
            tran = &(*trans_it);
            trans_it++;
        }
        else {
            bill_i = std::get<0>(*bills_it);
            tran_date = std::get<1>(*bills_it);
            repeat_num = std::get<2>(*bills_it);
            bill_tran = Fused_Transaction::execute_bill(bills[bill_i], tran_date);
            tran = &bill_tran;
            bills_it++;
        }

        if (isTrash_ != !tran->DELETEDTIME.IsEmpty())
            continue;
        if (ignore_future && tran_date > today_date)
            break;

        // update m_account_balance even if tran is filtered out
        double account_flow;
        if (!isAllAccounts_ && !isTrash_) {
            // note: !isTrash_ implies tran->DELETEDTIME.IsEmpty()
            account_flow = Model_Checking::account_flow(tran, m_AccountID);
            m_account_balance += account_flow;
            if (Model_Checking::status_id(tran->STATUS) == Model_Checking::STATUS_ID_RECONCILED)
                m_account_recbalance += account_flow;
            else
                m_show_reconciled = true;
        }

        if (!m_transFilterActive && m_filter_id != FILTER_ID_ALL &&
            (tran_date < m_begin_date || tran_date > m_end_date))
            continue;

        Fused_Transaction::Full_Data full_tran = (repeat_num == 0) ?
            Fused_Transaction::Full_Data(*tran, trans_splits, trans_tags) :
            Fused_Transaction::Full_Data(bills[bill_i], tran_date, repeat_num, bills_splits, bills_tags);

        bool expandSplits = false;
        if (m_transFilterActive)
        {
            int txnMatch = m_trans_filter_dlg->mmIsRecordMatches(*tran, full_tran.m_splits);
            if (!txnMatch)
                continue;
            if (txnMatch < static_cast<int>(full_tran.m_splits.size()) + 1)
                expandSplits = true;
        }

        full_tran.PAYEENAME = full_tran.real_payee_name(m_AccountID);
        if (!isAllAccounts_ && !isTrash_) {
            if (full_tran.ACCOUNTID_W != m_AccountID) {
                full_tran.ACCOUNTID_W = -1; full_tran.TRANSAMOUNT_W = 0.0;
            }
            if (full_tran.ACCOUNTID_D != m_AccountID) {
                full_tran.ACCOUNTID_D = -1; full_tran.TRANSAMOUNT_D = 0.0;
            }
            full_tran.ACCOUNT_FLOW = account_flow;
            full_tran.ACCOUNT_BALANCE = m_account_balance;
        }

        if (repeat_num == 0 && trans_attachments.find(tran->TRANSID) != trans_attachments.end())
        {
            for (const auto& entry : trans_attachments.at(tran->TRANSID))
                full_tran.ATTACHMENT_DESCRIPTION.Add(entry.DESCRIPTION);
        }
        else if (repeat_num > 0 && bills_attachments.find(full_tran.m_bdid) != bills_attachments.end())
        {
            for (const auto& entry : bills_attachments.at(full_tran.m_bdid))
                full_tran.ATTACHMENT_DESCRIPTION.Add(entry.DESCRIPTION);
        }

        full_tran.UDFC01_Type = Model_CustomField::TYPE_ID_UNKNOWN;
        full_tran.UDFC02_Type = Model_CustomField::TYPE_ID_UNKNOWN;
        full_tran.UDFC03_Type = Model_CustomField::TYPE_ID_UNKNOWN;
        full_tran.UDFC04_Type = Model_CustomField::TYPE_ID_UNKNOWN;
        full_tran.UDFC05_Type = Model_CustomField::TYPE_ID_UNKNOWN;
        full_tran.UDFC01_val = -DBL_MAX;
        full_tran.UDFC02_val = -DBL_MAX;
        full_tran.UDFC03_val = -DBL_MAX;
        full_tran.UDFC04_val = -DBL_MAX;
        full_tran.UDFC05_val = -DBL_MAX;

        if (repeat_num == 0 && trans_fields_data.find(tran->TRANSID) != trans_fields_data.end())
        {
            for (const auto& udfc : trans_fields_data.at(tran->TRANSID))
            {
                if (udfc.FIELDID == udfc01_ref_id) {
                    full_tran.UDFC01 = udfc.CONTENT;
                    full_tran.UDFC01_val = cleanseNumberStringToDouble(udfc.CONTENT, UDFC01_Scale);
                    full_tran.UDFC01_Type = UDFC01_Type;
                }
                else if (udfc.FIELDID == udfc02_ref_id) {
                    full_tran.UDFC02 = udfc.CONTENT;
                    full_tran.UDFC02_val = cleanseNumberStringToDouble(udfc.CONTENT, UDFC02_Scale);
                    full_tran.UDFC02_Type = UDFC02_Type;
                }
                else if (udfc.FIELDID == udfc03_ref_id) {
                    full_tran.UDFC03 = udfc.CONTENT;
                    full_tran.UDFC03_val = cleanseNumberStringToDouble(udfc.CONTENT, UDFC03_Scale);
                    full_tran.UDFC03_Type = UDFC03_Type;
                }
                else if (udfc.FIELDID == udfc04_ref_id) {
                    full_tran.UDFC04 = udfc.CONTENT;
                    full_tran.UDFC04_val = cleanseNumberStringToDouble(udfc.CONTENT, UDFC04_Scale);
                    full_tran.UDFC04_Type = UDFC04_Type;
                }
                else if (udfc.FIELDID == udfc05_ref_id) {
                    full_tran.UDFC05 = udfc.CONTENT;
                    full_tran.UDFC05_val = cleanseNumberStringToDouble(udfc.CONTENT, UDFC05_Scale);
                    full_tran.UDFC05_Type = UDFC05_Type;
                }
            }
        }
        else if (repeat_num > 0)
        {
            // not yet implemented: custom fields for scheduled transaction
        }

        if (!expandSplits) {
            m_listCtrlAccount->m_trans.push_back(full_tran);
            if (!isAllAccounts_ && !isTrash_)
                m_account_flow += account_flow;
        }
        else
        {
            int splitIndex = 1;
            wxString tranTagnames = full_tran.TAGNAMES;
            for (const auto& split : full_tran.m_splits)
            {
                if (repeat_num == 0)
                    full_tran.displayID = wxString::Format("%lld", tran->TRANSID) +
                        "." + wxString::Format("%i", splitIndex);
                else
                    full_tran.displayID = ".";
                splitIndex++;
                full_tran.CATEGID = split.CATEGID;
                full_tran.CATEGNAME = Model_Category::full_name(split.CATEGID);
                full_tran.TRANSAMOUNT = split.SPLITTRANSAMOUNT;
                full_tran.NOTES = tran->NOTES;
                full_tran.TAGNAMES = tranTagnames;
                Model_Checking::Data splitWithTxnNotes = full_tran;
                Model_Checking::Data splitWithSplitNotes = full_tran;
                splitWithSplitNotes.NOTES = split.NOTES;
                if (m_trans_filter_dlg->mmIsSplitRecordMatches<Model_Splittransaction>(split)
                    && (m_trans_filter_dlg->mmIsRecordMatches<Model_Checking>(splitWithSplitNotes, true)
                        || m_trans_filter_dlg->mmIsRecordMatches<Model_Checking>(splitWithTxnNotes, true)))
                {
                    full_tran.ACCOUNT_FLOW = Model_Checking::account_flow(splitWithTxnNotes, m_AccountID);
                    full_tran.NOTES.Append((tran->NOTES.IsEmpty() ? "" : " ") + split.NOTES);
                    wxString tagnames;
                    const wxString reftype = (repeat_num == 0) ? splitRefType : billsplitRefType;
                    for (const auto& tag : Model_Taglink::instance().get(reftype, split.SPLITTRANSID))
                        tagnames.Append(tag.first + " ");
                    if (!tagnames.IsEmpty())
                        full_tran.TAGNAMES.Append((full_tran.TAGNAMES.IsEmpty() ? "" : ", ") + tagnames.Trim());
                    m_listCtrlAccount->m_trans.push_back(full_tran);
                    m_account_flow += full_tran.ACCOUNT_FLOW;
                }
            }
        }
    }
}

void mmCheckingPanel::OnButtonRightDown(wxMouseEvent& event)
{
    int id = event.GetId();
    switch (id)
    {
    case ID_TRX_FILTER:
    {
        wxCommandEvent evt(wxID_ANY, wxID_HIGHEST + FILTER_ID_DIALOG);
        OnViewPopupSelected(evt);
        break;
    }
    case wxID_FILE:
    {
        auto selected_id = m_listCtrlAccount->getSelectedId();
        if (selected_id.size() == 1) {
            const wxString refType = !selected_id[0].second ?
                Model_Attachment::REFTYPE_STR_TRANSACTION :
                Model_Attachment::REFTYPE_STR_BILLSDEPOSIT;
            mmAttachmentDialog dlg(this, refType, selected_id[0].first);
            dlg.ShowModal();
            RefreshList();
        }
        break;
    }
    case wxID_NEW:
    {
        wxMenu menu;
        menu.Append(Model_Checking::TYPE_ID_WITHDRAWAL, wxGetTranslation(wxString::FromUTF8(wxTRANSLATE("&New Withdrawal…"))));
        menu.Append(Model_Checking::TYPE_ID_DEPOSIT, wxGetTranslation(wxString::FromUTF8(wxTRANSLATE("&New Deposit…"))));
        menu.Append(Model_Checking::TYPE_ID_TRANSFER, wxGetTranslation(wxString::FromUTF8(wxTRANSLATE("&New Transfer…"))));
        PopupMenu(&menu);
    }
    default:
        break;
    }
}

void mmCheckingPanel::OnMouseLeftDown(wxCommandEvent& event)
{
    wxMenu menu;
    int id = 0;
    for (const auto& item : FILTER_STR)
    {
        if ((!isAllAccounts_ && !isTrash_) || (FILTER_ID_STATEMENTDATE != id))
            menu.Append(wxID_HIGHEST + id, wxGetTranslation(item));
        id++;
    }
    PopupMenu(&menu);
    m_bitmapTransFilter->Layout();
    event.Skip();
}

//----------------------------------------------------------------------------

void mmCheckingPanel::CreateControls()
{
    wxBoxSizer* sizerV = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(sizerV);

    /* ---------------------- */

    wxFlexGridSizer* sizerVHeader = new wxFlexGridSizer(0, 1, 0, 0);
    sizerVHeader->AddGrowableCol(0, 0);
    sizerV->Add(sizerVHeader, g_flagsBorder1V);

    m_header_text = new wxStaticText(this, wxID_STATIC, "");
    m_header_text->SetFont(this->GetFont().Larger().Bold());
    sizerVHeader->Add(m_header_text, g_flagsExpandBorder1);

    wxBoxSizer* sizerHInfo = new wxBoxSizer(wxHORIZONTAL);
    m_header_balance = new wxStaticText(this, wxID_STATIC, "");
    sizerHInfo->Add(m_header_balance, g_flagsH);
    m_header_credit = new wxGauge(this, wxID_ANY, 100, wxDefaultPosition, wxSize(100,-1));
    sizerHInfo->Add(m_header_credit, g_flagsH);
    sizerVHeader->Add(sizerHInfo, g_flagsBorder1V);

    wxBoxSizer* sizerHCtrl = new wxBoxSizer(wxHORIZONTAL);
    m_bitmapTransFilter = new wxButton(this, ID_TRX_FILTER);
    m_bitmapTransFilter->SetBitmap(mmBitmapBundle(png::TRANSFILTER, mmBitmapButtonSize));
    sizerHCtrl->Add(m_bitmapTransFilter, g_flagsH);
    if (!isTrash_)
    {
        sizerHCtrl->AddSpacer(15);
        const auto& size = m_bitmapTransFilter->GetSize().GetY();
        //m_header_scheduled = new wxCheckBox(this, ID_TRX_SCHEDULED, _("Scheduled Transactions"));
        m_header_scheduled = new wxBitmapToggleButton(this, ID_TRX_SCHEDULED, mmBitmapBundle(png::RECURRING), wxDefaultPosition, wxSize(size, size));
        sizerHCtrl->Add(m_header_scheduled, g_flagsH);
        sizerHCtrl->AddSpacer(10);
    }
    m_header_sortOrder = new wxStaticText(this, wxID_STATIC, "");
    sizerHCtrl->Add(m_header_sortOrder, g_flagsH);
    sizerVHeader->Add(sizerHCtrl, g_flagsBorder1H);

    m_bitmapTransFilter->Connect(wxEVT_RIGHT_DOWN,
        wxMouseEventHandler(mmCheckingPanel::OnButtonRightDown), nullptr, this);

    /* ---------------------- */

    wxSplitterWindow* splitterListFooter = new wxSplitterWindow(this
        , wxID_ANY, wxDefaultPosition, wxSize(200, 200)
        , wxSP_3DBORDER | wxSP_3DSASH | wxNO_BORDER);

    m_images.push_back(mmBitmapBundle(png::UNRECONCILED));
    m_images.push_back(mmBitmapBundle(png::RECONCILED));
    m_images.push_back(mmBitmapBundle(png::VOID_STAT));
    m_images.push_back(mmBitmapBundle(png::FOLLOW_UP));
    m_images.push_back(mmBitmapBundle(png::DUPLICATE_STAT));
    m_images.push_back(mmBitmapBundle(png::UPARROW));
    m_images.push_back(mmBitmapBundle(png::DOWNARROW));

    m_listCtrlAccount = new TransactionListCtrl(this, splitterListFooter);

    m_listCtrlAccount->SetSmallImages(m_images);
    m_listCtrlAccount->SetNormalImages(m_images);

    m_listCtrlAccount->setSortOrder(m_listCtrlAccount->g_asc);
    m_listCtrlAccount->setSortColumn(m_listCtrlAccount->g_sortcol);

    // load the global variables
    m_sortSaveTitle = isAllAccounts_ ? "ALLTRANS" : (isTrash_ ? "DELETED" : "CHECK");

    long val = m_listCtrlAccount->COL_DEF_SORT;
    wxString strVal = Model_Setting::instance().GetStringSetting(wxString::Format("%s_SORT_COL", m_sortSaveTitle), wxString() << val);
    if (strVal.ToLong(&val)) m_listCtrlAccount->g_sortcol = m_listCtrlAccount->toEColumn(val);
    val = m_listCtrlAccount->COL_DEF_SORT2;
    strVal = Model_Setting::instance().GetStringSetting(wxString::Format("%s_SORT_COL2", m_sortSaveTitle), wxString() << val);
    if (strVal.ToLong(&val)) m_listCtrlAccount->prev_g_sortcol = m_listCtrlAccount->toEColumn(val);

    val = 1; // asc sorting default
    strVal = Model_Setting::instance().GetStringSetting(wxString::Format("%s_ASC", m_sortSaveTitle), wxString() << val);
    if (strVal.ToLong(&val)) m_listCtrlAccount->g_asc = val != 0;
    val = 1;
    strVal = Model_Setting::instance().GetStringSetting(wxString::Format("%s_ASC2", m_sortSaveTitle), wxString() << val);
    if (strVal.ToLong(&val)) m_listCtrlAccount->prev_g_asc = val != 0;

    // --
    m_listCtrlAccount->setSortColumn(m_listCtrlAccount->g_sortcol);
    m_listCtrlAccount->setSortOrder(m_listCtrlAccount->g_asc);
    m_listCtrlAccount->setColumnImage(m_listCtrlAccount->getSortColumn()
        , m_listCtrlAccount->getSortOrder() ? ICON_ASC : ICON_DESC); // asc\desc sort mark (arrow)

    wxPanel* panelFooter = new wxPanel(splitterListFooter, wxID_ANY
        , wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);
    mmThemeMetaColour(panelFooter, meta::COLOR_LISTPANEL);

    splitterListFooter->SplitHorizontally(m_listCtrlAccount, panelFooter);
    splitterListFooter->SetMinimumPaneSize(100);
    splitterListFooter->SetSashGravity(1.0);

    sizerV->Add(splitterListFooter, g_flagsExpandBorder1);

    wxBoxSizer* sizerVFooter = new wxBoxSizer(wxVERTICAL);
    panelFooter->SetSizer(sizerVFooter);

    wxBoxSizer* sizerHButtons = new wxBoxSizer(wxHORIZONTAL);
    sizerVFooter->Add(sizerHButtons, wxSizerFlags(g_flagsExpandBorder1).Proportion(0));

    m_btnDelete = new wxButton(panelFooter, wxID_REMOVE, _("&Delete "));
    mmToolTip(m_btnDelete, _("Delete all selected transactions"));

    if (!isTrash_) {
        m_btnNew = new wxButton(panelFooter, wxID_NEW, _("&New "));
        mmToolTip(m_btnNew, _("New Transaction"));
        sizerHButtons->Add(m_btnNew, 0, wxRIGHT, 2);

        m_btnEdit = new wxButton(panelFooter, wxID_EDIT, _("&Edit "));
        mmToolTip(m_btnEdit, _("Edit all selected transactions"));
        sizerHButtons->Add(m_btnEdit, 0, wxRIGHT, 2);
        m_btnEdit->Enable(false);

        m_btnDuplicate = new wxButton(panelFooter, wxID_DUPLICATE, _("D&uplicate "));
        mmToolTip(m_btnDuplicate, _("Duplicate selected transaction"));
        sizerHButtons->Add(m_btnDuplicate, 0, wxRIGHT, 2);
        m_btnDuplicate->Enable(false);

        sizerHButtons->Add(m_btnDelete, 0, wxRIGHT, 2);
        m_btnDelete->Enable(false);

        m_btnEnter = new wxButton(panelFooter, wxID_PASTE, _("Ente&r"));
        mmToolTip(m_btnEnter, _("Enter Next Scheduled Transaction Occurrence"));
        sizerHButtons->Add(m_btnEnter, 0, wxRIGHT, 2);
        m_btnEnter->Enable(false);

        m_btnSkip = new wxButton(panelFooter, wxID_IGNORE, _("&Skip"));
        mmToolTip(m_btnSkip, _("Skip Next Scheduled Transaction Occurrence"));
        sizerHButtons->Add(m_btnSkip, 0, wxRIGHT, 2);
        m_btnSkip->Enable(false);

        const auto& btnDupSize = m_btnDuplicate->GetSize();
        m_btnAttachment = new wxBitmapButton(panelFooter, wxID_FILE
            , mmBitmapBundle(png::CLIP), wxDefaultPosition
            , wxSize(30, btnDupSize.GetY()));
        mmToolTip(m_btnAttachment, _("Open attachments"));
        sizerHButtons->Add(m_btnAttachment, 0, wxRIGHT, 5);
        m_btnAttachment->Enable(false);

        m_btnAttachment->Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(mmCheckingPanel::OnButtonRightDown), nullptr, this);
        m_btnNew->Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(mmCheckingPanel::OnButtonRightDown), nullptr, this);
    }
    else
    {
        m_btnRestore = new wxButton(panelFooter, wxID_UNDELETE, _("&Restore "));
        mmToolTip(m_btnRestore, _("Restore selected transaction"));
        sizerHButtons->Add(m_btnRestore, 0, wxRIGHT, 5);
        m_btnRestore->Enable(false);

        sizerHButtons->Add(m_btnDelete, 0, wxRIGHT, 5);
        m_btnDelete->Enable(false);
    }

    wxSearchCtrl* searchCtrl = new wxSearchCtrl(panelFooter
        , wxID_FIND, wxEmptyString, wxDefaultPosition
        , wxSize(100, m_btnDelete->GetSize().GetHeight())
        , wxTE_NOHIDESEL, wxDefaultValidator);
    searchCtrl->SetDescriptiveText(_("Search"));
    sizerHButtons->Add(searchCtrl, g_flagsExpandBorder1);
    mmToolTip(searchCtrl,
        _("Enter any string to find it in the nearest transaction data") + "\n\n" +
        _("Tips: You can use wildcard characters - question mark (?), asterisk (*) - in your search criteria.") + "\n" +
        _("Use the question mark (?) to find any single character - for example, s?t finds 'sat' and 'set'.") + "\n" +
        _("Use the asterisk (*) to find any number of characters - for example, s*d finds 'sad' and 'started'.") + "\n" +
        _("Use the asterisk (*) in the begin to find any string in the middle of the sentence.")
    );

    //Infobar-mini
    m_info_panel_mini = new wxStaticText(panelFooter, wxID_STATIC, "");
    sizerHButtons->Add(m_info_panel_mini, 1, wxGROW | wxTOP | wxLEFT, 5);

    //Infobar
    m_info_panel = new wxStaticText(panelFooter, wxID_STATIC, ""
        , wxDefaultPosition, wxSize(200, -1), wxTE_MULTILINE | wxTE_WORDWRAP);
    sizerVFooter->Add(m_info_panel, g_flagsExpandBorder1);
    //Show tips when no any transaction selected
    showTips();
}

wxString mmCheckingPanel::GetPanelTitle(const Model_Account::Data& account) const
{
    if (isAllAccounts_)
        return wxString::Format(_("All Transactions"));
    else if (isTrash_)
        return wxString::Format(_("Deleted Transactions"));
    else
        return wxString::Format(_("Account View: %s"), account.ACCOUNTNAME);
}

wxString mmCheckingPanel::BuildPage() const
{
    Model_Account::Data *account = Model_Account::instance().get(m_AccountID);
    return m_listCtrlAccount->BuildPage((account ? GetPanelTitle(*account) : ""));
}

void mmCheckingPanel::setAccountSummary()
{
    Model_Account::Data *account = Model_Account::instance().get(m_AccountID);
    m_header_text->SetLabelText(GetPanelTitle(*account));
    m_header_credit->Hide();

    if (!isAllAccounts_ && !isTrash_)
    {
        bool show_displayed_balance_ = (m_transFilterActive || m_filter_id != FILTER_ID_ALL);
        wxString summaryLine = wxString::Format("%s%s" "%s%s%s" "%s%s%s" "%s%s%s"
            , _("Account Bal: ")
            , Model_Account::toCurrency(m_account_balance, account)
            , m_show_reconciled ? "     " : ""
            , m_show_reconciled ? _("Reconciled Bal: ") : ""
            , m_show_reconciled ? Model_Account::toCurrency(m_account_recbalance, account) : ""
            , m_show_reconciled ? "     " : ""
            , m_show_reconciled ? _("Diff: ") : ""
            , m_show_reconciled ? Model_Account::toCurrency(m_account_balance - m_account_recbalance, account) : ""
            , show_displayed_balance_ ? "     " : ""
            , show_displayed_balance_ ? _("Filtered Flow: ") : ""
            , show_displayed_balance_ ? Model_Account::toCurrency(m_account_flow, account) : "");
        if (account->CREDITLIMIT != 0.0)
        {
            double limit = 100.0 * ((m_account_balance < 0.0) ? -m_account_balance / account->CREDITLIMIT : 0.0);
            summaryLine.Append(wxString::Format("   %s %.1f%%"
                                , _("Credit Limit:")
                                , limit));
           m_header_credit->SetValue(limit);
           m_header_credit->Show();
        }
        m_header_balance->SetLabelText(summaryLine);
    }
    this->Layout();
}
//----------------------------------------------------------------------------

void mmCheckingPanel::enableButtons(bool edit, bool dup, bool del, bool enter, bool skip, bool attach)
{
    if (!isTrash_) {
        m_btnEdit->Enable(edit);
        m_btnDuplicate->Enable(dup);
        m_btnDelete->Enable(del);
        m_btnEnter->Enable(enter);
        m_btnSkip->Enable(skip);
        m_btnAttachment->Enable(attach);
    }
    else {
        m_btnRestore->Enable(edit);
        m_btnDelete->Enable(del);
    }
}

//----------------------------------------------------------------------------

void mmCheckingPanel::updateExtraTransactionData(bool single, int repeat_num, bool foreign)
{
    if (single)
    {
        enableButtons(
            /* Edit      */ true,
            /* Duplicate */ !foreign,
            /* Delete    */ !repeat_num,
            /* Enter     */ repeat_num == 1,
            /* Skip      */ repeat_num == 1,
            /* attach    */ true);

        long x = -1;
        for (x = 0; x < m_listCtrlAccount->GetItemCount(); x++) {
            if (m_listCtrlAccount->GetItemState(x, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED)
                break;
        }

        Fused_Transaction::Full_Data full_tran(m_listCtrlAccount->m_trans[x]);
        wxString miniStr = full_tran.info();
        //Show only first line but full string set as tooltip
        if (miniStr.Find("\n") > 1 && !miniStr.IsEmpty())
        {
            m_info_panel_mini->SetLabelText(miniStr.substr(0, miniStr.Find("\n")) + wxString::FromUTF8Unchecked(" …"));
            mmToolTip(m_info_panel_mini, miniStr);
        }
        else
        {
            m_info_panel_mini->SetLabelText(miniStr);
            mmToolTip(m_info_panel_mini, miniStr);
        }

        wxString notesStr = full_tran.NOTES;
        if (!full_tran.m_repeat_num) {
            auto splits = Model_Splittransaction::instance().find(Model_Splittransaction::TRANSID(full_tran.TRANSID));
            for (const auto& split : splits)
                if (!split.NOTES.IsEmpty()) {
                    notesStr += notesStr.empty() ? "" : "\n";
                    notesStr += split.NOTES;
                }
            if (full_tran.has_attachment()) {
                const wxString& RefType = Model_Attachment::REFTYPE_STR_TRANSACTION;
                Model_Attachment::Data_Set attachments = Model_Attachment::instance().FilterAttachments(RefType, full_tran.id());
                for (const auto& i : attachments) {
                    notesStr += notesStr.empty() ? "" : "\n";
                    notesStr += _("Attachment") + " " + i.DESCRIPTION + " " + i.FILENAME;
                }
            }
        }
        else {
            // not yet implemented
        }
        m_info_panel->SetLabelText(notesStr);
    }
    else /* !single */ {
        m_info_panel_mini->SetLabelText("");
        const auto selected = m_listCtrlAccount->getSelectedId();
        if (selected.size() > 0) {
            bool selected_bill = false;
            for (const auto& id : selected)
                if (id.second) { selected_bill = true; break; }

            enableButtons(
                /* Edit      */ true,
                /* Duplicate */ false,
                /* Delete    */ !selected_bill,
                /* Enter     */ false,
                /* Skip      */ false,
                /* attach    */ false);

            Model_Account::Data *account = Model_Account::instance().get(m_AccountID);
            Model_Currency::Data* currency = nullptr;
            if (account) currency = Model_Currency::instance().get(account->CURRENCYID);

            double balance = 0;
            wxString maxDate;
            wxString minDate;
            long item = -1;
            while (true) {
                item = m_listCtrlAccount->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
                if (item == -1) break;
                if (currency)
                    balance += Model_Checking::account_flow(m_listCtrlAccount->m_trans[item], m_AccountID);
                wxString transdate = m_listCtrlAccount->m_trans[item].TRANSDATE;
                if (minDate > transdate || minDate.empty()) minDate = transdate;
                if (maxDate < transdate || maxDate.empty()) maxDate = transdate;
            }

            wxDateTime min_date, max_date;
            min_date.ParseISODate(minDate);
            max_date.ParseISODate(maxDate);
            int days = max_date.Subtract(min_date).GetDays();

            wxString msg;
            msg = wxString::Format(_("Transactions selected: %zu"), selected.size());
            msg += "\n";
            if (currency) {
                msg += wxString::Format(_("Selected transactions balance: %s"),
                    Model_Currency::toCurrency(balance, currency));
                msg += "\n";
            }
            msg += wxString::Format(_("Days between selected transactions: %d"), days);
#ifdef __WXMAC__    // See issue #2914
            msg = "";
#endif
            m_info_panel->SetLabelText(msg);
        }
        else /* selected.size() == 0 */ {
            enableButtons(false, false, false, false, false, false);
            showTips();
        }
    }
}
//----------------------------------------------------------------------------

void mmCheckingPanel::showTips()
{
    if (Option::instance().getShowMoneyTips())
        m_info_panel->SetLabelText(
            wxGetTranslation(wxString::FromUTF8(
                TIPS[rand() % (sizeof(TIPS) / sizeof(wxString))]
                .ToStdString())
            )
        );
    else
        m_info_panel->SetLabelText("");
}
//----------------------------------------------------------------------------

void mmCheckingPanel::OnDeleteTransaction(wxCommandEvent& event)
{
    m_listCtrlAccount->OnDeleteTransaction(event);
}
//----------------------------------------------------------------------------

void mmCheckingPanel::OnRestoreTransaction(wxCommandEvent& event)
{
    m_listCtrlAccount->OnRestoreTransaction(event);
}
//----------------------------------------------------------------------------

void mmCheckingPanel::OnNewTransaction(wxCommandEvent& event)
{
    m_listCtrlAccount->OnNewTransaction(event);
}
//----------------------------------------------------------------------------

void mmCheckingPanel::OnEditTransaction(wxCommandEvent& event)
{
    m_listCtrlAccount->OnEditTransaction(event);
    m_listCtrlAccount->SetFocus();
}
//----------------------------------------------------------------------------

void mmCheckingPanel::OnDuplicateTransaction(wxCommandEvent& event)
{
    m_listCtrlAccount->OnDuplicateTransaction(event);
}
//----------------------------------------------------------------------------

void mmCheckingPanel::OnEnterScheduled(wxCommandEvent& event)
{
    m_listCtrlAccount->OnEnterScheduled(event);
}
//----------------------------------------------------------------------------

void mmCheckingPanel::OnSkipScheduled(wxCommandEvent& event)
{
    m_listCtrlAccount->OnSkipScheduled(event);
}
//----------------------------------------------------------------------------

void mmCheckingPanel::OnMoveTransaction(wxCommandEvent& event)
{
    m_listCtrlAccount->OnMoveTransaction(event);
}
//----------------------------------------------------------------------------

void mmCheckingPanel::OnOpenAttachment(wxCommandEvent& event)
{
    m_listCtrlAccount->OnOpenAttachment(event);
    m_listCtrlAccount->SetFocus();
}

//----------------------------------------------------------------------------

void mmCheckingPanel::initFilterChoices()
{
    const wxString& def_view = wxString::Format("{ \"FILTER\": \"%s\" }",
        Model_Setting::instance().ViewTransactions());
    const auto& data = Model_Infotable::instance().GetStringInfo(
        wxString::Format("CHECK_FILTER_ID_%lld", m_AccountID), def_view);
    Document j_doc;
    if (j_doc.Parse(data.utf8_str()).HasParseError()) {
        j_doc.Parse("{}");
    }

    Value& j_filter = GetValueByPointerWithDefault(j_doc, "/FILTER", "");
    m_filter_id = j_filter.IsString() ?
        FILTER_STR.Index(wxString::FromUTF8(j_filter.GetString())) :
        FILTER_ID_ALL;
    if (m_filter_id < 0 || m_filter_id >= FILTER_ID_MAX)
        m_filter_id = FILTER_ID_ALL;

    m_scheduled_selected = false;
    if (!isTrash_ && j_doc.HasMember("SCHEDULED") && j_doc["SCHEDULED"].IsBool())
        m_scheduled_selected = j_doc["SCHEDULED"].GetBool();
}
//----------------------------------------------------------------------------

void mmCheckingPanel::saveFilterChoices()
{
   const wxString& def_view = wxString::Format(R"({ "FILTER": "%s" })"
        , Model_Setting::instance().ViewTransactions());
    wxString json = Model_Infotable::instance().GetStringInfo(
        wxString::Format("CHECK_FILTER_ID_%lld", m_AccountID), def_view);

    Document j_doc;
    if (j_doc.Parse(json.utf8_str()).HasParseError() || !j_doc.IsArray()) {
        j_doc.Parse("{}");
    }

    int menu_index = m_transFilterActive ? FILTER_ID_DIALOG : m_filter_id;
    auto menu_item = FILTER_STR[menu_index];
    if (j_doc.HasMember("FILTER")) {
        j_doc["FILTER"].SetString(menu_item.mb_str(), j_doc.GetAllocator());
    }
    else
    {
        auto& allocator = j_doc.GetAllocator();
        rapidjson::Value key("FILTER", allocator);
        rapidjson::Value value(menu_item.mb_str(), allocator);
        j_doc.AddMember(key, value, allocator);
    }

    if (!isTrash_)
    {
        if (j_doc.HasMember("SCHEDULED")) {
            j_doc["SCHEDULED"].SetBool(m_scheduled_selected);
        }
        else
        {
            auto& allocator = j_doc.GetAllocator();
            rapidjson::Value key("SCHEDULED", allocator);
            j_doc.AddMember(key, m_scheduled_selected, allocator);
        }
    }

    json = JSON_PrettyFormated(j_doc);
    Model_Infotable::instance().Set(wxString::Format("CHECK_FILTER_ID_%lld", m_AccountID), json);
}
//----------------------------------------------------------------------------

void mmCheckingPanel::updateFilterState()
{
    m_transFilterActive = false;
    m_bitmapTransFilter->UnsetToolTip();
    wxSharedPtr<mmDateRange> date_range(new mmAllTime);

    m_begin_date = "";
    m_end_date = "";

    switch (m_filter_id) {
    case FILTER_ID_TODAY:
        date_range = new mmToday; break;
    case FILTER_ID_CURRENTMONTH:
        date_range = new mmCurrentMonth; break;
    case FILTER_ID_LAST30:
        date_range = new mmLast30Days; break;
    case FILTER_ID_LAST90:
        date_range = new mmLast90Days; break;
    case FILTER_ID_LASTMONTH:
        date_range = new mmLastMonth; break;
    case FILTER_ID_LAST3MONTHS:
        date_range = new mmLast3Months; break;
    case FILTER_ID_LAST12MONTHS:
        date_range = new mmLast12Months; break;
    case  FILTER_ID_CURRENTYEAR:
        date_range = new mmCurrentYear; break;
    case  FILTER_ID_CURRENTFINYEAR:
        date_range = new mmCurrentFinancialYear(); break;
    case  FILTER_ID_LASTYEAR:
        date_range = new mmLastYear; break;
    case  FILTER_ID_LASTFINYEAR:
        date_range = new mmLastFinancialYear(); break;
    case  FILTER_ID_STATEMENTDATE:
        date_range = new mmSpecifiedRange(
            Model_Account::DateOf(m_account->STATEMENTDATE).Add(wxDateSpan::Day())
            , wxDateTime::Today()
        );

        if (!Option::instance().getIgnoreFutureTransactions())
            date_range->set_end_date(date_range->future_date());

        break;
    case FILTER_ID_DIALOG:
        m_transFilterActive = true;
        break;
    }

    if (m_begin_date.empty()) {
        m_begin_date = date_range->start_date().FormatISOCombined();
    }

    if (m_end_date.empty()) {
        m_end_date = date_range->end_date().FormatISOCombined();
    }

    auto item = m_transFilterActive ? FILTER_STR[FILTER_ID_DIALOG] : FILTER_STR[m_filter_id];
    m_bitmapTransFilter->SetLabel(wxGetTranslation(item));
    m_bitmapTransFilter->SetBitmap(m_transFilterActive ?
        mmBitmapBundle(png::TRANSFILTER_ACTIVE, mmBitmapButtonSize) :
        mmBitmapBundle(png::TRANSFILTER, mmBitmapButtonSize));

    //Text field for name of day of the week
    wxSize buttonSize(wxDefaultSize);
    buttonSize.IncTo(GetTextExtent(wxGetTranslation(item)));
    int width = buttonSize.GetWidth();
    if (width < 200) width = 200;
    m_bitmapTransFilter->SetMinSize(
        wxSize(width + Option::instance().getIconSize() * 2, -1));

    m_scheduled_allowed = !isTrash_ &&
        (m_filter_id >= FILTER_ID_TODAY && m_filter_id <= FILTER_ID_LASTFINYEAR);

    if (!isTrash_)
    {
        m_header_scheduled->SetValue(m_scheduled_selected);
        m_header_scheduled->Enable(m_scheduled_allowed);
        updateScheduledToolTip();
    }

    saveFilterChoices();
}

void mmCheckingPanel::updateScheduledToolTip()
{
    mmToolTip(m_header_scheduled,
        !m_scheduled_allowed ? _("Unable to show scheduled transactions because the current filter choice extends into the future without limit.") :
        !m_scheduled_selected ? _("Click to show scheduled transactions. This feature works best with filter choices that extend into the future (e.g., Current Month).") :
        _("Click to hide scheduled transactions."));
}

void mmCheckingPanel::OnViewPopupSelected(wxCommandEvent& event)
{
    int oldView = m_filter_id;
    m_filter_id = event.GetId() - wxID_HIGHEST;

    if (m_filter_id == FILTER_ID_DIALOG)
    {
        if (!m_trans_filter_dlg) {
            const wxString& def_view = wxString::Format(R"({ "FILTER": "%s" })", Model_Setting::instance().ViewTransactions());
            wxString json = Model_Infotable::instance().GetStringInfo(wxString::Format("CHECK_FILTER_ID_ADV_%lld", m_AccountID), def_view);
            m_trans_filter_dlg.reset(new mmFilterTransactionsDialog(this, m_AccountID, false, json));
        }

        const auto json_settings = m_trans_filter_dlg->mmGetJsonSetings();
        int status =  m_trans_filter_dlg->ShowModal();
        if (oldView == FILTER_ID_DIALOG)
        {
            if (status != wxID_OK)
                m_trans_filter_dlg.reset(new mmFilterTransactionsDialog(this, m_AccountID, false, json_settings));
        }
        else
        {
            m_transFilterActive = (status == wxID_OK && m_trans_filter_dlg->mmIsSomethingChecked());
            if (!m_transFilterActive)
                m_filter_id = oldView;
        }
    }
    else
    {
        m_transFilterActive = false;
    }

    updateFilterState();

    if (m_transFilterActive)
        m_bitmapTransFilter->SetToolTip(m_trans_filter_dlg->mmGetDescriptionToolTip());

    RefreshList();
}

void mmCheckingPanel::OnScheduled(wxCommandEvent&)
{
    if (!isTrash_) {
        m_scheduled_selected = m_header_scheduled->GetValue();
        updateScheduledToolTip();
        saveFilterChoices();
    }
    RefreshList();
}

void mmCheckingPanel::OnSearchTxtEntered(wxCommandEvent& event)
{
    const wxString search_string = event.GetString();
    if (search_string.IsEmpty()) return;

    m_listCtrlAccount->doSearchText(search_string);
}

void mmCheckingPanel::DisplaySplitCategories(Fused_Transaction::IdB fused_id)
{
    Fused_Transaction::Data fused = !fused_id.second ?
        Fused_Transaction::Data(*Model_Checking::instance().get(fused_id.first)) :
        Fused_Transaction::Data(*Model_Billsdeposits::instance().get(fused_id.first));
    std::vector<Split> splits;
    for (const auto& split : Fused_Transaction::split(fused)) {
        Split s;
        s.CATEGID          = split.CATEGID;
        s.SPLITTRANSAMOUNT = split.SPLITTRANSAMOUNT;
        s.NOTES            = split.NOTES;
        splits.push_back(s);
    }
    if (splits.empty()) return;
    int tranType = Model_Checking::type_id(fused.TRANSCODE);
    mmSplitTransactionDialog splitTransDialog(this, splits, m_AccountID, tranType, 0.0, true);

    //splitTransDialog.SetDisplaySplitCategories();
    splitTransDialog.ShowModal();
}

void mmCheckingPanel::RefreshList()
{
    m_listCtrlAccount->refreshVisualList();
}

void mmCheckingPanel::ResetColumnView()
{
    m_listCtrlAccount->DeleteAllColumns();
    m_listCtrlAccount->resetColumns();
    m_listCtrlAccount->refreshVisualList();
}

void mmCheckingPanel::SetSelectedTransaction(Fused_Transaction::IdRepeat fused_id)
{
    m_listCtrlAccount->setSelectedID(fused_id);
    RefreshList();
    m_listCtrlAccount->SetFocus();
}

// Refresh account screen with new details
void mmCheckingPanel::DisplayAccountDetails(int64 accountID)
{
    wxASSERT (-1 != accountID); // should not be called for all transaction view

    m_listCtrlAccount->setVisibleItemIndex(-1);
    m_AccountID = accountID;
    m_account = Model_Account::instance().get(m_AccountID);
    m_currency = Model_Account::currency(m_account);

    initFilterChoices();
    updateFilterState();

    if (m_transFilterActive)
    {
        const wxString& def_view = wxString::Format("{ \"FILTER\": \"%s\" }", Model_Setting::instance().ViewTransactions());
        wxString json = Model_Infotable::instance().GetStringInfo(wxString::Format("CHECK_FILTER_ID_ADV_%lld", m_AccountID), def_view);
        m_trans_filter_dlg.reset(new mmFilterTransactionsDialog(this, m_AccountID, false, json));
        m_bitmapTransFilter->SetToolTip(m_trans_filter_dlg->mmGetDescriptionToolTip());
    }

    RefreshList();
    showTips();

    enableButtons(false, false, false, false, false, false);
}

void mmCheckingPanel::mmPlayTransactionSound()
{
    int play = Model_Setting::instance().GetIntSetting(INIDB_USE_TRANSACTION_SOUND, 0);
    if (play)
    {
        wxString wav_path;
        switch (play) {
        case 2:
            wav_path = mmex::getPathResource(mmex::TRANS_SOUND2);
            break;
        default:
            wav_path = mmex::getPathResource(mmex::TRANS_SOUND1);
            break;
        }

        wxLogDebug("%s", wav_path);
        wxSound registerSound(wav_path);

        if (registerSound.IsOk())
            registerSound.Play(wxSOUND_ASYNC);
    }
}
