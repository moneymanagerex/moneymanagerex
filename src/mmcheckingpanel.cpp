/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel

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

#include "mmcheckingpanel.h"
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
#include "attachmentdialog.h"
#include "model/Model_Infotable.h"
#include "model/Model_Setting.h"
#include "model/Model_Checking.h"
#include "model/Model_Splittransaction.h"
#include "model/Model_Account.h"
#include "model/Model_Payee.h"
#include "model/Model_Category.h"
#include "model/Model_Attachment.h"
#include "billsdepositsdialog.h"


//----------------------------------------------------------------------------

#include <wx/srchctrl.h>
#include <algorithm>
#include <wx/sound.h>
//----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(mmCheckingPanel, wxPanel)
    EVT_BUTTON(wxID_NEW,         mmCheckingPanel::OnNewTransaction)
    EVT_BUTTON(wxID_EDIT,        mmCheckingPanel::OnEditTransaction)
    EVT_BUTTON(wxID_REMOVE,      mmCheckingPanel::OnDeleteTransaction)
    EVT_BUTTON(wxID_DUPLICATE,    mmCheckingPanel::OnDuplicateTransaction)
    EVT_BUTTON(wxID_FILE, mmCheckingPanel::OnOpenAttachment)
    EVT_MENU_RANGE(wxID_HIGHEST + MENU_VIEW_ALLTRANSACTIONS, wxID_HIGHEST + MENU_VIEW_ALLTRANSACTIONS + menu_labels().size()
        , mmCheckingPanel::OnViewPopupSelected)
    EVT_SEARCHCTRL_SEARCH_BTN(wxID_FIND, mmCheckingPanel::OnSearchTxtEntered)
wxEND_EVENT_TABLE()
//----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(TransactionListCtrl, mmListCtrl)
    EVT_LIST_ITEM_SELECTED(wxID_ANY, TransactionListCtrl::OnListItemSelected)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY, TransactionListCtrl::OnListItemActivated)
    EVT_RIGHT_DOWN(TransactionListCtrl::OnMouseRightClick)
    EVT_LEFT_DOWN(TransactionListCtrl::OnListLeftClick)
    EVT_LIST_KEY_DOWN(wxID_ANY,  TransactionListCtrl::OnListKeyDown)

    EVT_MENU_RANGE(MENU_TREEPOPUP_MARKRECONCILED
        , MENU_TREEPOPUP_MARKDELETE,        TransactionListCtrl::OnMarkTransaction)

    EVT_MENU_RANGE(MENU_TREEPOPUP_MARKRECONCILED_ALL
        , MENU_TREEPOPUP_DELETE_UNRECONCILED, TransactionListCtrl::OnMarkAllTransactions)

    EVT_MENU_RANGE(MENU_TREEPOPUP_NEW_WITHDRAWAL, MENU_TREEPOPUP_NEW_DEPOSIT, TransactionListCtrl::OnNewTransaction)
    EVT_MENU(MENU_TREEPOPUP_NEW_TRANSFER,   TransactionListCtrl::OnNewTransferTransaction)
    EVT_MENU(MENU_TREEPOPUP_DELETE2,        TransactionListCtrl::OnDeleteTransaction)
    EVT_MENU(MENU_TREEPOPUP_EDIT2,          TransactionListCtrl::OnEditTransaction)
    EVT_MENU(MENU_TREEPOPUP_MOVE2,          TransactionListCtrl::OnMoveTransaction)

    EVT_MENU(MENU_ON_COPY_TRANSACTION,      TransactionListCtrl::OnCopy)
    EVT_MENU(MENU_ON_PASTE_TRANSACTION,     TransactionListCtrl::OnPaste)
    EVT_MENU(MENU_ON_NEW_TRANSACTION,       TransactionListCtrl::OnNewTransaction)
    EVT_MENU(MENU_ON_DUPLICATE_TRANSACTION, TransactionListCtrl::OnDuplicateTransaction)
    EVT_MENU_RANGE(MENU_ON_SET_UDC0, MENU_ON_SET_UDC7, TransactionListCtrl::OnSetUserColour)

    EVT_MENU(MENU_TREEPOPUP_VIEW_SPLIT_CATEGORIES, TransactionListCtrl::OnViewSplitTransaction)
    EVT_MENU(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, TransactionListCtrl::OnOrganizeAttachments)
    EVT_MENU(MENU_TREEPOPUP_CREATE_REOCCURANCE, TransactionListCtrl::OnCreateReoccurance)
    EVT_CHAR(TransactionListCtrl::OnChar)

wxEND_EVENT_TABLE();

//----------------------------------------------------------------------------

mmCheckingPanel::mmCheckingPanel(wxWindow *parent, mmGUIFrame *frame, int accountID, int id) 
    : filteredBalance_(0.0)
    , m_listCtrlAccount()
    , m_AccountID(accountID)
    , m_account(Model_Account::instance().get(accountID))
    , m_currency(Model_Account::currency(m_account))
    , transFilterDlg_(0)
    , m_frame(frame)
{
    m_basecurrecyID = Model_Infotable::instance().GetBaseCurrencyId();
    long style = wxTAB_TRAVERSAL | wxNO_BORDER;
    Create(parent, mmID_CHECKING, wxDefaultPosition, wxDefaultSize, style);
}
//----------------------------------------------------------------------------

/*
    We cannon use OnClose() event because this class deletes
    via DestroyChildren() of its parent.
*/
mmCheckingPanel::~mmCheckingPanel()
{
    if (transFilterDlg_) delete transFilterDlg_;
}

bool mmCheckingPanel::Create(
    wxWindow *parent,
    wxWindowID winid, const wxPoint& pos,
    const wxSize& size,long style, const wxString& name
)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    if (! wxPanel::Create(parent, winid, pos, size, style, name)) return false;

    this->windowsFreezeThaw();
    CreateControls();

    transFilterActive_ = false;
    transFilterDlg_    = new mmFilterTransactionsDialog(this);
    SetTransactionFilterState(true);

    initViewTransactionsHeader();
    initFilterSettings();

    RefreshList();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    this->windowsFreezeThaw();

    return true;
}

void mmCheckingPanel::sortTable()
{
    std::sort(this->m_trans.begin(), this->m_trans.end());
    switch (m_listCtrlAccount->g_sortcol)
    {
    case TransactionListCtrl::COL_ID:
        std::stable_sort(this->m_trans.begin(), this->m_trans.end(),SorterByTRANSID());
        break;
    case TransactionListCtrl::COL_NUMBER:
        std::stable_sort(this->m_trans.begin(), this->m_trans.end(), Model_Checking::SorterByNUMBER());
        break;
    case TransactionListCtrl::COL_PAYEE_STR:
        std::stable_sort(this->m_trans.begin(), this->m_trans.end(), SorterByPAYEENAME());
        break;
    case TransactionListCtrl::COL_STATUS:
        std::stable_sort(this->m_trans.begin(), this->m_trans.end(), SorterBySTATUS());
        break;
    case TransactionListCtrl::COL_CATEGORY:
        std::stable_sort(this->m_trans.begin(), this->m_trans.end(), SorterByCATEGNAME());
        break;
    case TransactionListCtrl::COL_WITHDRAWAL:
        std::stable_sort(this->m_trans.begin(), this->m_trans.end(), Model_Checking::SorterByWITHDRAWAL());
        break;
    case TransactionListCtrl::COL_DEPOSIT:
        std::stable_sort(this->m_trans.begin(), this->m_trans.end(), Model_Checking::SorterByDEPOSIT());
        break;
    case TransactionListCtrl::COL_BALANCE:
        std::stable_sort(this->m_trans.begin(), this->m_trans.end(), Model_Checking::SorterByBALANCE());
        break;
    case TransactionListCtrl::COL_NOTES:
        std::stable_sort(this->m_trans.begin(), this->m_trans.end(), SorterByNOTES());
        break;
    default:
        std::stable_sort(this->m_trans.begin(), this->m_trans.end(), SorterByTRANSDATE());
        break;
    }

    if (!m_listCtrlAccount->g_asc) std::reverse(this->m_trans.begin(), this->m_trans.end());
}

void mmCheckingPanel::filterTable()
{
    this->m_trans.clear();
    account_balance_ = m_account ? m_account->INITIALBAL : 0.0;
    reconciled_balance_ = account_balance_;
    filteredBalance_ = 0.0;

    const auto splits = Model_Splittransaction::instance().get_all();
    const auto attachments = Model_Attachment::instance().get_all(Model_Attachment::TRANSACTION);
    for (const auto& tran : Model_Account::transaction(this->m_account))
    {
        double transaction_amount = Model_Checking::amount(tran, m_AccountID);
        if (Model_Checking::status(tran.STATUS) != Model_Checking::VOID_)
            account_balance_ += transaction_amount;

        if (Model_Checking::status(tran.STATUS) == Model_Checking::RECONCILED)
            reconciled_balance_ += transaction_amount;

        if (transFilterActive_)
        {
            if (!transFilterDlg_->checkAll(tran, m_AccountID, splits))
                continue;
        }
        else
        {
            if (!Model_Checking::TRANSDATE(tran).IsBetween(quickFilterBeginDate_, quickFilterEndDate_))
                continue;
        }

        Model_Checking::Full_Data full_tran(tran, splits);
        full_tran.PAYEENAME = full_tran.real_payee_name(m_AccountID);
        full_tran.BALANCE = account_balance_;
        full_tran.AMOUNT = transaction_amount;
        filteredBalance_ += transaction_amount;

        if (attachments.count(full_tran.TRANSID))
            full_tran.NOTES.Prepend(mmAttachmentManage::GetAttachmentNoteSign());

        this->m_trans.push_back(full_tran);
    }
}

void mmCheckingPanel::updateTable()
{
    account_balance_ = 0.0;
    reconciled_balance_ = 0.0;
    if (m_account)
    {
        account_balance_ = m_account->INITIALBAL;
        reconciled_balance_ = account_balance_;
    }
    for (const auto& tran : Model_Account::transaction(m_account))
    {
        double transaction_amount = Model_Checking::amount(tran, m_AccountID);
        if (Model_Checking::status(tran.STATUS) != Model_Checking::VOID_)
            account_balance_ += transaction_amount;
        reconciled_balance_ += Model_Checking::reconciled(tran, m_AccountID);
    }
    filteredBalance_ = 0.0;
    for (const auto & tran : m_trans)
    {
        filteredBalance_ += Model_Checking::amount(tran, m_AccountID);
    }

    setAccountSummary();

    if (m_listCtrlAccount->g_sortcol == TransactionListCtrl::COL_STATUS)
    {
        sortTable();
        m_listCtrlAccount->RefreshItems(0, m_trans.size() - 1);
    }
}

void mmCheckingPanel::markSelectedTransaction(int trans_id)
{
    if (trans_id > 0)
    {
        long i = 0;
        for (const auto & tran : m_trans)
        {
            if (trans_id == tran.TRANSID)
            {
                m_listCtrlAccount->m_selectedIndex = i;
                break;
            }
            ++i;
        }
    }

    if (!m_trans.empty() && m_listCtrlAccount->m_selectedIndex < 0)
    {
        if (m_listCtrlAccount->g_asc)
            m_listCtrlAccount->EnsureVisible(static_cast<long>(m_trans.size()) - 1);
        else
            m_listCtrlAccount->EnsureVisible(0);
    }
    else
    {
        enableEditDeleteButtons(false);
        showTips();
    }
}
//----------------------------------------------------------------------------

void mmCheckingPanel::OnMouseLeftDown( wxMouseEvent& event )
{
    // depending on the clicked control's window id.
    switch( event.GetId() )
    {
        case ID_PANEL_CHECKING_STATIC_BITMAP_FILTER :
        {
            wxCommandEvent ev(wxEVT_COMMAND_MENU_SELECTED, ID_PANEL_CHECKING_STATIC_BITMAP_FILTER);
            GetEventHandler()->AddPendingEvent(ev);

            break;
        }
        default:
        {
            wxMenu menu;
            int id = wxID_HIGHEST + MENU_VIEW_ALLTRANSACTIONS;
            for (const auto& i : menu_labels())
            {
                menu.Append(id++, wxGetTranslation(i));
            }
            PopupMenu(&menu, event.GetPosition());

            break;
        }
    }
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
    itemBoxSizer9->Add(headerPanel, g_flagsBorder1);

    wxBoxSizer* itemBoxSizerVHeader = new wxBoxSizer(wxVERTICAL);
    headerPanel->SetSizer(itemBoxSizerVHeader);

    wxGridSizer* itemBoxSizerVHeader2 = new wxGridSizer(0, 1, 5, 20);
    itemBoxSizerVHeader->Add(itemBoxSizerVHeader2);

    header_text_ = new wxStaticText( headerPanel, wxID_STATIC, "");
    header_text_->SetFont(this->GetFont().Larger().Bold());
    itemBoxSizerVHeader2->Add(header_text_, g_flagsBorder1);

    wxBoxSizer* itemBoxSizerHHeader2 = new wxBoxSizer(wxHORIZONTAL);
    wxFlexGridSizer* itemFlexGridSizerHHeader2 = new wxFlexGridSizer(5, 1, 1);
    itemBoxSizerVHeader2->Add(itemBoxSizerHHeader2);
    itemBoxSizerHHeader2->Add(itemFlexGridSizerHHeader2);

    bitmapMainFilter_ = new wxStaticBitmap(headerPanel, wxID_PAGE_SETUP
        , mmBitmap(png::RIGHTARROW));
    itemFlexGridSizerHHeader2->Add(bitmapMainFilter_, g_flagsBorder1);
    bitmapMainFilter_->Connect(wxID_ANY, wxEVT_RIGHT_DOWN
        , wxMouseEventHandler(mmCheckingPanel::OnFilterResetToViewAll), nullptr, this);
    bitmapMainFilter_->Connect(wxID_ANY, wxEVT_LEFT_DOWN
        , wxMouseEventHandler(mmCheckingPanel::OnMouseLeftDown), nullptr, this);

    stxtMainFilter_ = new wxStaticText(headerPanel, wxID_ANY, "", wxDefaultPosition, wxSize(250, -1));
    itemFlexGridSizerHHeader2->Add(stxtMainFilter_, g_flagsBorder1);

    itemFlexGridSizerHHeader2->AddSpacer(20);

    bitmapTransFilter_ = new wxStaticBitmap(headerPanel, ID_PANEL_CHECKING_STATIC_BITMAP_FILTER
        , mmBitmap(png::RIGHTARROW));
    itemFlexGridSizerHHeader2->Add(bitmapTransFilter_, g_flagsBorder1);
    bitmapTransFilter_->Connect(wxID_ANY, wxEVT_LEFT_DOWN
        , wxMouseEventHandler(mmCheckingPanel::OnFilterTransactions), nullptr, this);
    bitmapTransFilter_->Connect(wxID_ANY, wxEVT_RIGHT_DOWN
        , wxMouseEventHandler(mmCheckingPanel::OnFilterTransactions), nullptr, this);

    statTextTransFilter_ = new wxStaticText(headerPanel, wxID_ANY
        , _("Transaction Filter"));
    itemFlexGridSizerHHeader2->Add(statTextTransFilter_, g_flagsBorder1);

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

    /* ---------------------- */

    wxSplitterWindow* itemSplitterWindow10 = new wxSplitterWindow(this
        , wxID_ANY, wxDefaultPosition, wxSize(200, 200)
        , wxSP_3DBORDER | wxSP_3DSASH | wxNO_BORDER);

    int x = mmIniOptions::instance().ico_size_;
    m_imageList.reset(new wxImageList(x, x));
    m_imageList->Add(mmBitmap(png::RECONCILED));
    m_imageList->Add(mmBitmap(png::VOID_STAT));
    m_imageList->Add(mmBitmap(png::FOLLOW_UP));
    m_imageList->Add(mmBitmap(png::EMPTY));
    m_imageList->Add(mmBitmap(png::DUPLICATE_STAT));
    m_imageList->Add(mmBitmap(png::UPARROW));
    m_imageList->Add(mmBitmap(png::DOWNARROW));

    m_listCtrlAccount = new TransactionListCtrl(this, itemSplitterWindow10
        , wxID_ANY);

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
    itemBoxSizer4->Add(itemButtonsSizer, g_flagsBorder1);

    btnNew_ = new wxButton(itemPanel12, wxID_NEW, _("&New "));
    btnNew_->SetToolTip(_("New Transaction"));
    itemButtonsSizer->Add(btnNew_, 0, wxRIGHT, 5);

    btnEdit_ = new wxButton(itemPanel12, wxID_EDIT, _("&Edit "));
    btnEdit_->SetToolTip(_("Edit selected transaction"));
    itemButtonsSizer->Add(btnEdit_, 0, wxRIGHT, 5);
    btnEdit_->Enable(false);

    btnDelete_ = new wxButton(itemPanel12, wxID_REMOVE, _("&Delete "));
    btnDelete_->SetToolTip(_("Delete selected transaction"));
    itemButtonsSizer->Add(btnDelete_, 0, wxRIGHT, 5);
    btnDelete_->Enable(false);

    btnDuplicate_ = new wxButton(itemPanel12, wxID_DUPLICATE, _("D&uplicate "));
    btnDuplicate_->SetToolTip(_("Duplicate selected transaction"));
    itemButtonsSizer->Add(btnDuplicate_, 0, wxRIGHT, 5);
    btnDuplicate_->Enable(false);

    btnAttachment_ = new wxBitmapButton(itemPanel12, wxID_FILE
        , mmBitmap(png::CLIP), wxDefaultPosition
        , wxSize(30, btnDuplicate_->GetSize().GetY()));
    btnAttachment_->SetToolTip(_("Open attachments"));
    itemButtonsSizer->Add(btnAttachment_, 0, wxRIGHT, 5);
	btnAttachment_->Enable(false);

    wxSearchCtrl* searchCtrl = new wxSearchCtrl(itemPanel12
        , wxID_FIND, wxEmptyString, wxDefaultPosition
        , wxSize(100, btnDuplicate_->GetSize().GetHeight())
        , wxTE_NOHIDESEL, wxDefaultValidator);
    searchCtrl->SetDescriptiveText(_("Search"));
    itemButtonsSizer->Add(searchCtrl, 0, wxCENTER, 1);
    searchCtrl->SetToolTip(_("Enter any string to find it in the nearest transaction notes"));

    //Infobar-mini
    info_panel_mini_ = new wxStaticText(itemPanel12, wxID_STATIC, "");
    itemButtonsSizer->Add(info_panel_mini_, 1, wxGROW | wxTOP | wxLEFT, 5);

    //Infobar
    info_panel_ = new wxStaticText(itemPanel12, wxID_STATIC, ""
        , wxDefaultPosition, wxSize(200, -1), wxTE_MULTILINE | wxTE_WORDWRAP);
    itemBoxSizer4->Add(info_panel_, g_flagsExpandBorder1);
    //Show tips when no any transaction selected
    showTips();
}

//----------------------------------------------------------------------------
wxString mmCheckingPanel::GetPanelTitle(const Model_Account::Data& account) const
{
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

    if (account)
        header_text_->SetLabelText(GetPanelTitle(*account));

    bool show_displayed_balance_ = (transFilterActive_ || currentView_ != MENU_VIEW_ALLTRANSACTIONS);
    wxStaticText* header = static_cast<wxStaticText*>(FindWindow(ID_PANEL_CHECKING_STATIC_BALHEADER1));
    header->SetLabelText(Model_Account::toCurrency(account_balance_, account));
    header = static_cast<wxStaticText*>(FindWindow(ID_PANEL_CHECKING_STATIC_BALHEADER2));
    header->SetLabelText(Model_Account::toCurrency(reconciled_balance_, account));
    header = static_cast<wxStaticText*>(FindWindow(ID_PANEL_CHECKING_STATIC_BALHEADER3));
    header->SetLabelText(Model_Account::toCurrency(account_balance_ - reconciled_balance_, account));
    header = static_cast<wxStaticText*>(FindWindow(ID_PANEL_CHECKING_STATIC_BALHEADER4));
    header->SetLabelText(show_displayed_balance_
        ? _("Displayed Bal: ") : "");
    header = static_cast<wxStaticText*>(FindWindow(ID_PANEL_CHECKING_STATIC_BALHEADER5));
    header->SetLabelText(show_displayed_balance_
        ? Model_Account::toCurrency(filteredBalance_, account) : "");
    this->Layout();
}

//----------------------------------------------------------------------------
void mmCheckingPanel::enableEditDeleteButtons(bool en)
{
    if (m_listCtrlAccount->GetSelectedItemCount()>1)
    {
        btnEdit_->Enable(false);
        btnDelete_->Enable(true);
        btnDuplicate_->Enable(false);
        btnAttachment_->Enable(false);
    }
    else
    {
        btnEdit_->Enable(en);
        btnDelete_->Enable(en);
        btnDuplicate_->Enable(en);
        btnAttachment_->Enable(en);
    }
}
//----------------------------------------------------------------------------

void mmCheckingPanel::updateExtraTransactionData(int selIndex)
{
    if (selIndex > -1)
    {
        enableEditDeleteButtons(true);
        const Model_Checking::Data& tran = this->m_trans.at(selIndex);
        Model_Checking::Full_Data full_tran(tran);
        info_panel_->SetLabelText(tran.NOTES);
        wxString miniStr = full_tran.info();

        //Show only first line but full string set as tooltip
        if (miniStr.Find("\n") > 1 && !miniStr.IsEmpty())
        {
            info_panel_mini_->SetLabelText(miniStr.substr(0, miniStr.Find("\n")) + " ...");
            info_panel_mini_->SetToolTip(miniStr);
        }
        else
        {
            info_panel_mini_->SetLabelText(miniStr);
            info_panel_mini_->SetToolTip(miniStr);
        }

    }
    else
    {
        info_panel_mini_->SetLabelText("");
        enableEditDeleteButtons(false);
        showTips();
    }
}
//----------------------------------------------------------------------------
void mmCheckingPanel::showTips()
{
    info_panel_->SetLabelText(wxGetTranslation(TIPS[rand() % (sizeof(TIPS) / sizeof(wxString))]));
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
    currentView_ = menu_labels().Index(Model_Infotable::instance().GetStringInfo(wxString::Format("CHECK_FILTER_ID_%d", m_AccountID), def_view));
    if (currentView_ < 0 || currentView_ >= (int) menu_labels().size())
        currentView_ = menu_labels().Index(VIEW_TRANS_ALL_STR);

    SetTransactionFilterState(currentView_ == MENU_VIEW_ALLTRANSACTIONS);
    stxtMainFilter_->SetLabelText(wxGetTranslation(menu_labels()[currentView_]));
}
//----------------------------------------------------------------------------
void mmCheckingPanel::initFilterSettings()
{
    mmDateRange* date_range = 0;

    if (transFilterActive_)
        date_range = new mmAllTime;
    else if (currentView_ == MENU_VIEW_ALLTRANSACTIONS)
        date_range = new mmAllTime;
    else if (currentView_ == MENU_VIEW_TODAY)
        date_range = new mmToday;
    else if (currentView_ == MENU_VIEW_CURRENTMONTH)
        date_range = new mmCurrentMonth;
    else if (currentView_ == MENU_VIEW_LAST30)
        date_range = new mmLast30Days;
    else if (currentView_ == MENU_VIEW_LAST90)
        date_range = new mmLast90Days;
    else if (currentView_ == MENU_VIEW_LASTMONTH)
        date_range = new mmLastMonth;
    else if (currentView_ == MENU_VIEW_LAST3MONTHS)
        date_range = new mmLast3Months;
    else if (currentView_ == MENU_VIEW_LAST12MONTHS)
        date_range = new mmLast12Months;
    else if (currentView_ == MENU_VIEW_CURRENTYEAR)
        date_range = new mmCurrentYear;
    else
        wxASSERT(false);

    if (date_range)
    {
        quickFilterBeginDate_ = date_range->start_date();
        quickFilterEndDate_ = date_range->end_date();
        delete date_range;
    }
}

void mmCheckingPanel::OnFilterResetToViewAll(wxMouseEvent& event) {

    if (currentView_ == MENU_VIEW_ALLTRANSACTIONS)
    {
        event.Skip();
        return;
    }

    currentView_ = MENU_VIEW_ALLTRANSACTIONS;
    stxtMainFilter_->SetLabelText(wxGetTranslation(menu_labels()[currentView_]));
    SetTransactionFilterState(true);
    initFilterSettings();

    m_listCtrlAccount->m_selectedIndex = -1;
    RefreshList();
    this->Layout();
}

void mmCheckingPanel::OnViewPopupSelected(wxCommandEvent& event)
{
    currentView_ = event.GetId() - wxID_HIGHEST;

    if (currentView_ == MENU_VIEW_ALLTRANSACTIONS)
        transFilterActive_ = false;

    stxtMainFilter_->SetLabelText(wxGetTranslation(menu_labels()[currentView_]));
    SetTransactionFilterState(currentView_ == MENU_VIEW_ALLTRANSACTIONS);

    m_listCtrlAccount->m_selectedIndex = -1;

    Model_Infotable::instance().Set(wxString::Format("CHECK_FILTER_ID_%ld", (long) m_AccountID)
        , menu_labels()[currentView_]);
    initFilterSettings();
    RefreshList(m_listCtrlAccount->m_selectedID);
    stxtMainFilter_->Layout();
}

void mmCheckingPanel::DeleteViewedTransactions()
{
    Model_Checking::instance().Savepoint();
    for (const auto& tran: this->m_trans)
    {
        // remove also removes any split transactions
        Model_Checking::instance().remove(tran.TRANSID);
        mmAttachmentManage::DeleteAllAttachments(Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION), tran.TRANSID);
        if (m_listCtrlAccount->m_selectedForCopy == tran.TRANSID) m_listCtrlAccount->m_selectedForCopy = -1;
    }
    Model_Checking::instance().ReleaseSavepoint();
}

void mmCheckingPanel::DeleteFlaggedTransactions(const wxString& status)
{
    Model_Checking::instance().Savepoint();
    for (const auto& tran: this->m_trans)
    {
        if (tran.STATUS == status)
        {
            // remove also removes any split transactions
            Model_Checking::instance().remove(tran.TRANSID);
            mmAttachmentManage::DeleteAllAttachments(Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION), tran.TRANSID);
            if (m_listCtrlAccount->m_selectedForCopy == tran.TRANSID) m_listCtrlAccount->m_selectedForCopy = -1;
        }
    }
    Model_Checking::instance().ReleaseSavepoint();
}

void mmCheckingPanel::OnFilterTransactions(wxMouseEvent& event)
{
    int e = event.GetEventType();

    if (e == wxEVT_LEFT_DOWN) {
        transFilterDlg_->setAccountToolTip(_("Select account used in transfer transactions"));
        transFilterActive_ = (transFilterDlg_->ShowModal() == wxID_OK && transFilterDlg_->somethingSelected());
    } else {
        if (transFilterActive_ == false) return;
        transFilterActive_ = false;
    }
    
    bitmapTransFilter_->SetBitmap(transFilterActive_ ? mmBitmap(png::RIGHTARROW_ACTIVE) : mmBitmap(png::RIGHTARROW));
    SetTransactionFilterState(true);

    RefreshList();
}


const wxString mmCheckingPanel::getItem(long item, long column)
{
    if (item < 0 || item >= (int)m_trans.size()) return "";

    const Model_Checking::Full_Data& tran = this->m_trans.at(item);
    switch (column)
    {
    case TransactionListCtrl::COL_ID:
        return wxString::Format("%i", tran.TRANSID).Trim();
    case TransactionListCtrl::COL_DATE:
        return mmGetDateForDisplay(Model_Checking::TRANSDATE(tran));
    case TransactionListCtrl::COL_NUMBER:
        return tran.TRANSACTIONNUMBER;
    case TransactionListCtrl::COL_CATEGORY:
        return tran.CATEGNAME;
    case TransactionListCtrl::COL_PAYEE_STR:
        return tran.PAYEENAME;
    case TransactionListCtrl::COL_STATUS:
        return tran.STATUS;
    case TransactionListCtrl::COL_WITHDRAWAL:
        return tran.AMOUNT <= 0 ? Model_Currency::toString(fabs(tran.AMOUNT), this->m_currency) : "";
    case TransactionListCtrl::COL_DEPOSIT:
        return tran.AMOUNT > 0 ? Model_Currency::toString(tran.AMOUNT, this->m_currency) : "";
    case TransactionListCtrl::COL_BALANCE:
        return Model_Currency::toString(tran.BALANCE, this->m_currency);
    case TransactionListCtrl::COL_NOTES:
        return tran.NOTES;
    default:
        return wxEmptyString;
    }
}

void mmCheckingPanel::OnSearchTxtEntered(wxCommandEvent& event)
{
    const wxString search_string = event.GetString().Lower();
    if (search_string.IsEmpty()) return;

    long last = m_listCtrlAccount->GetItemCount() - 1;
    long selectedItem = m_listCtrlAccount->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (selectedItem <= 0 || selectedItem >= last) //nothing selected
        selectedItem = m_listCtrlAccount->g_asc ? last : 0;

    while (selectedItem >= 0 && selectedItem <= last)
    {
        m_listCtrlAccount->g_asc ?  selectedItem-- : selectedItem++;
        const wxString t = getItem(selectedItem, m_listCtrlAccount->COL_NOTES).Lower();
        if (t.Matches(search_string + "*"))
        {
            //First of all any items should be unselected
            long cursel = m_listCtrlAccount->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            if (cursel != wxNOT_FOUND)
                m_listCtrlAccount->SetItemState(cursel, 0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);

            //Then finded item will be selected
            m_listCtrlAccount->SetItemState(selectedItem, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
            m_listCtrlAccount->EnsureVisible(selectedItem);
            break;
        }
    }
}

void mmCheckingPanel::DisplaySplitCategories(int transID)
{
    const Model_Checking::Data* tran = Model_Checking::instance().get(transID);
    int transType = Model_Checking::type(tran->TRANSCODE);

    Model_Checking::Data *transaction = Model_Checking::instance().get(transID);
    auto splits = Model_Checking::splittransaction(transaction);
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

void mmCheckingPanel::RefreshList(int transID)
{
    m_listCtrlAccount->refreshVisualList(transID);
}

void mmCheckingPanel::SetTransactionFilterState(bool active)
{
    bitmapMainFilter_->Enable(!transFilterActive_);
    stxtMainFilter_->Enable(!transFilterActive_);
    //bitmapTransFilter_->Enable(active || transFilterActive_);
    //statTextTransFilter_->Enable(active || transFilterActive_);
}

void mmCheckingPanel::SetSelectedTransaction(int transID)
{
    RefreshList(transID);
    m_listCtrlAccount->SetFocus();
}

// Refresh account screen with new details
void mmCheckingPanel::DisplayAccountDetails(int accountID)
{
    m_AccountID = accountID;
    m_account = Model_Account::instance().get(accountID);
    if (m_account)
        m_currency = Model_Account::currency(m_account);

    initViewTransactionsHeader();
    initFilterSettings();
    if (m_listCtrlAccount->m_selectedIndex > -1)
        m_listCtrlAccount->SetItemState(m_listCtrlAccount->m_selectedIndex, 0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
    m_listCtrlAccount->m_selectedIndex = -1;
    RefreshList();
    showTips();
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
//----------------------------------------------------------------------------

TransactionListCtrl::TransactionListCtrl(
    mmCheckingPanel *cp,
    wxWindow *parent,
    const wxWindowID id
) :
    mmListCtrl(parent, id),
    m_cp(cp),
    m_selectedIndex(-1),
    m_selectedForCopy(-1),
    m_attr1(*wxBLACK, mmColors::listAlternativeColor0, wxNullFont),
    m_attr2(*wxBLACK, mmColors::listAlternativeColor1, wxNullFont),
    m_attr3(mmColors::listFutureDateColor, mmColors::listAlternativeColor0, wxNullFont),
    m_attr4(mmColors::listFutureDateColor, mmColors::listAlternativeColor1, wxNullFont),
    m_attr11(*wxBLACK, mmColors::userDefColor1, wxNullFont),
    m_attr12(*wxBLACK, mmColors::userDefColor2, wxNullFont),
    m_attr13(*wxBLACK, mmColors::userDefColor3, wxNullFont),
    m_attr14(*wxBLACK, mmColors::userDefColor4, wxNullFont),
    m_attr15(*wxBLACK, mmColors::userDefColor5, wxNullFont),
    m_attr16(*wxYELLOW, mmColors::userDefColor6, wxNullFont),
    m_attr17(*wxYELLOW, mmColors::userDefColor7, wxNullFont),
    m_sortCol(COL_DEF_SORT),
    g_sortcol(COL_DEF_SORT),
    g_asc(true),
    m_selectedID(-1),
    topItemIndex_(-1)
{
    wxASSERT(m_cp);

    const wxAcceleratorEntry entries[] =
    {
        wxAcceleratorEntry(wxACCEL_CTRL, 'C', MENU_ON_COPY_TRANSACTION),
        wxAcceleratorEntry(wxACCEL_CTRL, 'V', MENU_ON_PASTE_TRANSACTION),
        wxAcceleratorEntry(wxACCEL_ALT,  'N', MENU_ON_NEW_TRANSACTION),
        wxAcceleratorEntry(wxACCEL_CTRL, 'D', MENU_ON_DUPLICATE_TRANSACTION),
        wxAcceleratorEntry(wxACCEL_CTRL, '0', MENU_ON_SET_UDC0),
        wxAcceleratorEntry(wxACCEL_CTRL, '1', MENU_ON_SET_UDC1),
        wxAcceleratorEntry(wxACCEL_CTRL, '2', MENU_ON_SET_UDC2),
        wxAcceleratorEntry(wxACCEL_CTRL, '3', MENU_ON_SET_UDC3),
        wxAcceleratorEntry(wxACCEL_CTRL, '4', MENU_ON_SET_UDC4),
        wxAcceleratorEntry(wxACCEL_CTRL, '5', MENU_ON_SET_UDC5),
        wxAcceleratorEntry(wxACCEL_CTRL, '6', MENU_ON_SET_UDC6),
        wxAcceleratorEntry(wxACCEL_CTRL, '7', MENU_ON_SET_UDC7)
    };

    wxAcceleratorTable tab(sizeof(entries)/sizeof(*entries), entries);
    SetAcceleratorTable(tab);

    m_columns.push_back(std::make_tuple(" ", 25, wxLIST_FORMAT_LEFT));
    m_columns.push_back(std::make_tuple(_("ID"), 0, wxLIST_FORMAT_LEFT));
    m_columns.push_back(std::make_tuple(_("Date"), 112, wxLIST_FORMAT_LEFT));
    m_columns.push_back(std::make_tuple(_("Number"), 70, wxLIST_FORMAT_LEFT));
    m_columns.push_back(std::make_tuple(_("Payee"), 150, wxLIST_FORMAT_LEFT));
    m_columns.push_back(std::make_tuple(_("Status"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT));
    m_columns.push_back(std::make_tuple(_("Category"), 150, wxLIST_FORMAT_LEFT));
    m_columns.push_back(std::make_tuple(_("Withdrawal"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT));
    m_columns.push_back(std::make_tuple(_("Deposit"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT));
    m_columns.push_back(std::make_tuple(_("Balance"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT));
    m_columns.push_back(std::make_tuple(_("Notes"), 250, wxLIST_FORMAT_LEFT));

    m_col_width = "CHECK_COL%d_WIDTH";

    m_default_sort_column = COL_DEF_SORT;
}

TransactionListCtrl::~TransactionListCtrl()
{
}

//----------------------------------------------------------------------------
void TransactionListCtrl::createColumns(mmListCtrl &lst)
{
    int i = 0;
    for (const auto& entry : m_columns)
    {
        const wxString& heading = std::get<0>(entry);
        int width = Model_Setting::instance().GetIntSetting(wxString::Format(m_col_width, i), std::get<1>(entry));
        int format = std::get<2>(entry);
        lst.InsertColumn(i, heading, format, width);
        i++;
    }
}

void TransactionListCtrl::OnListItemSelected(wxListEvent& event)
{
    m_selectedIndex = event.GetIndex();
    m_cp->updateExtraTransactionData(m_selectedIndex);
    topItemIndex_ = GetTopItem() + GetCountPerPage() - 1;

    if (GetSelectedItemCount()>1)
        m_cp->btnEdit_->Enable(false);

    m_selectedID = m_cp->m_trans[m_selectedIndex].TRANSID;
}
//----------------------------------------------------------------------------

void TransactionListCtrl::OnListLeftClick(wxMouseEvent& event)
{
    int Flags = wxLIST_HITTEST_ONITEM;
    long index = HitTest(wxPoint(event.m_x, event.m_y), Flags);
    if (index == -1)
    {
        m_selectedIndex = -1;
        m_cp->updateExtraTransactionData(m_selectedIndex);
    }
    event.Skip();
}

void TransactionListCtrl::OnMouseRightClick(wxMouseEvent& event)
{
    if (m_selectedIndex > -1)
        SetItemState(m_selectedIndex, 0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
    int Flags = wxLIST_HITTEST_ONITEM;
    m_selectedIndex = HitTest(wxPoint(event.m_x, event.m_y), Flags);

    if (m_selectedIndex >= 0)
    {
        SetItemState(m_selectedIndex, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        SetItemState(m_selectedIndex, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
    }
    m_cp->updateExtraTransactionData(m_selectedIndex);

    bool hide_menu_item = (m_selectedIndex < 0);
    bool type_transfer = false;
    bool have_category = false;
    if (m_selectedIndex > -1)
    {
        const Model_Checking::Full_Data& tran = m_cp->m_trans.at(m_selectedIndex);
        if (Model_Checking::type(tran.TRANSCODE) == Model_Checking::TRANSFER)
            type_transfer = true;
        if (!tran.has_split())
            have_category = true;
    }
    wxMenu menu;
    menu.Append(MENU_TREEPOPUP_NEW_WITHDRAWAL, _("&New Withdrawal"));
    menu.Append(MENU_TREEPOPUP_NEW_DEPOSIT, _("&New Deposit"));
    menu.Append(MENU_TREEPOPUP_NEW_TRANSFER, _("&New Transfer"));

    menu.AppendSeparator();

    menu.Append(MENU_TREEPOPUP_EDIT2, _("&Edit Transaction"));
    if (hide_menu_item) menu.Enable(MENU_TREEPOPUP_EDIT2, false);

    menu.Append(MENU_ON_COPY_TRANSACTION, _("&Copy Transaction"));
    if (hide_menu_item) menu.Enable(MENU_ON_COPY_TRANSACTION, false);

    menu.Append(MENU_ON_PASTE_TRANSACTION, _("&Paste Transaction"));
    if (m_selectedForCopy < 0) menu.Enable(MENU_ON_PASTE_TRANSACTION, false);

    menu.Append(MENU_ON_DUPLICATE_TRANSACTION, _("D&uplicate Transaction"));
    if (hide_menu_item) menu.Enable(MENU_ON_DUPLICATE_TRANSACTION, false);

    menu.Append(MENU_TREEPOPUP_MOVE2, _("&Move Transaction"));
    if (hide_menu_item || type_transfer || (Model_Account::checking_account_num() < 2))
        menu.Enable(MENU_TREEPOPUP_MOVE2, false);

    menu.AppendSeparator();

    menu.Append(MENU_TREEPOPUP_VIEW_SPLIT_CATEGORIES, _("&View Split Categories"));
    if (hide_menu_item || have_category)
        menu.Enable(MENU_TREEPOPUP_VIEW_SPLIT_CATEGORIES, false);

    menu.Append(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, _("&Organize Attachments"));
    if (hide_menu_item)
        menu.Enable(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, false);

    menu.Append(MENU_TREEPOPUP_CREATE_REOCCURANCE, _("Create Reoccuring T&ransaction"));
    if (hide_menu_item) menu.Enable(MENU_TREEPOPUP_CREATE_REOCCURANCE, false);

    menu.AppendSeparator();

    wxMenu* subGlobalOpMenuDelete = new wxMenu();
    subGlobalOpMenuDelete->Append(MENU_TREEPOPUP_DELETE2, _("&Delete Transaction"));
    if (hide_menu_item) subGlobalOpMenuDelete->Enable(MENU_TREEPOPUP_DELETE2, false);
    subGlobalOpMenuDelete->AppendSeparator();
    subGlobalOpMenuDelete->Append(MENU_TREEPOPUP_DELETE_VIEWED, _("Delete all transactions in current view"));
    subGlobalOpMenuDelete->Append(MENU_TREEPOPUP_DELETE_FLAGGED, _("Delete Viewed \"Follow Up\" Trans."));
    subGlobalOpMenuDelete->Append(MENU_TREEPOPUP_DELETE_UNRECONCILED, _("Delete Viewed \"Unreconciled\" Trans."));
    menu.Append(MENU_TREEPOPUP_DELETE2, _("&Delete "), subGlobalOpMenuDelete);

    menu.AppendSeparator();

    wxMenu* subGlobalOpMenuMark = new wxMenu();
    subGlobalOpMenuMark->Append(MENU_TREEPOPUP_MARKRECONCILED, _("Mark As &Reconciled"));
    if (hide_menu_item) subGlobalOpMenuMark->Enable(MENU_TREEPOPUP_MARKRECONCILED, false);
    subGlobalOpMenuMark->Append(MENU_TREEPOPUP_MARKUNRECONCILED, _("Mark As &Unreconciled"));
    if (hide_menu_item) subGlobalOpMenuMark->Enable(MENU_TREEPOPUP_MARKUNRECONCILED, false);
    subGlobalOpMenuMark->Append(MENU_TREEPOPUP_MARKVOID, _("Mark As &Void"));
    if (hide_menu_item) subGlobalOpMenuMark->Enable(MENU_TREEPOPUP_MARKVOID, false);
    subGlobalOpMenuMark->Append(MENU_TREEPOPUP_MARK_ADD_FLAG_FOLLOWUP, _("Mark For &Followup"));
    if (hide_menu_item) subGlobalOpMenuMark->Enable(MENU_TREEPOPUP_MARK_ADD_FLAG_FOLLOWUP, false);
    subGlobalOpMenuMark->Append(MENU_TREEPOPUP_MARKDUPLICATE, _("Mark As &Duplicate"));
    if (hide_menu_item) subGlobalOpMenuMark->Enable(MENU_TREEPOPUP_MARKDUPLICATE, false);
    menu.Append(wxID_ANY, _("Mark"), subGlobalOpMenuMark);

    wxMenu* subGlobalOpMenu = new wxMenu();
    subGlobalOpMenu->Append(MENU_TREEPOPUP_MARKRECONCILED_ALL, _("as Reconciled"));
    subGlobalOpMenu->Append(MENU_TREEPOPUP_MARKUNRECONCILED_ALL, _("as Unreconciled"));
    subGlobalOpMenu->Append(MENU_TREEPOPUP_MARKVOID_ALL, _("as Void"));
    subGlobalOpMenu->Append(MENU_TREEPOPUP_MARK_ADD_FLAG_FOLLOWUP_ALL, _("as needing Followup"));
    subGlobalOpMenu->Append(MENU_TREEPOPUP_MARKDUPLICATE_ALL, _("as Duplicate"));
    menu.Append(MENU_SUBMENU_MARK_ALL, _("Mark all being viewed"), subGlobalOpMenu);

    PopupMenu(&menu, event.GetPosition());
    this->SetFocus();
}
//----------------------------------------------------------------------------

void TransactionListCtrl::OnMarkTransaction(wxCommandEvent& event)
{
    int evt = event.GetId();
    wxString org_status = "";
    wxString status = "";
    if (evt == MENU_TREEPOPUP_MARKRECONCILED)              status = "R";
    else if (evt == MENU_TREEPOPUP_MARKUNRECONCILED)       status = "";
    else if (evt == MENU_TREEPOPUP_MARKVOID)               status = "V";
    else if (evt == MENU_TREEPOPUP_MARK_ADD_FLAG_FOLLOWUP) status = "F";
    else if (evt == MENU_TREEPOPUP_MARKDUPLICATE)          status = "D";
    else wxASSERT(false);

    Model_Checking::Data *trx = Model_Checking::instance().get(m_cp->m_trans[m_selectedIndex].TRANSID);
    if (trx)
    {
        org_status = trx->STATUS;
        m_cp->m_trans[m_selectedIndex].STATUS = status;
        trx->STATUS = status;
        Model_Checking::instance().save(trx);
    }

    bool bRefreshRequired = (status == "V") || (org_status == "V");

    if ((m_cp->transFilterActive_ && m_cp->transFilterDlg_->getStatusCheckBox()) || bRefreshRequired)
    {
        refreshVisualList(m_cp->m_trans[m_selectedIndex].TRANSID);
    }
    else
    {
        RefreshItems(m_selectedIndex, m_selectedIndex);
        m_cp->updateTable();
    }
}
//----------------------------------------------------------------------------

void TransactionListCtrl::OnMarkAllTransactions(wxCommandEvent& event)
{
    int evt =  event.GetId();
    wxString status = "";
    if (evt ==  MENU_TREEPOPUP_MARKRECONCILED_ALL)             status = "R";
    else if (evt == MENU_TREEPOPUP_MARKUNRECONCILED_ALL)       status = "";
    else if (evt == MENU_TREEPOPUP_MARKVOID_ALL)               status = "V";
    else if (evt == MENU_TREEPOPUP_MARK_ADD_FLAG_FOLLOWUP_ALL) status = "F";
    else if (evt == MENU_TREEPOPUP_MARKDUPLICATE_ALL)          status = "D";
    else if (evt == MENU_TREEPOPUP_DELETE_VIEWED)              status = "X";
    else if (evt == MENU_TREEPOPUP_DELETE_FLAGGED)             status = "M";
    else if (evt == MENU_TREEPOPUP_DELETE_UNRECONCILED)        status = "0";
    else  wxASSERT(false);

    if (status == "X")
    {
        wxMessageDialog msgDlg(this
            ,_("Do you really want to delete all the transactions shown?")
            , _("Confirm Transaction Deletion")
            , wxYES_NO | wxNO_DEFAULT | wxICON_ERROR);
        if (msgDlg.ShowModal() == wxID_YES)
        {
            m_cp->DeleteViewedTransactions();
        }
    }
    else if (status == "M" || status == "0")
    {
        const wxString statusStr = (status == "M" ? _("Follow Up") : _("Unreconciled"));
        const wxString shotStatusStr = (status == "M" ? "F" : "");
        wxMessageDialog msgDlg(this
            ,wxString::Format(_("Do you really want to delete all the \"%s\" transactions shown?"), statusStr)
            , _("Confirm Transaction Deletion")
            , wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION);
        if (msgDlg.ShowModal() == wxID_YES)
        {
            m_cp->DeleteFlaggedTransactions(shotStatusStr);
        }
    }
    else
    {
        for (auto& tran : m_cp->m_trans)
        {
            tran.NOTES.Replace(mmAttachmentManage::GetAttachmentNoteSign(), wxEmptyString, true);
            tran.STATUS = status;
        }
        Model_Checking::instance().save(m_cp->m_trans);
    }

    refreshVisualList();
}
//----------------------------------------------------------------------------

void TransactionListCtrl::OnColClick(wxListEvent& event)
{
    int ColumnNr;
    if (event.GetId() != MENU_HEADER_SORT)
        ColumnNr = event.GetColumn();
    else
        ColumnNr = m_ColumnHeaderNbr;

    if (0 > ColumnNr || ColumnNr >= COL_MAX || ColumnNr == COL_IMGSTATUS) return;

    /* Clear previous column image */
    setColumnImage(m_sortCol, -1);

	if (g_sortcol == ColumnNr && event.GetId() != MENU_HEADER_SORT) m_asc = !m_asc; // toggle sort order
    g_asc = m_asc;

    m_sortCol = toEColumn(ColumnNr);
    g_sortcol = m_sortCol;

    setColumnImage(m_sortCol, m_asc ? ICON_ASC : ICON_DESC);
    Model_Setting::instance().Set("CHECK_ASC", (g_asc ? 1 : 0));
    Model_Setting::instance().Set("CHECK_SORT_COL", g_sortcol);

    refreshVisualList(m_selectedID, false);

}
//----------------------------------------------------------------------------

void TransactionListCtrl::setColumnImage(EColumn col, int image)
{
    wxListItem item;
    item.SetMask(wxLIST_MASK_IMAGE);
    item.SetImage(image);

    SetColumn(col, item);
}
//----------------------------------------------------------------------------

wxString TransactionListCtrl::OnGetItemText(long item, long column) const
{
    return m_cp->getItem(item, column);
}
//----------------------------------------------------------------------------

/*
    Returns the icon to be shown for each transaction for the required column
*/
int TransactionListCtrl::OnGetItemColumnImage(long item, long column) const
{
    if (m_cp->m_trans.empty()) return ICON_NONE;

    int res = -1;
    if (column == COL_IMGSTATUS)
    {
        res = ICON_NONE;
        wxString status = m_cp->getItem(item, COL_STATUS);
        if ( status == "F")
            res = ICON_FOLLOWUP;
        else if (status == "R")
            res = ICON_RECONCILED;
        else if (status == "V")
            res = ICON_VOID;
        else if (status == "D")
            res = ICON_DUPLICATE;
    }

    return res;
}
//----------------------------------------------------------------------------

/*
    Failed wxASSERT will hang application if active modal dialog presents on screen.
    Assertion's message box will be hidden until you press tab to activate one.
*/
wxListItemAttr* TransactionListCtrl::OnGetItemAttr(long item) const
{
    if (item < 0 || item >= (int)m_cp->m_trans.size()) return 0;

    const Model_Checking::Full_Data& tran = m_cp->m_trans[item];
    bool in_the_future = Model_Checking::TRANSDATE(&tran) > wxDateTime::Today();

    // apply alternating background pattern
    int user_colour_id = tran.FOLLOWUPID;
    if (user_colour_id < 0 ) user_colour_id = 0;
    else if (user_colour_id > 7) user_colour_id = 0;

    if (user_colour_id != 0)
    {
        if      (user_colour_id == 1) return (wxListItemAttr*)&m_attr11;
        else if (user_colour_id == 2) return (wxListItemAttr*)&m_attr12;
        else if (user_colour_id == 3) return (wxListItemAttr*)&m_attr13;
        else if (user_colour_id == 4) return (wxListItemAttr*)&m_attr14;
        else if (user_colour_id == 5) return (wxListItemAttr*)&m_attr15;
        else if (user_colour_id == 6) return (wxListItemAttr*)&m_attr16;
        else if (user_colour_id == 7) return (wxListItemAttr*)&m_attr17;
    }
    else if (in_the_future && item % 2) return (wxListItemAttr*)&m_attr3;
    else if (in_the_future)             return (wxListItemAttr*)&m_attr4;
    else if (item % 2)                  return (wxListItemAttr*)&m_attr1;

    return (wxListItemAttr*)&m_attr2;
}
//----------------------------------------------------------------------------
// If any of these keys are encountered, the search for the event handler
// should continue as these keys may be processed by the operating system.
void TransactionListCtrl::OnChar(wxKeyEvent& event)
{

    if (wxGetKeyState(WXK_ALT) ||
        wxGetKeyState(WXK_COMMAND) ||
        wxGetKeyState(WXK_UP) ||
        wxGetKeyState(WXK_DOWN) ||
        wxGetKeyState(WXK_LEFT) ||
        wxGetKeyState(WXK_RIGHT) ||
        wxGetKeyState(WXK_HOME) ||
        wxGetKeyState(WXK_END) ||
        wxGetKeyState(WXK_PAGEUP) ||
        wxGetKeyState(WXK_PAGEDOWN) ||
        wxGetKeyState(WXK_NUMPAD_UP) ||
        wxGetKeyState(WXK_NUMPAD_DOWN) ||
        wxGetKeyState(WXK_NUMPAD_LEFT) ||
        wxGetKeyState(WXK_NUMPAD_RIGHT) ||
        wxGetKeyState(WXK_NUMPAD_PAGEDOWN) ||
        wxGetKeyState(WXK_NUMPAD_PAGEUP) ||
        wxGetKeyState(WXK_NUMPAD_HOME) ||
        wxGetKeyState(WXK_NUMPAD_END) ||
        wxGetKeyState(WXK_DELETE) ||
        wxGetKeyState(WXK_NUMPAD_DELETE) ||
        wxGetKeyState(WXK_TAB)||
        wxGetKeyState(WXK_RETURN)||
        wxGetKeyState(WXK_NUMPAD_ENTER)||
        wxGetKeyState(WXK_SPACE)||
        wxGetKeyState(WXK_NUMPAD_SPACE)
        )
    {
        event.Skip();
    }
}
//----------------------------------------------------------------------------

void TransactionListCtrl::OnCopy(wxCommandEvent& WXUNUSED(event))
{
    if (m_selectedIndex < 0) return;

    m_selectedForCopy = m_cp->m_trans[m_selectedIndex].TRANSID;
}

void TransactionListCtrl::OnDuplicateTransaction(wxCommandEvent& event)
{
    if (m_selectedIndex < 0) return;

    int transaction_id = m_cp->m_trans[m_selectedIndex].TRANSID;
    mmTransDialog dlg(this, m_cp->m_AccountID, transaction_id, true);
    if (dlg.ShowModal() == wxID_OK)
    {
        m_selectedIndex = dlg.getTransactionID();
        refreshVisualList(m_selectedIndex);
    }
    topItemIndex_ = GetTopItem() + GetCountPerPage() - 1;
}

void TransactionListCtrl::OnPaste(wxCommandEvent& WXUNUSED(event))
{
    if (m_selectedForCopy < 0) return;
    Model_Checking::Data* tran = Model_Checking::instance().get(m_selectedForCopy);
    if (tran)
    {
        int transactionID = OnPaste(tran);
        refreshVisualList(transactionID);
    }
}
int TransactionListCtrl::OnPaste(Model_Checking::Data* tran)
{
    bool useOriginalDate = Model_Setting::instance().GetBoolSetting(INIDB_USE_ORG_DATE_COPYPASTE, false);

    Model_Checking::Data* copy = Model_Checking::instance().clone(tran); //TODO: this function can't clone split transactions
    if (!useOriginalDate) copy->TRANSDATE = wxDateTime::Now().FormatISODate();
    if (Model_Checking::type(copy->TRANSCODE) != Model_Checking::TRANSFER) copy->ACCOUNTID = m_cp->m_AccountID;
    int transactionID = Model_Checking::instance().save(copy);

    Model_Splittransaction::Cache copy_split;
    for (const auto& split_item : Model_Checking::splittransaction(tran))
    {
        Model_Splittransaction::Data *copy_split_item = Model_Splittransaction::instance().clone(&split_item);
        copy_split_item->TRANSID = transactionID;
        copy_split.push_back(copy_split_item);
    }
    Model_Splittransaction::instance().save(copy_split);

    return transactionID;
}

void TransactionListCtrl::OnOpenAttachment(wxCommandEvent& event)
{
    if (m_selectedIndex < 0) return;
    int transaction_id = m_cp->m_trans[m_selectedIndex].TRANSID;
    wxString RefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);

	mmAttachmentManage::OpenAttachmentFromPanelIcon(this, RefType, transaction_id);
	refreshVisualList(transaction_id);
}

//----------------------------------------------------------------------------

void TransactionListCtrl::OnListKeyDown(wxListEvent& event)
{
    if (wxGetKeyState(WXK_COMMAND) || wxGetKeyState(WXK_ALT) || wxGetKeyState(WXK_CONTROL)
        || m_selectedIndex == -1) {
        event.Skip();
        return;
    }

    topItemIndex_ = GetTopItem() + GetCountPerPage() -1;

    //Read status of the selected transaction
    wxString status = m_cp->m_trans[m_selectedIndex].STATUS;

    if (wxGetKeyState(wxKeyCode('R')) && status != "R") {
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_TREEPOPUP_MARKRECONCILED);
        OnMarkTransaction(evt);
    }
    else if (wxGetKeyState(wxKeyCode('U')) && status != "") {
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_TREEPOPUP_MARKUNRECONCILED);
        OnMarkTransaction(evt);
    }
    else if (wxGetKeyState(wxKeyCode('F')) && status != "F") {
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_TREEPOPUP_MARK_ADD_FLAG_FOLLOWUP);
        OnMarkTransaction(evt);
    }
    else if (wxGetKeyState(wxKeyCode('D')) && status != "D") {
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_TREEPOPUP_MARKDUPLICATE);
        OnMarkTransaction(evt);
    }
    else if (wxGetKeyState(wxKeyCode('V')) && status != "V") {
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_TREEPOPUP_MARKVOID);
        OnMarkTransaction(evt);
    }
    else if ((wxGetKeyState(WXK_DELETE) || wxGetKeyState(WXK_NUMPAD_DELETE)) && status != "V")
    {
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_TREEPOPUP_MARKVOID);
        OnMarkTransaction(evt);
    }
    else if (wxGetKeyState(WXK_DELETE) || wxGetKeyState(WXK_NUMPAD_DELETE))
    {
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_TREEPOPUP_DELETE2);
        OnDeleteTransaction(evt);
    }
    else {
        event.Skip();
        return;
    }
}
//----------------------------------------------------------------------------

void TransactionListCtrl::OnDeleteTransaction(wxCommandEvent& /*event*/)
{
    //check if a transaction is selected
    if (GetSelectedItemCount() < 1) return;

    topItemIndex_ = GetTopItem() + GetCountPerPage() -1;

    //ask if they really want to delete
    wxMessageDialog msgDlg(this
        , _("Do you really want to delete the selected transaction?")
        , _("Confirm Transaction Deletion")
        , wxYES_NO | wxYES_DEFAULT | wxICON_ERROR);

    if (msgDlg.ShowModal() == wxID_YES)
    {
        long x = 0;
        for (const auto& i : m_cp->m_trans)
        {
            long transID = i.TRANSID;
            if (GetItemState(x, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED)
            {
                // remove also removes any split transactions
                Model_Checking::instance().remove(transID);
                mmAttachmentManage::DeleteAllAttachments(Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION), transID);
                if (x <= topItemIndex_) topItemIndex_--;
                if (!m_cp->m_trans.empty() && m_selectedIndex > 0) m_selectedIndex--;
                if (m_selectedForCopy == transID) m_selectedForCopy = -1;
            }
            x++;
        }

        refreshVisualList();
    }
}
//----------------------------------------------------------------------------

void TransactionListCtrl::OnEditTransaction(wxCommandEvent& /*event*/)
{
    if (m_selectedIndex < 0) return;

    int transaction_id = m_cp->m_trans[m_selectedIndex].TRANSID;
    mmTransDialog dlg(this, m_cp->m_AccountID, transaction_id);
    if (dlg.ShowModal() == wxID_OK)
    {
        refreshVisualList(transaction_id);
    }
    topItemIndex_ = GetTopItem() + GetCountPerPage() - 1;
}

void TransactionListCtrl::OnNewTransaction(wxCommandEvent& event)
{
    int type = event.GetId() == MENU_TREEPOPUP_NEW_DEPOSIT ? Model_Checking::DEPOSIT : Model_Checking::WITHDRAWAL;
    mmTransDialog dlg(this, m_cp->m_AccountID, 0, false, type);
    if (dlg.ShowModal() == wxID_OK)
    {
        m_cp->mmPlayTransactionSound();
        refreshVisualList(dlg.getTransactionID());
    }
}

void TransactionListCtrl::OnNewTransferTransaction(wxCommandEvent& /*event*/)
{
    mmTransDialog dlg(this, m_cp->m_AccountID, 0, false, Model_Checking::TRANSFER);
    if (dlg.ShowModal() == wxID_OK)
    {
        m_cp->mmPlayTransactionSound();
        refreshVisualList(dlg.getTransactionID());
    }
}
//----------------------------------------------------------------------------

void TransactionListCtrl::OnSetUserColour(wxCommandEvent& event)
{
    int user_colour_id = event.GetId();
    user_colour_id -= MENU_ON_SET_UDC0;
    wxLogDebug("id: %i", user_colour_id);

    Model_Checking::Data* transaction = Model_Checking::instance().get(m_selectedID);
    if (transaction)
    {
        transaction->FOLLOWUPID = user_colour_id;
        Model_Checking::instance().save(transaction);
        m_cp->m_trans[m_selectedIndex].FOLLOWUPID = user_colour_id;
        RefreshItems(m_selectedIndex, m_selectedIndex);
    }
}
//----------------------------------------------------------------------------

void TransactionListCtrl::refreshVisualList(int trans_id, bool filter)
{
    this->SetEvtHandlerEnabled(false);
    Hide();

    // decide whether top or down icon needs to be shown
    setColumnImage(g_sortcol, g_asc ? ICON_ASC : ICON_DESC);
    if (filter) m_cp->filterTable();
    SetItemCount(m_cp->m_trans.size());
    Show();
    m_cp->sortTable();
    m_cp->markSelectedTransaction(trans_id);

    if (topItemIndex_ >= (long)m_cp->m_trans.size())
        topItemIndex_ = g_asc ? (long)m_cp->m_trans.size() - 1 : 0;
    if (m_selectedIndex > (long)m_cp->m_trans.size() - 1) m_selectedIndex = -1;
    if (topItemIndex_ < m_selectedIndex) topItemIndex_ = m_selectedIndex;


    if (m_selectedIndex >= 0 && !m_cp->m_trans.empty())
    {
        SetItemState(m_selectedIndex, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        SetItemState(m_selectedIndex, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
        if (topItemIndex_ < 0 || (topItemIndex_ - m_selectedIndex) > GetCountPerPage())
            topItemIndex_ = m_selectedIndex;
        EnsureVisible(topItemIndex_);
    }

    m_cp->setAccountSummary();
    m_cp->updateExtraTransactionData(m_selectedIndex);
    this->SetEvtHandlerEnabled(true);
    Refresh();
    Update();
    m_cp->m_listCtrlAccount->SetFocus();
}

void TransactionListCtrl::OnMoveTransaction(wxCommandEvent& /*event*/)
{
    if (m_selectedIndex < 0) return;

    const Model_Account::Data* source_account = Model_Account::instance().get(m_cp->m_AccountID);
    wxString source_name = source_account->ACCOUNTNAME;
    wxString headerMsg = wxString::Format(_("Moving Transaction from %s to..."), source_name);

    mmSingleChoiceDialog scd(this
        , _("Select the destination Account ")
        , headerMsg
        , Model_Account::instance().all_checking_account_names());

    if (scd.ShowModal() == wxID_OK)
    {
        int dest_account_id = -1;
        wxString dest_account_name = scd.GetStringSelection();
        Model_Account::Data* dest_account = Model_Account::instance().get(dest_account_name);
        if (dest_account)
            dest_account_id = dest_account->ACCOUNTID;
        else
            return;

        Model_Checking::Full_Data& tran = m_cp->m_trans[m_selectedIndex];
        tran.ACCOUNTID = dest_account_id;
        Model_Checking::instance().save(&tran);
        refreshVisualList();
    }
}

//----------------------------------------------------------------------------
void TransactionListCtrl::OnViewSplitTransaction(wxCommandEvent& /*event*/)
{
    if (m_selectedIndex > -1) {
        const Model_Checking::Full_Data& tran = m_cp->m_trans.at(m_selectedIndex);
        if (tran.has_split())
            m_cp->DisplaySplitCategories(tran.TRANSID);
    }
}

//----------------------------------------------------------------------------
void TransactionListCtrl::OnOrganizeAttachments(wxCommandEvent& /*event*/)
{
    if (m_selectedIndex < 0) return;

    wxString RefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
    int RefId = m_cp->m_trans[m_selectedIndex].TRANSID;

    mmAttachmentDialog dlg(this, RefType, RefId);
    dlg.ShowModal();

	refreshVisualList(RefId);
}

//----------------------------------------------------------------------------
void TransactionListCtrl::OnCreateReoccurance(wxCommandEvent& /*event*/)
{
    if (m_selectedIndex < 0) return;

    mmBDDialog dlg(this, 0, false, false);
    dlg.SetDialogParameters(m_cp->m_trans[m_selectedIndex]);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxMessageBox(_("Reoccuring Transaction saved."));
    }
}

//----------------------------------------------------------------------------
void TransactionListCtrl::OnListItemActivated(wxListEvent& /*event*/)
{
    if (m_selectedIndex < 0) return;

    wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_TREEPOPUP_EDIT2);
    AddPendingEvent(evt);
}

