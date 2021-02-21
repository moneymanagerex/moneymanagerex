/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2014-2020 Nikolay Akimov
 Copyright (C) 2021 Mark Whalley (mark@ipx.co.uk)

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

//----------------------------------------------------------------------------

#include <wx/srchctrl.h>
#include <algorithm>
#include <wx/sound.h>
//----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(mmCheckingPanel, wxPanel)
    EVT_BUTTON(wxID_NEW,       mmCheckingPanel::OnNewTransaction)
    EVT_BUTTON(wxID_EDIT,      mmCheckingPanel::OnEditTransaction)
    EVT_BUTTON(wxID_REMOVE,    mmCheckingPanel::OnDeleteTransaction)
    EVT_BUTTON(wxID_DUPLICATE, mmCheckingPanel::OnDuplicateTransaction)
    EVT_BUTTON(wxID_FILE,      mmCheckingPanel::OnOpenAttachment)
    EVT_BUTTON(ID_TRX_FILTER,  mmCheckingPanel::OnMouseLeftDown)
    EVT_SEARCHCTRL_SEARCH_BTN(wxID_FIND, mmCheckingPanel::OnSearchTxtEntered)
    EVT_MENU_RANGE(wxID_HIGHEST + MENU_VIEW_ALLTRANSACTIONS, wxID_HIGHEST + MENU_VIEW_ALLTRANSACTIONS + menu_labels().size()
        , mmCheckingPanel::OnViewPopupSelected)
wxEND_EVENT_TABLE()
//----------------------------------------------------------------------------

mmCheckingPanel::mmCheckingPanel(wxWindow *parent, mmGUIFrame *frame, int accountID, int id) 
    : m_filteredBalance(0.0)
    , m_listCtrlAccount()
    , m_AccountID(accountID)
    , m_allAccounts((-1 == accountID) ? true : false)
    , m_trans_filter_dlg(nullptr)
    , m_frame(frame)
{
    Create(parent, id);
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
    wxWindow *parent,
    wxWindowID winid, const wxPoint& pos,
    const wxSize& size,long style, const wxString& name
)
{
    if (m_allAccounts)
    {
        m_currency = Model_Currency::GetBaseCurrency();
    } else 
    {
        m_account = Model_Account::instance().get(m_AccountID);
        m_currency = Model_Account::currency(m_account);
    }

    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    if (! wxPanel::Create(parent, winid, pos, size, style, name)) return false;

    this->windowsFreezeThaw();
    CreateControls();
    initViewTransactionsHeader();

    m_transFilterActive = false;
    m_trans_filter_dlg = new mmFilterTransactionsDialog(this);
    initFilterSettings();

    RefreshList();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    this->windowsFreezeThaw();

    Model_Usage::instance().pageview(this);

    return true;
}

void mmCheckingPanel::sortTable()
{
    m_listCtrlAccount->sortTable();
}

void mmCheckingPanel::filterTableAll()
{
    m_listCtrlAccount->m_trans.clear();
 
    bool ignore_future = Option::instance().getIgnoreFutureTransactions();
    const wxString today_date_string = wxDate::Today().FormatISODate();

    const auto splits = Model_Splittransaction::instance().get_all();
    const auto attachments = Model_Attachment::instance().get_all(Model_Attachment::TRANSACTION);
    for (const auto& tran : Model_Checking::instance().all())
    {
        if (ignore_future) {
            if (tran.TRANSDATE > today_date_string) continue;
        }

        if (m_transFilterActive)
        {
            if (!m_trans_filter_dlg->checkAll(tran, -1, splits))
                continue;
        }
        else
        {
            if (m_currentView != MENU_VIEW_ALLTRANSACTIONS)
            {
                if (tran.TRANSDATE < m_begin_date) continue;
                if (tran.TRANSDATE > m_end_date) continue;
            }
        }

        Model_Checking::Full_Data full_tran(tran, splits);
        full_tran.PAYEENAME = full_tran.real_payee_name(m_AccountID);
        full_tran.AMOUNT = Model_Checking::amount(tran);

        if (attachments.count(full_tran.TRANSID))
            full_tran.NOTES.Prepend(mmAttachmentManage::GetAttachmentNoteSign());

        m_listCtrlAccount->m_trans.push_back(full_tran);
    }
}

void mmCheckingPanel::filterTable()
{
    m_listCtrlAccount->m_trans.clear();

    m_account_balance = m_account ? m_account->INITIALBAL : 0.0;
    m_reconciled_balance = m_account_balance;
    m_filteredBalance = 0.0;

    bool ignore_future = Option::instance().getIgnoreFutureTransactions();
    const wxString today_date_string = wxDate::Today().FormatISODate();

    const auto splits = Model_Splittransaction::instance().get_all();
    const auto attachments = Model_Attachment::instance().get_all(Model_Attachment::TRANSACTION);
    for (const auto& tran : Model_Account::transaction(this->m_account))
    {
        double transaction_amount = Model_Checking::amount(tran, m_AccountID);
        if (Model_Checking::status(tran.STATUS) != Model_Checking::VOID_)
            m_account_balance += transaction_amount;

        if (Model_Checking::status(tran.STATUS) == Model_Checking::RECONCILED)
            m_reconciled_balance += transaction_amount;

        if (ignore_future) {
            if (tran.TRANSDATE > today_date_string) continue;
        }

        if (m_transFilterActive)
        {
            if (!m_trans_filter_dlg->checkAll(tran, m_AccountID, splits))
                continue;
        }
        else
        {
            if (m_currentView != MENU_VIEW_ALLTRANSACTIONS)
            {
                if (tran.TRANSDATE < m_begin_date) continue;
                if (tran.TRANSDATE > m_end_date) continue;
            }
        }

        Model_Checking::Full_Data full_tran(tran, splits);
        full_tran.PAYEENAME = full_tran.real_payee_name(m_AccountID);
        full_tran.BALANCE = m_account_balance;
        full_tran.AMOUNT = transaction_amount;
        m_filteredBalance += transaction_amount;

        if (attachments.count(full_tran.TRANSID))
            full_tran.NOTES.Prepend(mmAttachmentManage::GetAttachmentNoteSign());

        m_listCtrlAccount->m_trans.push_back(full_tran);
    }
}

void mmCheckingPanel::OnMouseLeftDown(wxCommandEvent& event)
{
    wxMenu menu;
    int id = MENU_VIEW_ALLTRANSACTIONS;
    for (const auto& i : menu_labels())
    {
        if (!m_allAccounts || (MENU_VIEW_STATEMENTDATE != id))
            menu.Append(wxID_HIGHEST + id, wxGetTranslation(i));
        id++;
    }
    PopupMenu(&menu);

    event.Skip();
}

//----------------------------------------------------------------------------

void mmCheckingPanel::CreateControls()
{
    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer9);

    /* ---------------------- */
    wxPanel* headerPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition
        , wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);
    itemBoxSizer9->Add(headerPanel, g_flagsBorder1V);

    wxBoxSizer* itemBoxSizerVHeader = new wxBoxSizer(wxVERTICAL);
    headerPanel->SetSizer(itemBoxSizerVHeader);

    wxGridSizer* itemBoxSizerVHeader2 = new wxGridSizer(0, 1, 5, 20);
    itemBoxSizerVHeader->Add(itemBoxSizerVHeader2);

    m_header_text = new wxStaticText( headerPanel, wxID_STATIC, "");
    m_header_text->SetFont(this->GetFont().Larger().Bold());
    itemBoxSizerVHeader2->Add(m_header_text, g_flagsBorder1H);

    wxBoxSizer* itemBoxSizerHHeader2 = new wxBoxSizer(wxHORIZONTAL);
    wxFlexGridSizer* itemFlexGridSizerHHeader2 = new wxFlexGridSizer(3, 1, 1);
    itemBoxSizerVHeader2->Add(itemBoxSizerHHeader2);
    itemBoxSizerHHeader2->Add(itemFlexGridSizerHHeader2);

    m_bitmapTransFilter = new wxButton(headerPanel, ID_TRX_FILTER);
    m_bitmapTransFilter->SetBitmap(mmBitmap(png::RIGHTARROW));
    m_bitmapTransFilter->SetMinSize(wxSize(220, -1));

    itemFlexGridSizerHHeader2->Add(m_bitmapTransFilter, g_flagsBorder1H);

    itemFlexGridSizerHHeader2->AddSpacer(20);

    m_statTextTransFilter = new wxStaticText(headerPanel, wxID_ANY, "");
    itemFlexGridSizerHHeader2->Add(m_statTextTransFilter, g_flagsBorder1H);

    if (!m_allAccounts)  // not relevant to full transaction view
    {
        wxStaticText* itemStaticText12 = new wxStaticText(headerPanel
            , ID_PANEL_CHECKING_STATIC_BALHEADER1, "$", wxDefaultPosition, wxSize(120, -1));
        wxStaticText* itemStaticText14 = new wxStaticText(headerPanel
            , ID_PANEL_CHECKING_STATIC_BALHEADER2, "$", wxDefaultPosition, wxSize(120, -1));
        wxStaticText* itemStaticText16 = new wxStaticText(headerPanel
            , ID_PANEL_CHECKING_STATIC_BALHEADER3, "$", wxDefaultPosition, wxSize(120, -1));
        wxStaticText* itemStaticText17 = new wxStaticText(headerPanel
            , ID_PANEL_CHECKING_STATIC_BALHEADER4, _("Displayed Bal: "));
        wxStaticText* itemStaticText18 = new wxStaticText(headerPanel
            , ID_PANEL_CHECKING_STATIC_BALHEADER5, "$", wxDefaultPosition, wxSize(120, -1));

        wxFlexGridSizer* balances_header = new wxFlexGridSizer(0,8,5,10);
        itemBoxSizerVHeader->Add(balances_header, g_flagsExpandBorder1);
        balances_header->Add(new wxStaticText(headerPanel, wxID_STATIC, _("Account Bal: ")));
        balances_header->Add(itemStaticText12);
        balances_header->Add(new wxStaticText(headerPanel,  wxID_STATIC, _("Reconciled Bal: ")));
        balances_header->Add(itemStaticText14);
        balances_header->Add(new wxStaticText(headerPanel, wxID_STATIC, _("Diff: ")));
        balances_header->Add(itemStaticText16);
        balances_header->Add(itemStaticText17);
        balances_header->Add(itemStaticText18);
    }

    /* ---------------------- */

    wxSplitterWindow* itemSplitterWindow10 = new wxSplitterWindow(this
        , wxID_ANY, wxDefaultPosition, wxSize(200, 200)
        , wxSP_3DBORDER | wxSP_3DSASH | wxNO_BORDER);

    m_imageList.reset(createImageList());
    m_imageList->Add(mmBitmap(png::RECONCILED));
    m_imageList->Add(mmBitmap(png::VOID_STAT));
    m_imageList->Add(mmBitmap(png::FOLLOW_UP));
    m_imageList->Add(mmBitmap(png::EMPTY));
    m_imageList->Add(mmBitmap(png::DUPLICATE_STAT));
    m_imageList->Add(mmBitmap(png::UPARROW));
    m_imageList->Add(mmBitmap(png::DOWNARROW));

    m_listCtrlAccount = new TransactionListCtrl(this, itemSplitterWindow10);

    m_listCtrlAccount->SetImageList(m_imageList.get(), wxIMAGE_LIST_SMALL);
    m_listCtrlAccount->setSortOrder(m_listCtrlAccount->g_asc);
    m_listCtrlAccount->setSortColumn(m_listCtrlAccount->g_sortcol);
    m_listCtrlAccount->SetFocus();

    m_listCtrlAccount->createColumns(*m_listCtrlAccount);

    // load the global variables
    long val = m_listCtrlAccount->COL_DEF_SORT;
    wxString strVal = Model_Setting::instance().GetStringSetting("CHECK_SORT_COL", wxString() << val);
    if (strVal.ToLong(&val)) m_listCtrlAccount->g_sortcol = m_listCtrlAccount->toEColumn(val);
    // --
    val = 1; // asc sorting default
    strVal = Model_Setting::instance().GetStringSetting("CHECK_ASC", wxString() << val);
    if (strVal.ToLong(&val)) m_listCtrlAccount->g_asc = val != 0;

    // --
    m_listCtrlAccount->setSortColumn(m_listCtrlAccount->g_sortcol);
    m_listCtrlAccount->setSortOrder(m_listCtrlAccount->g_asc);
    m_listCtrlAccount->setColumnImage(m_listCtrlAccount->getSortColumn()
        , m_listCtrlAccount->getSortOrder() ? m_listCtrlAccount->ICON_ASC : m_listCtrlAccount->ICON_DESC); // asc\desc sort mark (arrow)

    wxPanel *itemPanel12 = new wxPanel(itemSplitterWindow10, wxID_ANY
        , wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxTAB_TRAVERSAL);

    itemSplitterWindow10->SplitHorizontally(m_listCtrlAccount, itemPanel12);
    itemSplitterWindow10->SetMinimumPaneSize(100);
    itemSplitterWindow10->SetSashGravity(1.0);

    itemBoxSizer9->Add(itemSplitterWindow10, g_flagsExpandBorder1);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemPanel12->SetSizer(itemBoxSizer4);

    wxBoxSizer* itemButtonsSizer = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer4->Add(itemButtonsSizer, g_flagsBorder1V);

    m_btnNew = new wxButton(itemPanel12, wxID_NEW, _("&New "));
    m_btnNew->SetToolTip(_("New Transaction"));
    itemButtonsSizer->Add(m_btnNew, 0, wxRIGHT, 5);

    m_btnEdit = new wxButton(itemPanel12, wxID_EDIT, _("&Edit "));
    m_btnEdit->SetToolTip(_("Edit selected transaction"));
    itemButtonsSizer->Add(m_btnEdit, 0, wxRIGHT, 5);
    m_btnEdit->Enable(false);

    m_btnDelete = new wxButton(itemPanel12, wxID_REMOVE, _("&Delete "));
    m_btnDelete->SetToolTip(_("Delete selected transaction"));
    itemButtonsSizer->Add(m_btnDelete, 0, wxRIGHT, 5);
    m_btnDelete->Enable(false);

    m_btnDuplicate = new wxButton(itemPanel12, wxID_DUPLICATE, _("D&uplicate "));
    m_btnDuplicate->SetToolTip(_("Duplicate selected transaction"));
    itemButtonsSizer->Add(m_btnDuplicate, 0, wxRIGHT, 5);
    m_btnDuplicate->Enable(false);

    m_btnAttachment = new wxBitmapButton(itemPanel12, wxID_FILE
        , mmBitmap(png::CLIP), wxDefaultPosition
        , wxSize(30, m_btnDuplicate->GetSize().GetY()));
    m_btnAttachment->SetToolTip(_("Open attachments"));
    itemButtonsSizer->Add(m_btnAttachment, 0, wxRIGHT, 5);
    m_btnAttachment->Enable(false);

    wxSearchCtrl* searchCtrl = new wxSearchCtrl(itemPanel12
        , wxID_FIND, wxEmptyString, wxDefaultPosition
        , wxSize(100, m_btnDuplicate->GetSize().GetHeight())
        , wxTE_NOHIDESEL, wxDefaultValidator);
    searchCtrl->SetDescriptiveText(_("Search"));
    itemButtonsSizer->Add(searchCtrl, 0, wxCENTER, 1);
    searchCtrl->SetToolTip(_("Enter any string to find it in the nearest transaction notes"));

    //Infobar-mini
    m_info_panel_mini = new wxStaticText(itemPanel12, wxID_STATIC, "");
    itemButtonsSizer->Add(m_info_panel_mini, 1, wxGROW | wxTOP | wxLEFT, 5);

    //Infobar
    m_info_panel = new wxStaticText(itemPanel12, wxID_STATIC, ""
        , wxDefaultPosition, wxSize(200, -1), wxTE_MULTILINE | wxTE_WORDWRAP);
    itemBoxSizer4->Add(m_info_panel, g_flagsExpandBorder1);
    //Show tips when no any transaction selected
    showTips();
}

wxString mmCheckingPanel::GetPanelTitle(const Model_Account::Data& account) const
{
    if (m_allAccounts)
        return wxString::Format(_("Full Transactions Report"));
    else
        return wxString::Format(_("Account View : %s"), account.ACCOUNTNAME);
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

    if (!m_allAccounts)
    {
        bool show_displayed_balance_ = (m_transFilterActive || m_currentView != MENU_VIEW_ALLTRANSACTIONS);
        wxStaticText* header = static_cast<wxStaticText*>(FindWindow(ID_PANEL_CHECKING_STATIC_BALHEADER1));
        header->SetLabelText(Model_Account::toCurrency(m_account_balance, account));
        header = static_cast<wxStaticText*>(FindWindow(ID_PANEL_CHECKING_STATIC_BALHEADER2));
        header->SetLabelText(Model_Account::toCurrency(m_reconciled_balance, account));
        header = static_cast<wxStaticText*>(FindWindow(ID_PANEL_CHECKING_STATIC_BALHEADER3));
        header->SetLabelText(Model_Account::toCurrency(m_account_balance - m_reconciled_balance, account));
        header = static_cast<wxStaticText*>(FindWindow(ID_PANEL_CHECKING_STATIC_BALHEADER4));
        header->SetLabelText(show_displayed_balance_
            ? _("Displayed Bal: ") : "");
        header = static_cast<wxStaticText*>(FindWindow(ID_PANEL_CHECKING_STATIC_BALHEADER5));
        header->SetLabelText(show_displayed_balance_
            ? Model_Account::toCurrency(m_filteredBalance, account) : "");
    }
    this->Layout();
}

//----------------------------------------------------------------------------
void mmCheckingPanel::enableTransactionButtons(bool editDelete, bool dupAttach)
{
 
    m_btnEdit->Enable(editDelete);
    m_btnDelete->Enable(editDelete);
    m_btnDuplicate->Enable(editDelete);
    m_btnAttachment->Enable(editDelete);

    m_btnDuplicate->Enable(dupAttach);
    m_btnAttachment->Enable(dupAttach);

}
//----------------------------------------------------------------------------

void mmCheckingPanel::updateExtraTransactionData(bool single)
{
    if (single)
    {
        enableTransactionButtons(true, true);
        int trx_id = m_listCtrlAccount->getSelectedId()[0];
        const Model_Checking::Data* trx = Model_Checking::instance().get(trx_id);
        Model_Checking::Full_Data full_tran(*trx);
        m_info_panel->SetLabelText(full_tran.NOTES);
        wxString miniStr = full_tran.info();

        //Show only first line but full string set as tooltip
        if (miniStr.Find("\n") > 1 && !miniStr.IsEmpty())
        {
            m_info_panel_mini->SetLabelText(miniStr.substr(0, miniStr.Find("\n")) + " ...");
            m_info_panel_mini->SetToolTip(miniStr);
        }
        else
        {
            m_info_panel_mini->SetLabelText(miniStr);
            m_info_panel_mini->SetToolTip(miniStr);
        }

    }
    else
    {
        if (m_listCtrlAccount->getSelectedId().size() > 0)
            enableTransactionButtons(true, false);
        else
            enableTransactionButtons(false, false);
        m_info_panel_mini->SetLabelText("");

        showTips();
    }
}
//----------------------------------------------------------------------------
void mmCheckingPanel::showTips()
{
    m_info_panel->SetLabelText(wxGetTranslation(TIPS[rand() % (sizeof(TIPS) / sizeof(wxString))]));
}
//----------------------------------------------------------------------------

void mmCheckingPanel::OnDeleteTransaction(wxCommandEvent& event)
{
    m_listCtrlAccount->OnDeleteTransaction(event);
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


void mmCheckingPanel::initViewTransactionsHeader()
{
    const wxString& def_view = Model_Setting::instance().ViewTransactions();
    m_currentView = menu_labels().Index(Model_Infotable::instance().GetStringInfo(wxString::Format("CHECK_FILTER_ID_%d", m_AccountID), def_view));
    if (m_currentView < 0 || m_currentView >= static_cast<int>(menu_labels().size()))
        m_currentView = menu_labels().Index(VIEW_TRANS_ALL_STR);

}
//----------------------------------------------------------------------------
void mmCheckingPanel::initFilterSettings()
{
    m_transFilterActive = false;
    wxString label = "";
    m_bitmapTransFilter->UnsetToolTip();
    mmDateRange* date_range = NULL;

    m_begin_date = "";
    m_end_date = "";

    switch (m_currentView) {
    case MENU_VIEW_TODAY:
        date_range = new mmToday;
        break;
    case MENU_VIEW_CURRENTMONTH:
        date_range = new mmCurrentMonth;
        break;
    case MENU_VIEW_LAST30:
        date_range = new mmLast30Days;
        break;
    case MENU_VIEW_LAST90:
        date_range = new mmLast90Days;
        break;
    case MENU_VIEW_LASTMONTH:
        date_range = new mmLastMonth;
        break;
    case MENU_VIEW_LAST3MONTHS:
        date_range = new mmLast3Months;
        break;
    case MENU_VIEW_LAST12MONTHS:
        date_range = new mmLast12Months;
        break;
    case  MENU_VIEW_CURRENTYEAR:
        date_range = new mmCurrentYear;
        break;
    case  MENU_VIEW_CURRENTFINANCIALYEAR:
        date_range = new mmCurrentFinancialYear(wxAtoi(Option::instance().FinancialYearStartDay())
            , wxAtoi(Option::instance().FinancialYearStartMonth()));
        break;
    case  MENU_VIEW_LASTYEAR:
        date_range = new mmLastYear;
        break;
    case  MENU_VIEW_LASTFINANCIALYEAR:
        date_range = new mmLastFinancialYear(wxAtoi(Option::instance().FinancialYearStartDay())
            , wxAtoi(Option::instance().FinancialYearStartMonth()));
        break;
    case  MENU_VIEW_STATEMENTDATE:
        if (Model_Account::BoolOf(m_account->STATEMENTLOCKED))
        {
            date_range = new mmSpecifiedRange(Model_Account::DateOf(m_account->STATEMENTDATE)
                .Add(wxDateSpan::Day()), wxDateTime::Today());
            label = mmGetDateForDisplay(date_range->start_date().FormatISODate());
        }
        break;
    case MENU_VIEW_FILTER_DIALOG:
        m_bitmapTransFilter->SetToolTip(m_trans_filter_dlg->getDescriptionToolTip());
        m_transFilterActive = true;
        break;
    }

    if (date_range == NULL) {
        date_range = new mmAllTime;
    }

    if (m_begin_date.empty()) {
        m_begin_date = date_range->start_date().FormatISODate();
    }

    if (m_end_date.empty()) {
        m_end_date = date_range->end_date().FormatISODate();
    }
    delete date_range;

    const auto item = menu_labels()[m_currentView];
    Model_Infotable::instance().Set(wxString::Format("CHECK_FILTER_ID_%d", m_AccountID), item);
    m_bitmapTransFilter->SetLabel(wxGetTranslation(item));
    m_bitmapTransFilter->SetBitmap(m_transFilterActive ? mmBitmap(png::RIGHTARROW_ACTIVE) : mmBitmap(png::RIGHTARROW));
    m_statTextTransFilter->SetLabelText(label);
}

void mmCheckingPanel::OnViewPopupSelected(wxCommandEvent& event)
{
    m_currentView = event.GetId() - wxID_HIGHEST;

    m_transFilterActive = false;

    if (m_currentView == MENU_VIEW_FILTER_DIALOG)
    {
        m_trans_filter_dlg->setAccountToolTip(_("Select account used in transfer transactions"));
        m_transFilterActive = (m_trans_filter_dlg->ShowModal() == wxID_OK
            && m_trans_filter_dlg->isSomethingSelected());
        if (!m_transFilterActive)
            m_currentView = MENU_VIEW_ALLTRANSACTIONS;
    }

    initFilterSettings();
    RefreshList();
}

void mmCheckingPanel::OnSearchTxtEntered(wxCommandEvent& event)
{
    const wxString search_string = event.GetString().Lower();
    if (search_string.IsEmpty()) return;

    m_listCtrlAccount->doSearchText(search_string);

}

void mmCheckingPanel::DisplaySplitCategories(int transID)
{
    const Model_Checking::Data* tran = Model_Checking::instance().get(transID);
    int transType = Model_Checking::type(tran->TRANSCODE);

    Model_Checking::Data *transaction = Model_Checking::instance().get(transID);
    auto splits = Model_Checking::splittransaction(transaction);

    if (splits.empty()) return;

    std::vector<Split> splt;
    for (const auto& entry : splits) {
        Split s;
        s.CATEGID = entry.CATEGID;
        s.SUBCATEGID = entry.SUBCATEGID;
        s.SPLITTRANSAMOUNT = entry.SPLITTRANSAMOUNT;
        splt.push_back(s);
    }
    SplitTransactionDialog splitTransDialog(this
        , splt
        , transType
        , m_AccountID);

    splitTransDialog.SetDisplaySplitCategories();
    splitTransDialog.ShowModal();
}

void mmCheckingPanel::RefreshList()
{
    m_listCtrlAccount->refreshVisualList();
}

void mmCheckingPanel::SetSelectedTransaction(int transID)
{
    m_listCtrlAccount->setSelectedID(transID);
    RefreshList();
    m_listCtrlAccount->SetFocus();
}

// Refresh account screen with new details
void mmCheckingPanel::DisplayAccountDetails(int accountID)
{
    wxASSERT (-1 != accountID); // should not be called for all transaction view even though it should work

    m_AccountID = accountID;
    m_allAccounts = (-1 == m_AccountID) ? true : false;
    if (m_allAccounts)
    {
        m_currency = Model_Currency::GetBaseCurrency();
    } else 
    {
        m_account = Model_Account::instance().get(m_AccountID);
        m_currency = Model_Account::currency(m_account);
    }

    initViewTransactionsHeader();
    initFilterSettings();
    RefreshList();
    showTips();
    m_listCtrlAccount->getSelectedId().clear();
    enableTransactionButtons(false, false);
}

void mmCheckingPanel::mmPlayTransactionSound()
{
    bool play = Model_Setting::instance().GetBoolSetting(INIDB_USE_TRANSACTION_SOUND, true);
    if (play)
    {
        wxString wav_path = mmex::getPathResource(mmex::TRANS_SOUND);
        wxLogDebug("%s", wav_path);
        wxSound registerSound(wav_path);

        if (registerSound.IsOk())
            registerSound.Play(wxSOUND_ASYNC);
    }
}

