/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2014-2016 Nikolay Akimov
 Copyright (C) 2017 Stefano Giorgio [stef145g]

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
#include "assetdialog.h"
#include "attachmentdialog.h"
#include "billsdepositsdialog.h"
#include "constants.h"
#include "filtertransdialog.h"
#include "images_list.h"
#include "mmex.h"
#include "mmframe.h"
#include "mmSimpleDialogs.h"
#include "mmTips.h"
#include "Model_Usage.h"
#include "Model_Setting.h"
#include "Model_Infotable.h"
#include "paths.h"
#include "sharetransactiondialog.h"
#include "splittransactionsdialog.h"
#include "transdialog.h"
#include "transactionsupdatedialog.h"
#include "util.h"
#include "validators.h"

//----------------------------------------------------------------------------
#include <algorithm>
#include <wx/clipbrd.h>
#include <wx/srchctrl.h>
#include <wx/sound.h>
//----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(mmCheckingPanel, wxPanel)
    EVT_BUTTON(wxID_NEW,         mmCheckingPanel::OnNewTransaction)
    EVT_BUTTON(wxID_EDIT,        mmCheckingPanel::OnEditTransaction)
    EVT_BUTTON(wxID_REMOVE,      mmCheckingPanel::OnDeleteTransaction)
    EVT_BUTTON(wxID_DUPLICATE,    mmCheckingPanel::OnDuplicateTransaction)
    EVT_BUTTON(wxID_FILE, mmCheckingPanel::OnOpenAttachment)
    EVT_MENU_RANGE(wxID_HIGHEST + MENU_VIEW_ALLTRANSACTIONS
        , wxID_HIGHEST + MENU_VIEW_ALLTRANSACTIONS + menu_labels().size()
        , mmCheckingPanel::OnViewPopupSelected)
    EVT_SEARCHCTRL_SEARCH_BTN(wxID_FIND, mmCheckingPanel::OnSearchTxtEntered)
wxEND_EVENT_TABLE()
//----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(TransactionListCtrl, mmListCtrl)
    EVT_LIST_ITEM_SELECTED(wxID_ANY, TransactionListCtrl::OnListItemSelected)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY, TransactionListCtrl::OnListItemActivated)
    EVT_LIST_ITEM_FOCUSED(wxID_ANY, TransactionListCtrl::OnListItemFocused)
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
    : m_trans_filter_dlg(0)
    , m_AccountID(accountID)
    , m_filteredBalance(0.0)
    , m_listCtrlAccount()
    , m_account(Model_Account::instance().get(accountID))
    , m_currency(Model_Account::currency(m_account))
    , m_frame(frame)
{
    long style = wxTAB_TRAVERSAL | wxNO_BORDER;
    Create(parent, id, wxDefaultPosition, wxDefaultSize, style);
}
//----------------------------------------------------------------------------

/*
    We cannon use OnClose() event because this class deletes
    via DestroyChildren() of its parent.
*/
mmCheckingPanel::~mmCheckingPanel()
{
    if (m_trans_filter_dlg) delete m_trans_filter_dlg;
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
    wxDateTime start = wxDateTime::UNow();
    CreateControls();

    m_transFilterActive = false;
    m_trans_filter_dlg = new mmFilterTransactionsDialog(this, m_AccountID);
    SetTransactionFilterState(true);

    initViewTransactionsHeader();
    initFilterSettings();

    RefreshList();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    this->windowsFreezeThaw();

    Model_Usage::instance().pageview(this, (wxDateTime::UNow() - start).GetMilliseconds().ToLong());

    return true;
}

void mmCheckingPanel::sortTable()
{
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
    case TransactionListCtrl::COL_DATE:
        std::stable_sort(this->m_trans.begin(), this->m_trans.end(), SorterByTRANSDATE());
        break;
    case TransactionListCtrl::COL_UDFC01:
        std::stable_sort(this->m_trans.begin(), this->m_trans.end(), SorterByUDFC01);
        break;
    case TransactionListCtrl::COL_UDFC02:
        std::stable_sort(this->m_trans.begin(), this->m_trans.end(), SorterByUDFC02);
        break;
    case TransactionListCtrl::COL_UDFC03:
        std::stable_sort(this->m_trans.begin(), this->m_trans.end(), SorterByUDFC03);
        break;
    case TransactionListCtrl::COL_UDFC04:
        std::stable_sort(this->m_trans.begin(), this->m_trans.end(), SorterByUDFC04);
        break;
    case TransactionListCtrl::COL_UDFC05:
        std::stable_sort(this->m_trans.begin(), this->m_trans.end(), SorterByUDFC05);
        break;
    case TransactionListCtrl::COL_IMGSTATUS: break;
    case TransactionListCtrl::COL_MAX: break;

    }

    if (!m_listCtrlAccount->g_asc)
        std::reverse(this->m_trans.begin(), this->m_trans.end());
}

void mmCheckingPanel::filterTable()
{
    this->m_trans.clear();
    m_account_balance = m_account ? m_account->INITIALBAL : 0.0;
    m_reconciled_balance = m_account_balance;
    m_filteredBalance = 0.0;

    const auto custom_fields = Model_CustomFieldData::instance().get_all(Model_Attachment::TRANSACTION);
    const auto matrix = Model_CustomField::getMatrix(Model_Attachment::TRANSACTION);
    int udfc01_ref_id = matrix.at("UDFC01");
    int udfc02_ref_id = matrix.at("UDFC02");
    int udfc03_ref_id = matrix.at("UDFC03");
    int udfc04_ref_id = matrix.at("UDFC04");
    int udfc05_ref_id = matrix.at("UDFC05");

    const auto splits = Model_Splittransaction::instance().get_all();
    const auto attachments = Model_Attachment::instance().get_all(Model_Attachment::TRANSACTION);
    for (const auto& tran : Model_Account::transaction(this->m_account))
    {
        double transaction_amount = Model_Checking::amount(tran, m_AccountID);
        Model_Checking::Full_Data full_tran(m_AccountID, tran, splits);

        if (Model_Checking::status(full_tran.STATUSFD) != Model_Checking::VOID_)
            m_account_balance += transaction_amount;

        if (Model_Checking::status(full_tran.STATUSFD) == Model_Checking::RECONCILED)
            m_reconciled_balance += transaction_amount;

        if (m_transFilterActive)
        {
            if (!m_trans_filter_dlg->checkAll(full_tran, m_AccountID))
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

        full_tran.PAYEENAME = full_tran.real_payee_name(m_AccountID);
        full_tran.BALANCE = m_account_balance;
        full_tran.AMOUNT = transaction_amount;
        m_filteredBalance += transaction_amount;

        if (custom_fields.find(tran.TRANSID) != custom_fields.end()) {
            const auto& udfcs = custom_fields.at(tran.TRANSID);
            for (const auto& udfc : udfcs)
            {
                if (udfc.FIELDID == udfc01_ref_id) {
                    full_tran.UDFC01 = udfc.CONTENT;
                }
                else if (udfc.FIELDID == udfc02_ref_id) {
                    full_tran.UDFC02 = udfc.CONTENT;
                }
                else if (udfc.FIELDID == udfc03_ref_id) {
                    full_tran.UDFC03 = udfc.CONTENT;
                }
                else if (udfc.FIELDID == udfc04_ref_id) {
                    full_tran.UDFC04 = udfc.CONTENT;
                }
                else if (udfc.FIELDID == udfc05_ref_id) {
                    full_tran.UDFC05 = udfc.CONTENT;
                }
            }
        }

        if (attachments.count(full_tran.TRANSID)) {
            full_tran.NOTES.Prepend(mmAttachmentManage::GetAttachmentNoteSign());
        }

        this->m_trans.push_back(full_tran);
    }
}

void mmCheckingPanel::updateTable()
{
    m_account_balance = 0.0;
    m_reconciled_balance = 0.0;
    if (m_account)
    {
        m_account_balance = m_account->INITIALBAL;
        m_reconciled_balance = m_account_balance;
    }
    for (const auto& tran : m_trans /*Model_Account::transaction(m_account)*/)
    {
        double transaction_amount = Model_Checking::amount(tran, m_AccountID);
        if (Model_Checking::status(tran.STATUSFD) != Model_Checking::VOID_)
            m_account_balance += transaction_amount;
        m_reconciled_balance += Model_Checking::reconciled(tran, m_AccountID);
    }
    m_filteredBalance = 0.0;
    for (const auto & tran : m_trans)
    {
        m_filteredBalance += Model_Checking::amount(tran, m_AccountID);
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
        m_listCtrlAccount->m_selectedIndex = -1;
        m_listCtrlAccount->m_selectedID = -1;
        long i = 0;
        for (const auto & tran : m_trans)
        {
            //reset any selected items in the list
            if (m_listCtrlAccount->GetItemState(i, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED)
            {
                m_listCtrlAccount->SetItemState(i, 0, wxLIST_STATE_SELECTED);
            }
            // discover where the transaction has ended up in the list
            if (trans_id == tran.TRANSID)
            {
                m_listCtrlAccount->m_selectedIndex = i;
                // set the selected ID to this transaction.
                m_listCtrlAccount->m_selectedID = trans_id;
            }
            ++i;
        }
    }

    if (!m_trans.empty() && m_listCtrlAccount->m_selectedIndex < 0)
    {
        long i = m_listCtrlAccount->g_asc ? static_cast<long>(m_trans.size()) - 1 : 0;

        if (m_listCtrlAccount->g_sortcol == TransactionListCtrl::COL_DATE)
        {
            bool future = true;
            while (future && i >= 0)
            {
                if (m_trans[i].TRANSDATE > m_listCtrlAccount->m_today && i > 0)
                {
                    i = m_listCtrlAccount->g_asc ? i - 1 : i + 1;
                }
                else
                    future = false;
            }
        }
        m_listCtrlAccount->EnsureVisible(i);
        m_listCtrlAccount->m_selectedIndex = i;
        m_listCtrlAccount->m_selectedID = m_trans[i].TRANSID;
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
    itemBoxSizer9->Add(headerPanel, g_flagsBorder1V);

    wxBoxSizer* itemBoxSizerVHeader = new wxBoxSizer(wxVERTICAL);
    headerPanel->SetSizer(itemBoxSizerVHeader);

    wxGridSizer* itemBoxSizerVHeader2 = new wxGridSizer(0, 1, 5, 20);
    itemBoxSizerVHeader->Add(itemBoxSizerVHeader2);

    m_header_text = new wxStaticText( headerPanel, wxID_STATIC, "");
    m_header_text->SetFont(this->GetFont().Larger().Bold());
    itemBoxSizerVHeader2->Add(m_header_text, g_flagsBorder1H);

    wxBoxSizer* itemBoxSizerHHeader2 = new wxBoxSizer(wxHORIZONTAL);
    wxFlexGridSizer* itemFlexGridSizerHHeader2 = new wxFlexGridSizer(5, 1, 1);
    itemBoxSizerVHeader2->Add(itemBoxSizerHHeader2);
    itemBoxSizerHHeader2->Add(itemFlexGridSizerHHeader2);

    m_bitmapMainFilter = new wxStaticBitmap(headerPanel, wxID_PAGE_SETUP
        , mmBitmap(png::RIGHTARROW));
    itemFlexGridSizerHHeader2->Add(m_bitmapMainFilter, g_flagsBorder1H);
    m_bitmapMainFilter->Connect(wxID_ANY, wxEVT_RIGHT_DOWN
        , wxMouseEventHandler(mmCheckingPanel::OnFilterResetToViewAll), nullptr, this);
    m_bitmapMainFilter->Connect(wxID_ANY, wxEVT_LEFT_DOWN
        , wxMouseEventHandler(mmCheckingPanel::OnMouseLeftDown), nullptr, this);

    m_stxtMainFilter = new wxStaticText(headerPanel, wxID_ANY, "", wxDefaultPosition, wxSize(250, -1));
    itemFlexGridSizerHHeader2->Add(m_stxtMainFilter, g_flagsBorder1H);

    itemFlexGridSizerHHeader2->AddSpacer(20);

    m_bitmapTransFilter = new wxStaticBitmap(headerPanel, ID_PANEL_CHECKING_STATIC_BITMAP_FILTER
        , mmBitmap(png::RIGHTARROW));
    itemFlexGridSizerHHeader2->Add(m_bitmapTransFilter, g_flagsBorder1H);
    m_bitmapTransFilter->Connect(wxID_ANY, wxEVT_LEFT_DOWN
        , wxMouseEventHandler(mmCheckingPanel::OnFilterTransactions), nullptr, this);
    m_bitmapTransFilter->Connect(wxID_ANY, wxEVT_RIGHT_DOWN
        , wxMouseEventHandler(mmCheckingPanel::OnFilterTransactions), nullptr, this);

    m_statTextTransFilter = new wxStaticText(headerPanel, wxID_ANY
        , _("Transaction Filter"));
    itemFlexGridSizerHHeader2->Add(m_statTextTransFilter, g_flagsBorder1H);

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

    int x = Option::instance().IconSize();
    m_imageList.reset(new wxImageList(x, x));
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

//----------------------------------------------------------------------------
wxString mmCheckingPanel::GetPanelTitle(const Model_Account::Data& account) const
{
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

    if (account)
        m_header_text->SetLabelText(GetPanelTitle(*account));

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
    this->Layout();
}

//----------------------------------------------------------------------------
void mmCheckingPanel::enableEditDeleteButtons(bool en)
{
    if (m_listCtrlAccount->GetSelectedItemCount()>1)
    {
        m_btnDuplicate->Enable(false);
        m_btnAttachment->Enable(false);
    }
    else
    {
        m_btnEdit->Enable(en);
        m_btnDelete->Enable(en);
        m_btnDuplicate->Enable(en);
        m_btnAttachment->Enable(en);
    }
}
//----------------------------------------------------------------------------

void mmCheckingPanel::updateExtraTransactionData(int selIndex)
{
    if (selIndex > -1)
    {
        enableEditDeleteButtons(true);

        Model_Checking::Full_Data& full_tran = this->m_trans.at(selIndex);
        m_info_panel->SetLabelText(full_tran.NOTES);
        wxString miniStr = full_tran.info();

		if (Model_Checking::foreignTransaction(full_tran))
		{
			m_btnDuplicate->Enable(false);
		}

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
        m_info_panel_mini->SetLabelText("");
        enableEditDeleteButtons(false);
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
    if (m_currentView < 0 || m_currentView >= (int) menu_labels().size())
        m_currentView = menu_labels().Index(VIEW_TRANS_ALL_STR);

    SetTransactionFilterState(m_currentView == MENU_VIEW_ALLTRANSACTIONS);
    m_stxtMainFilter->SetLabelText(wxGetTranslation(menu_labels()[m_currentView]));
}
//----------------------------------------------------------------------------
void mmCheckingPanel::initFilterSettings()
{
    mmDateRange* date_range = NULL;
    bool show_future = !Option::instance().IgnoreFutureTransactions();
    const wxString& future_date_string = wxDateTime(31, wxDateTime::Dec, 9999).FormatISODate();
    m_begin_date = "";
    m_end_date = "";

    if (!m_transFilterActive)
    {
        switch (m_currentView) {
        case MENU_VIEW_TODAY:
            date_range = new mmToday;
            break;
        case MENU_VIEW_CURRENTMONTH:
            date_range = new mmCurrentMonth;
            if (show_future) m_end_date = future_date_string;
            break;
        case MENU_VIEW_LAST30:
            date_range = new mmLast30Days;
            if (show_future) m_end_date = future_date_string;
            break;
        case MENU_VIEW_LAST90:
            date_range = new mmLast90Days;
            if (show_future) m_end_date = future_date_string;
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
            if (show_future) m_end_date = future_date_string;
            break;
        case  MENU_VIEW_CURRENTFINANCIALYEAR:
            date_range = new mmCurrentFinancialYear(wxAtoi(Option::instance().FinancialYearStartDay())
                , wxAtoi(Option::instance().FinancialYearStartMonth()));
            if (show_future) m_end_date = future_date_string;
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
            }
        }
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
}

void mmCheckingPanel::OnFilterResetToViewAll(wxMouseEvent& event) {

    if (m_currentView == MENU_VIEW_ALLTRANSACTIONS)
    {
        event.Skip();
        return;
    }

    m_currentView = MENU_VIEW_ALLTRANSACTIONS;
    m_stxtMainFilter->SetLabelText(wxGetTranslation(menu_labels()[m_currentView]));
    SetTransactionFilterState(true);
    initFilterSettings();

    m_listCtrlAccount->m_selectedIndex = -1;
    RefreshList();
    this->Layout();
}

void mmCheckingPanel::OnViewPopupSelected(wxCommandEvent& event)
{
    m_currentView = event.GetId() - wxID_HIGHEST;

    if (m_currentView == MENU_VIEW_ALLTRANSACTIONS)
        m_transFilterActive = false;

    m_stxtMainFilter->SetLabelText(wxGetTranslation(menu_labels()[m_currentView]));
    SetTransactionFilterState(m_currentView == MENU_VIEW_ALLTRANSACTIONS);

    m_listCtrlAccount->m_selectedIndex = -1;

    Model_Infotable::instance().Set(wxString::Format("CHECK_FILTER_ID_%ld", (long) m_AccountID)
        , menu_labels()[m_currentView]);
    initFilterSettings();
    RefreshList(m_listCtrlAccount->m_selectedID);
    m_stxtMainFilter->Layout();
}

void mmCheckingPanel::DeleteViewedTransactions()
{
    Model_Checking::instance().Savepoint();
    for (const auto& tran: this->m_trans)
    {
        if (Model_Checking::foreignTransaction(tran))
        {
            Model_Translink::RemoveTranslinkEntry(tran.TRANSID);
        }

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

    if (e == wxEVT_LEFT_DOWN)
    {
        m_trans_filter_dlg->setAccountToolTip(_("Select account used in transfer transactions"));
        m_transFilterActive = (m_trans_filter_dlg->ShowModal() == wxID_OK 
            && m_trans_filter_dlg->SomethingSelected());
    }
    else 
    {
        if (m_transFilterActive == false) {
            return;
        }
        m_transFilterActive = false;
        m_trans_filter_dlg->ResetFilterStatus();
    }
    
    m_bitmapTransFilter->SetBitmap(m_transFilterActive 
        ? mmBitmap(png::RIGHTARROW_ACTIVE) : mmBitmap(png::RIGHTARROW));
    SetTransactionFilterState(true);

    RefreshList(m_listCtrlAccount->m_selectedID);
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
        return mmGetDateForDisplay(tran.TRANSDATE);
    case TransactionListCtrl::COL_NUMBER:
        return tran.TRANSACTIONNUMBER;
    case TransactionListCtrl::COL_CATEGORY:
        return tran.CATEGNAME;
    case TransactionListCtrl::COL_PAYEE_STR:
        return tran.is_foreign_transfer() ? "< " + tran.PAYEENAME : tran.PAYEENAME;
    case TransactionListCtrl::COL_STATUS:
        return tran.is_foreign() ? "< " + tran.STATUSFD : tran.STATUSFD;
    case TransactionListCtrl::COL_WITHDRAWAL:
        return tran.AMOUNT <= 0 ? Model_Currency::toString(std::fabs(tran.AMOUNT), this->m_currency) : "";
    case TransactionListCtrl::COL_DEPOSIT:
        return tran.AMOUNT > 0 ? Model_Currency::toString(tran.AMOUNT, this->m_currency) : "";
    case TransactionListCtrl::COL_BALANCE:
        return Model_Currency::toString(tran.BALANCE, this->m_currency);
    case TransactionListCtrl::COL_NOTES:
        return tran.NOTES;
    case TransactionListCtrl::COL_UDFC01:
        return tran.UDFC01;
    case TransactionListCtrl::COL_UDFC02:
        return tran.UDFC02;
    case TransactionListCtrl::COL_UDFC03:
        return tran.UDFC03;
    case TransactionListCtrl::COL_UDFC04:
        return tran.UDFC04;
    case TransactionListCtrl::COL_UDFC05:
        return tran.UDFC05;

    default:
        return wxEmptyString;
    }
}

void mmCheckingPanel::OnSearchTxtEntered(wxCommandEvent& event)
{
    const wxString search_string = event.GetString().Lower();
    if (search_string.IsEmpty()) return;

    long last = (long)(m_listCtrlAccount->GetItemCount() - 1);
    long selectedItem = m_listCtrlAccount->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (selectedItem <= 0 || selectedItem >= last) //nothing selected
        selectedItem = m_listCtrlAccount->g_asc ? last : 0;

    while (selectedItem >= 0 && selectedItem <= last)
    {
        m_listCtrlAccount->g_asc ? selectedItem-- : selectedItem++;

        for (const auto& t : {
            getItem(selectedItem, m_listCtrlAccount->COL_NOTES)
            , getItem(selectedItem, m_listCtrlAccount->COL_NUMBER)
            , getItem(selectedItem, m_listCtrlAccount->COL_PAYEE_STR)
            , getItem(selectedItem, m_listCtrlAccount->COL_CATEGORY)
            , getItem(selectedItem, m_listCtrlAccount->COL_DATE)
            , getItem(selectedItem, m_listCtrlAccount->COL_WITHDRAWAL)
            , getItem(selectedItem, m_listCtrlAccount->COL_DEPOSIT)})
        {
            if (t.Lower().Matches(search_string + "*"))
            {
                //First of all any items should be unselected
                long cursel = m_listCtrlAccount->GetNextItem(-1
                    , wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
                if (cursel != wxNOT_FOUND)
                    m_listCtrlAccount->SetItemState(cursel, 0
                        , wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);

                //Then finded item will be selected
                m_listCtrlAccount->SetItemState(selectedItem
                    , wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
                m_listCtrlAccount->EnsureVisible(selectedItem);
                return;
            }
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
// FIXME: don't ignore active param or remove it
    m_bitmapMainFilter->Enable(!m_transFilterActive);
    m_stxtMainFilter->Enable(!m_transFilterActive);
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
    g_sortcol(COL_DEF_SORT),
    m_prevSortCol(COL_DEF_SORT),
    g_asc(true),
    m_selectedIndex(-1),
    m_selectedForCopy(-1),
    m_selectedID(-1),
    m_cp(cp),
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
    m_topItemIndex(-1),
    m_sortCol(COL_DEF_SORT)
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

    m_columns.push_back(PANEL_COLUMN(" ", 25, wxLIST_FORMAT_LEFT));
    m_columns.push_back(PANEL_COLUMN(_("ID"), wxLIST_AUTOSIZE, wxLIST_FORMAT_LEFT));
    m_columns.push_back(PANEL_COLUMN(_("Date"), 112, wxLIST_FORMAT_LEFT));
    m_columns.push_back(PANEL_COLUMN(_("Number"), 70, wxLIST_FORMAT_LEFT));
    m_columns.push_back(PANEL_COLUMN(_("Payee"), 150, wxLIST_FORMAT_LEFT));
    m_columns.push_back(PANEL_COLUMN(_("Status"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT));
    m_columns.push_back(PANEL_COLUMN(_("Category"), 150, wxLIST_FORMAT_LEFT));
    m_columns.push_back(PANEL_COLUMN(_("Withdrawal"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT));
    m_columns.push_back(PANEL_COLUMN(_("Deposit"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT));
    m_columns.push_back(PANEL_COLUMN(_("Balance"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT));
    m_columns.push_back(PANEL_COLUMN(_("Notes"), 250, wxLIST_FORMAT_LEFT));

    const auto& ref_type = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
    for (const auto& udfc_entry : Model_CustomField::UDFC_FIELDS())
    {
        if (udfc_entry.empty()) continue;
        const auto& name = Model_CustomField::getUDFCName(ref_type, udfc_entry);
        int width = name == udfc_entry ? 0 : 100;
        m_columns.push_back(PANEL_COLUMN(name, width, wxLIST_FORMAT_LEFT));
    }

    m_col_width = "CHECK_COL%d_WIDTH";

    m_default_sort_column = COL_DEF_SORT;
    m_today = wxDateTime::Today().FormatISODate();

    SetSingleStyle(wxLC_SINGLE_SEL, false);
}

TransactionListCtrl::~TransactionListCtrl()
{
}

//----------------------------------------------------------------------------
void TransactionListCtrl::createColumns(mmListCtrl &lst)
{
    for (const auto& entry : m_columns)
    {
        int count = lst.GetColumnCount();
        lst.InsertColumn(count
            , entry.HEADER
            , entry.FORMAT
            , Model_Setting::instance().GetIntSetting(wxString::Format(m_col_width, count), entry.WIDTH));
    }
}

void TransactionListCtrl::OnListItemSelected(wxListEvent& event)
{
    m_selectedIndex = event.GetIndex();
    m_cp->updateExtraTransactionData(m_selectedIndex);
    m_topItemIndex = GetTopItem() + GetCountPerPage() - 1;

    //m_cp->enableEditDeleteButtons(true);

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

    //m_cp->enableEditDeleteButtons(true);
    
    event.Skip();
}

void TransactionListCtrl::OnMouseRightClick(wxMouseEvent& event)
{
    int Flags = wxLIST_HITTEST_ONITEM;
    m_selectedIndex = HitTest(wxPoint(event.m_x, event.m_y), Flags);

    if (m_selectedIndex >= 0)
    {
        SetItemState(m_selectedIndex, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        SetItemState(m_selectedIndex, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
    }
    m_cp->updateExtraTransactionData(m_selectedIndex);

    bool hide_menu_item = (m_selectedIndex < 0);
    bool multiselect = (GetSelectedItemCount() > 1);
    bool type_transfer = false;
    bool have_category = false;
    bool is_foreign = false;
    if (m_selectedIndex > -1)
    {
        const Model_Checking::Full_Data& tran = m_cp->m_trans.at(m_selectedIndex);
        if (Model_Checking::type(tran.TRANSCODE) == Model_Checking::TRANSFER)
        {
            type_transfer = true;
        }
        if (!tran.has_split())
        {
            have_category = true;
        }
        if (Model_Checking::foreignTransaction(tran))
        {
            is_foreign = true;
        }
    }
    wxMenu menu;
    menu.Append(MENU_TREEPOPUP_NEW_WITHDRAWAL, _("&New Withdrawal"));
    menu.Append(MENU_TREEPOPUP_NEW_DEPOSIT, _("&New Deposit"));
    if (Model_Account::instance().all_checking_account_names(true).size() > 1)
        menu.Append(MENU_TREEPOPUP_NEW_TRANSFER, _("&New Transfer"));

    menu.AppendSeparator();

    menu.Append(MENU_TREEPOPUP_EDIT2, _("&Edit Transaction"));
    if (hide_menu_item) menu.Enable(MENU_TREEPOPUP_EDIT2, false);

    menu.Append(MENU_ON_COPY_TRANSACTION, _("&Copy Transaction"));
    if (hide_menu_item) menu.Enable(MENU_ON_COPY_TRANSACTION, false);

    menu.Append(MENU_ON_PASTE_TRANSACTION, _("&Paste Transaction"));
    if (m_selectedForCopy < 0) menu.Enable(MENU_ON_PASTE_TRANSACTION, false);

    menu.Append(MENU_ON_DUPLICATE_TRANSACTION, _("D&uplicate Transaction"));
    if (hide_menu_item || multiselect) menu.Enable(MENU_ON_DUPLICATE_TRANSACTION, false);

    menu.Append(MENU_TREEPOPUP_MOVE2, _("&Move Transaction"));
    if (hide_menu_item || type_transfer || (Model_Account::money_accounts_num() < 2) || is_foreign)
        menu.Enable(MENU_TREEPOPUP_MOVE2, false);

    menu.AppendSeparator();

    menu.Append(MENU_TREEPOPUP_VIEW_SPLIT_CATEGORIES, _("&View Split Categories"));
    if (hide_menu_item || multiselect || have_category)
        menu.Enable(MENU_TREEPOPUP_VIEW_SPLIT_CATEGORIES, false);

    menu.Append(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, _("&Organize Attachments"));
    if (hide_menu_item || multiselect)
        menu.Enable(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, false);

    menu.Append(MENU_TREEPOPUP_CREATE_REOCCURANCE, _("Create Reoccuring T&ransaction"));
    if (hide_menu_item || multiselect) menu.Enable(MENU_TREEPOPUP_CREATE_REOCCURANCE, false);

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
    if (hide_menu_item || multiselect) subGlobalOpMenuMark->Enable(MENU_TREEPOPUP_MARKRECONCILED, false);
    subGlobalOpMenuMark->Append(MENU_TREEPOPUP_MARKUNRECONCILED, _("Mark As &Unreconciled"));
    if (hide_menu_item || multiselect) subGlobalOpMenuMark->Enable(MENU_TREEPOPUP_MARKUNRECONCILED, false);
    subGlobalOpMenuMark->Append(MENU_TREEPOPUP_MARKVOID, _("Mark As &Void"));
    if (hide_menu_item || multiselect) subGlobalOpMenuMark->Enable(MENU_TREEPOPUP_MARKVOID, false);
    subGlobalOpMenuMark->Append(MENU_TREEPOPUP_MARK_ADD_FLAG_FOLLOWUP, _("Mark For &Followup"));
    if (hide_menu_item || multiselect) subGlobalOpMenuMark->Enable(MENU_TREEPOPUP_MARK_ADD_FLAG_FOLLOWUP, false);
    subGlobalOpMenuMark->Append(MENU_TREEPOPUP_MARKDUPLICATE, _("Mark As &Duplicate"));
    if (hide_menu_item || multiselect) subGlobalOpMenuMark->Enable(MENU_TREEPOPUP_MARKDUPLICATE, false);
    menu.Append(wxID_ANY, _("Mark"), subGlobalOpMenuMark);

    wxMenu* subGlobalOpMenu = new wxMenu();
    subGlobalOpMenu->Append(MENU_TREEPOPUP_MARKRECONCILED_ALL, _("as Reconciled"));
    subGlobalOpMenu->Append(MENU_TREEPOPUP_MARKUNRECONCILED_ALL, _("as Unreconciled"));
    subGlobalOpMenu->Append(MENU_TREEPOPUP_MARKVOID_ALL, _("as Void"));
    subGlobalOpMenu->Append(MENU_TREEPOPUP_MARK_ADD_FLAG_FOLLOWUP_ALL, _("as needing Followup"));
    subGlobalOpMenu->Append(MENU_TREEPOPUP_MARKDUPLICATE_ALL, _("as Duplicate"));
    menu.Append(MENU_SUBMENU_MARK_ALL, _("Mark all being viewed"), subGlobalOpMenu);

    // Disable menu items not ment for foreign transactions
    if (is_foreign)
    {
        menu.Enable(MENU_ON_COPY_TRANSACTION, false);
        menu.Enable(MENU_ON_PASTE_TRANSACTION, false);
        menu.Enable(MENU_ON_DUPLICATE_TRANSACTION, false);
    }

    PopupMenu(&menu, event.GetPosition());
    this->SetFocus();
}
//----------------------------------------------------------------------------

void TransactionListCtrl::OnMarkTransaction(wxCommandEvent& event)
{
    if (GetSelectedItemCount() > 1) return;

    int evt = event.GetId();
    wxString org_status = "";
    wxString status = "";
    if (evt == MENU_TREEPOPUP_MARKRECONCILED)              status = "R";
    else if (evt == MENU_TREEPOPUP_MARKUNRECONCILED)       status = "";
    else if (evt == MENU_TREEPOPUP_MARKVOID)               status = "V";
    else if (evt == MENU_TREEPOPUP_MARK_ADD_FLAG_FOLLOWUP) status = "F";
    else if (evt == MENU_TREEPOPUP_MARKDUPLICATE)          status = "D";
    else wxASSERT(false);

    Model_Checking::Full_Data& trx = m_cp->m_trans[m_selectedIndex];
    TransactionStatus trx_status(trx);
    org_status = trx_status.Status(m_cp->m_AccountID);
    trx_status.SetStatus(status, m_cp->m_AccountID, trx);
    trx.STATUSFD = trx_status.Status(m_cp->m_AccountID);
    Model_Checking::instance().save(&trx);

    bool bRefreshRequired = (status == "V") || (org_status == "V");

    if ((m_cp->m_transFilterActive && m_cp->m_trans_filter_dlg->getStatusCheckBox()) 
        || bRefreshRequired)
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
        for (auto& trx : m_cp->m_trans)
        {
            trx.NOTES.Replace(mmAttachmentManage::GetAttachmentNoteSign(), wxEmptyString, true);
            TransactionStatus trx_status(trx);
            trx_status.SetStatus(status, m_cp->m_AccountID, trx);
            trx.STATUSFD = trx_status.Status(m_cp->m_AccountID);
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

    if (0 > ColumnNr || (size_t)ColumnNr >= m_columns.size() || ColumnNr == COL_IMGSTATUS) return;

    /* Clear previous column image */
    if (m_sortCol != ColumnNr) {
        setColumnImage(m_sortCol, -1);
    }

    if (g_sortcol == ColumnNr && event.GetId() != MENU_HEADER_SORT) {
        m_asc = !m_asc; // toggle sort order
    }
    g_asc = m_asc;

    m_prevSortCol = m_sortCol;
    m_sortCol = toEColumn(ColumnNr);
    g_sortcol = m_sortCol;

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
        if (status.length() > 1)
            status = status.Mid(2, 1);
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
    bool in_the_future = (tran.TRANSDATE > m_today);

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
    if (in_the_future)
    {
        return (item % 2 ? (wxListItemAttr*)&m_attr3 : (wxListItemAttr*)&m_attr4);
    }

    return (item % 2 ? (wxListItemAttr*)&m_attr1 : (wxListItemAttr*)&m_attr2);
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

    if (GetSelectedItemCount() > 1)
        m_selectedForCopy = -1;
    else
        m_selectedForCopy = m_cp->m_trans[m_selectedIndex].TRANSID;

    if (wxTheClipboard->Open())
    {
        const wxString separator = "\t";
        wxString data = "";
        if (GetSelectedItemCount() > 1)
        {
            for (int row = 0; row < GetItemCount(); row++)
            {
                if (GetItemState(row, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED)
                {
                    for (int column = 0; column < (int) m_columns.size(); column++)
                    {
                        if (GetColumnWidth(column) > 0)
                            data += OnGetItemText(row, column) + separator;
                    }
                    data += "\n";
                }
            }
        }
        else
        {
            for (int column = 0; column < (int) m_columns.size(); column++)
            {
                if (GetColumnWidth(column) > 0)
                    data += OnGetItemText(m_selectedIndex, column) + separator;
            }
            data += "\n";
        }
        wxTheClipboard->SetData(new wxTextDataObject(data));
        wxTheClipboard->Close();
    }
}

void TransactionListCtrl::OnDuplicateTransaction(wxCommandEvent& WXUNUSED(event))
{
    if ((m_selectedIndex < 0) || (GetSelectedItemCount() > 1)) return;

    int transaction_id = m_cp->m_trans[m_selectedIndex].TRANSID;
    mmTransDialog dlg(this, m_cp->m_AccountID, transaction_id, m_cp->m_account_balance, true);
    if (dlg.ShowModal() == wxID_OK)
    {
        m_selectedIndex = dlg.GetTransactionID();
        refreshVisualList(m_selectedIndex);
    }
    m_topItemIndex = GetTopItem() + GetCountPerPage() - 1;
}

void TransactionListCtrl::OnPaste(wxCommandEvent& WXUNUSED(event))
{
    if (m_selectedForCopy < 0) return;
    Model_Checking::Data* tran = Model_Checking::instance().get(m_selectedForCopy);
    if (tran)
    {
        if ((m_selectedIndex >= 0) && (GetSelectedItemCount() == 1))
            SetItemState(m_selectedIndex, 0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
        else if (GetSelectedItemCount() > 1)
        {
            for (int x = 0; x < GetItemCount(); x++)
            {
                if (GetItemState(x, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED)
                    SetItemState(x, 0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
            }
        }
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

void TransactionListCtrl::OnOpenAttachment(wxCommandEvent& WXUNUSED(event))
{
    if ((m_selectedIndex < 0) || (GetSelectedItemCount() > 1)) return;
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

    m_topItemIndex = GetTopItem() + GetCountPerPage() - 1;

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

void TransactionListCtrl::OnDeleteTransaction(wxCommandEvent& WXUNUSED(event))
{
    //check if a transaction is selected
    if (GetSelectedItemCount() < 1) return;

    m_topItemIndex = GetTopItem() + GetCountPerPage() - 1;

    Model_Checking::Data checking_entry = m_cp->m_trans[m_selectedIndex];
    if (TransactionLocked(checking_entry.TRANSDATE))
    {
        return;
    }

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
                SetItemState(x, 0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);

                if (Model_Checking::foreignTransaction(i))
                {
                    Model_Translink::RemoveTranslinkEntry(transID);
                    m_cp->m_frame->RefreshNavigationTree();
                }

                // remove also removes any split transactions
                Model_Checking::instance().remove(transID);

                // remove attachments
                const auto& ref_type = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
                mmAttachmentManage::DeleteAllAttachments(ref_type, transID);
                // remove custom data
                Model_CustomFieldData::instance().DeleteAllData(ref_type, transID);

                if (x <= m_topItemIndex) m_topItemIndex--;
                if (!m_cp->m_trans.empty() && m_selectedIndex > 0) m_selectedIndex--;
                if (m_selectedForCopy == transID) m_selectedForCopy = -1;
            }
            x++;
        }

        refreshVisualList();
    }
}
//----------------------------------------------------------------------------
bool TransactionListCtrl::TransactionLocked(const wxString& transdate)
{
    if (Model_Account::BoolOf(m_cp->m_account->STATEMENTLOCKED))
    {
        wxDateTime transaction_date;
        if (transaction_date.ParseDate(transdate))
        {
            if (transaction_date <= Model_Account::DateOf(m_cp->m_account->STATEMENTDATE))
            {
                wxMessageBox(_(wxString::Format(
                    "Locked transaction to date: %s\n\n"
                    "Reconciled transactions."
                    , mmGetDateForDisplay(m_cp->m_account->STATEMENTDATE)))
                    , _("MMEX Transaction Check"), wxOK | wxICON_WARNING);
                return true;
            }
        }
    }
    return false;
}

void TransactionListCtrl::OnEditTransaction(wxCommandEvent& WXUNUSED(event))
{
    if (GetSelectedItemCount() > 1)
    {
        std::vector<int> v;
        long x = 0;
        for (const auto& i : m_cp->m_trans)
        {
            if (GetItemState(x, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED)
            {
                v.push_back( i.TRANSID);
            }
            x++;
        }

        transactionsUpdateDialog dlg(this, m_cp->m_AccountID, v);
        if (dlg.ShowModal() == wxID_OK)
        {
            refreshVisualList();
        }
        return;
    }

    if (m_selectedIndex < 0) return;

    Model_Checking::Data checking_entry = m_cp->m_trans[m_selectedIndex];
    int transaction_id = checking_entry.TRANSID;

    if (TransactionLocked(checking_entry.TRANSDATE))
    {
        return;
    }

    if (Model_Checking::foreignTransaction(checking_entry))
    {
        Model_Translink::Data translink = Model_Translink::TranslinkRecord(transaction_id);
        if (translink.LINKTYPE == Model_Attachment::reftype_desc(Model_Attachment::STOCK))
        {
            ShareTransactionDialog dlg(this, &translink, &checking_entry);
            if (dlg.ShowModal() == wxID_OK)
            {
                refreshVisualList(transaction_id);
            }
        }
        else
        {
            mmAssetDialog dlg(this, m_cp->m_frame, &translink, &checking_entry);
            if (dlg.ShowModal() == wxID_OK)
            {
                refreshVisualList(transaction_id);
            }
        }
    }
    else
    {
        mmTransDialog dlg(this, m_cp->m_AccountID, transaction_id, m_cp->m_account_balance);
        if (dlg.ShowModal() == wxID_OK)
        {
            refreshVisualList(transaction_id);
        }
    }
    m_topItemIndex = GetTopItem() + GetCountPerPage() - 1;
}

void TransactionListCtrl::OnNewTransaction(wxCommandEvent& event)
{
    int type = event.GetId() == MENU_TREEPOPUP_NEW_DEPOSIT ? Model_Checking::DEPOSIT : Model_Checking::WITHDRAWAL;
    mmTransDialog dlg(this, m_cp->m_AccountID, 0, m_cp->m_account_balance, false, type);
    if (dlg.ShowModal() == wxID_OK)
    {
        m_cp->mmPlayTransactionSound();
        refreshVisualList(dlg.GetTransactionID());
    }
}

void TransactionListCtrl::OnNewTransferTransaction(wxCommandEvent& WXUNUSED(event))
{
    mmTransDialog dlg(this, m_cp->m_AccountID, 0, m_cp->m_account_balance, false, Model_Checking::TRANSFER);
    if (dlg.ShowModal() == wxID_OK)
    {
        m_cp->mmPlayTransactionSound();
        refreshVisualList(dlg.GetTransactionID());
    }
}
//----------------------------------------------------------------------------

void TransactionListCtrl::OnSetUserColour(wxCommandEvent& event)
{
    if ((m_selectedIndex < 0) || (GetSelectedItemCount() > 1)) return;

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
    m_today = wxDateTime::Today().FormatISODate();
    this->SetEvtHandlerEnabled(false);
    Hide();

    // decide whether top or down icon needs to be shown
    setColumnImage(g_sortcol, g_asc ? ICON_ASC : ICON_DESC);
    if (filter) m_cp->filterTable();
    SetItemCount(m_cp->m_trans.size());
    Show();
    m_cp->sortTable();
    m_cp->markSelectedTransaction(trans_id);

    long i = (long)m_cp->m_trans.size();
    if (m_topItemIndex >= i)
        m_topItemIndex = g_asc ? i - 1 : 0;
    if (m_selectedIndex > i - 1) m_selectedIndex = -1;
    if (m_topItemIndex < m_selectedIndex) m_topItemIndex = m_selectedIndex;


    if (m_selectedIndex >= 0 && !m_cp->m_trans.empty())
    {
        SetItemState(m_selectedIndex, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        SetItemState(m_selectedIndex, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
        if (m_topItemIndex < 0 || (m_topItemIndex - m_selectedIndex) > GetCountPerPage())
            m_topItemIndex = m_selectedIndex;
        EnsureVisible(m_topItemIndex);
    }

    m_cp->setAccountSummary();
    m_cp->updateExtraTransactionData(m_selectedIndex);
    this->SetEvtHandlerEnabled(true);
    Refresh();
    Update();
    m_cp->m_listCtrlAccount->SetFocus();
}

void TransactionListCtrl::OnMoveTransaction(wxCommandEvent& WXUNUSED(event))
{
    if ((m_selectedIndex < 0)) return;

	
	// Abort if any transaction is locked.
	for (long index : GetSelected()) {
		Model_Checking::Data checking_entry = m_cp->m_trans[index];
		if (TransactionLocked(checking_entry.TRANSDATE)){
			return;
		}
	}

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

		// Update all selected transactions, first grab all transactions as we want to modify multiple ones in sequence and indices can change.
		std::vector<Model_Checking::Full_Data> transactions;
		for (long index : GetSelected()) {
			Model_Checking::Full_Data& tran = m_cp->m_trans[index];
			transactions.push_back(tran);
		}
		
		// Save the modification
		for (auto transaction : transactions) {
			transaction.ACCOUNTID = dest_account_id;
			Model_Checking::instance().save(&transaction);
		}

        refreshVisualList();
    }
}

//----------------------------------------------------------------------------
void TransactionListCtrl::OnViewSplitTransaction(wxCommandEvent& WXUNUSED(event))
{
    if ((m_selectedIndex > -1) && (GetSelectedItemCount() == 1)) {
        const Model_Checking::Full_Data& tran = m_cp->m_trans.at(m_selectedIndex);
        if (tran.has_split())
            m_cp->DisplaySplitCategories(tran.TRANSID);
    }
}

//----------------------------------------------------------------------------
void TransactionListCtrl::OnOrganizeAttachments(wxCommandEvent& WXUNUSED(event))
{
    if ((m_selectedIndex < 0) || (GetSelectedItemCount() > 1)) return;

    wxString RefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
    int RefId = m_cp->m_trans[m_selectedIndex].TRANSID;

    mmAttachmentDialog dlg(this, RefType, RefId);
    dlg.ShowModal();

    refreshVisualList(RefId);
}

//----------------------------------------------------------------------------
void TransactionListCtrl::OnCreateReoccurance(wxCommandEvent& WXUNUSED(event))
{
    if ((m_selectedIndex < 0) || (GetSelectedItemCount() > 1)) return;

    mmBDDialog dlg(this, 0, false, false);
    dlg.SetDialogParameters(m_cp->m_trans[m_selectedIndex]);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxMessageBox(_("Reoccuring Transaction saved."));
    }
}

//----------------------------------------------------------------------------
void TransactionListCtrl::OnListItemActivated(wxListEvent& WXUNUSED(event))
{
    if (m_selectedIndex < 0) return;

    wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_TREEPOPUP_EDIT2);
    AddPendingEvent(evt);
}

void TransactionListCtrl::OnListItemFocused(wxListEvent& WXUNUSED(event))
{
    m_cp->enableEditDeleteButtons(true);

    long count = this->GetSelectedItemCount();
    if (count < 2)
        return;

    long x = 0;
    wxString maxDate, minDate;
    double balance = 0;
    for (const auto& i : m_cp->m_trans)
    {
        if (GetItemState(x, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED)
        {
            balance += Model_Checking::balance(i);
            if (minDate > i.TRANSDATE || maxDate.empty()) minDate = i.TRANSDATE;
            if (maxDate < i.TRANSDATE || maxDate.empty()) maxDate = i.TRANSDATE;
        }
        x++;
    }

    wxDateTime min_date, max_date;
    min_date.ParseISODate(minDate);
    max_date.ParseISODate(maxDate);

    int days = wxTimeSpan(max_date.Subtract(min_date)).GetDays();

    wxString msg;
    Model_Account::Data *account = Model_Account::instance().get(m_cp->m_AccountID);
    msg = wxString::Format(_("Transactions selected: %ld"), count);
    msg += "\n";
    msg += wxString::Format(_("Selected transactions balance: %s")
        , Model_Account::toCurrency(balance, account));
    msg += "\n";
    msg += wxString::Format(_("Days between selected transactions: %d"), days);

    m_cp->m_info_panel->SetLabelText(msg);
}
