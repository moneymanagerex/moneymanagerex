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

#include "constants.h"
#include "util.h"
#include "mmcheckingpanel.h"
#include "mmex.h"
#include "mmframe.h"
#include "mmTips.h"
#include "mmsinglechoicedialog.h"
#include "splittransactionsdialog.h"
#include "transdialog.h"
#include "validators.h"
#include "model/Model_Infotable.h"
#include "model/Model_Setting.h"
#include "model/Model_Checking.h"
#include "model/Model_Splittransaction.h"
#include "model/Model_Account.h"
#include "model/Model_Payee.h"
#include "model/Model_Category.h"

#include "../resources/reconciled.xpm"
#include "../resources/unreconciled.xpm"
#include "../resources/duplicate.xpm"
#include "../resources/flag.xpm"
#include "../resources/void.xpm"
#include "../resources/rightarrow.xpm"
#include "../resources/uparrow.xpm"
#include "../resources/downarrow.xpm"
#include "../resources/tipicon.xpm"
#include "../resources/trash.xpm"


//----------------------------------------------------------------------------

#include <wx/srchctrl.h>
#include <algorithm>
//----------------------------------------------------------------------------

#ifndef _WIN32
#include <sys/time.h>

unsigned GetTickCount()
{
    struct timeval tv;
    if (gettimeofday(&tv, NULL) != 0)
        return 0;

    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}
#endif

BEGIN_EVENT_TABLE(mmCheckingPanel, wxPanel)
    EVT_BUTTON(wxID_NEW,         mmCheckingPanel::OnNewTransaction)
    EVT_BUTTON(wxID_EDIT,        mmCheckingPanel::OnEditTransaction)
    EVT_BUTTON(wxID_REMOVE,      mmCheckingPanel::OnDeleteTransaction)
    EVT_BUTTON(wxID_DUPLICATE,    mmCheckingPanel::OnDuplicateTransaction)
    EVT_MENU(wxID_ANY, mmCheckingPanel::OnViewPopupSelected)
    EVT_SEARCHCTRL_SEARCH_BTN(wxID_FIND, mmCheckingPanel::OnSearchTxtEntered)
    EVT_TEXT_ENTER(wxID_FIND, mmCheckingPanel::OnSearchTxtEntered)
END_EVENT_TABLE()
//----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(TransactionListCtrl, wxListCtrl)

    EVT_LIST_ITEM_SELECTED(wxID_ANY, TransactionListCtrl::OnListItemSelected)
    EVT_LIST_ITEM_DESELECTED(wxID_ANY, TransactionListCtrl::OnListItemDeselected)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY, TransactionListCtrl::OnListItemActivated)
    //EVT_LIST_ITEM_RIGHT_CLICK(wxID_ANY, TransactionListCtrl::OnItemRightClick)
    EVT_RIGHT_DOWN(TransactionListCtrl::OnListRightClick)
    EVT_LEFT_DOWN(TransactionListCtrl::OnListLeftClick)
    EVT_LIST_COL_END_DRAG(wxID_ANY, TransactionListCtrl::OnItemResize)
    EVT_LIST_COL_CLICK(wxID_ANY, TransactionListCtrl::OnColClick)
    EVT_LIST_KEY_DOWN(wxID_ANY,  TransactionListCtrl::OnListKeyDown)

    EVT_MENU_RANGE(MENU_TREEPOPUP_MARKRECONCILED
        ,MENU_TREEPOPUP_MARKDELETE,             TransactionListCtrl::OnMarkTransaction)

    EVT_MENU_RANGE(MENU_TREEPOPUP_MARKRECONCILED_ALL
        ,MENU_TREEPOPUP_DELETE_FLAGGED,         TransactionListCtrl::OnMarkAllTransactions)
    EVT_MENU(MENU_TREEPOPUP_SHOWTRASH,          TransactionListCtrl::OnShowChbClick)

    EVT_MENU(MENU_TREEPOPUP_NEW2,                TransactionListCtrl::OnNewTransaction)
    EVT_MENU(MENU_TREEPOPUP_DELETE2,             TransactionListCtrl::OnDeleteTransaction)
    EVT_MENU(MENU_TREEPOPUP_EDIT2,               TransactionListCtrl::OnEditTransaction)
    EVT_MENU(MENU_TREEPOPUP_MOVE2,               TransactionListCtrl::OnMoveTransaction)

    EVT_MENU(MENU_ON_COPY_TRANSACTION,      TransactionListCtrl::OnCopy)
    EVT_MENU(MENU_ON_PASTE_TRANSACTION,     TransactionListCtrl::OnPaste)
    EVT_MENU(MENU_ON_NEW_TRANSACTION,       TransactionListCtrl::OnNewTransaction)
    EVT_MENU(MENU_ON_DUPLICATE_TRANSACTION, TransactionListCtrl::OnDuplicateTransaction)
    EVT_MENU_RANGE(MENU_ON_SET_UDC0, MENU_ON_SET_UDC7, TransactionListCtrl::OnSetUserColour)

    EVT_MENU(MENU_TREEPOPUP_VIEW_SPLIT_CATEGORIES, TransactionListCtrl::OnViewSplitTransaction)

    EVT_CHAR(TransactionListCtrl::OnChar)

END_EVENT_TABLE();

//----------------------------------------------------------------------------

mmCheckingPanel::mmCheckingPanel(
    int accountID,
    wxWindow *parent)
    : filteredBalance_(0.0)
    , m_listCtrlAccount()
    , m_AccountID(accountID)
    , m_account(Model_Account::instance().get(accountID))
    , m_currency(Model_Account::currency(m_account))
    , transFilterDlg_(0)
{
    m_basecurrecyID = Model_Infotable::instance().GetBaseCurrencyId();
    long style = wxTAB_TRAVERSAL | wxNO_BORDER;
    Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, style, wxPanelNameStr);
}
//----------------------------------------------------------------------------

/*
    We cannon use OnClose() event because this class deletes
    via DestroyChildren() of its parent.
*/
mmCheckingPanel::~mmCheckingPanel()
{
    if (transFilterDlg_) delete transFilterDlg_;
    wxGetApp().m_frame->SetCheckingAccountPageInactive();
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
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    transFilterActive_ = false;
    transFilterDlg_    = new mmFilterTransactionsDialog(this);
    SetTransactionFilterState(true);

    initViewTransactionsHeader();
    initFilterSettings();

    m_listCtrlAccount->refreshVisualList();
    this->windowsFreezeThaw();

    return true;
}

void mmCheckingPanel::sortTable()
{
    std::sort(this->m_trans.begin(), this->m_trans.end());
    switch (m_listCtrlAccount->g_sortcol)
    {
    case TransactionListCtrl::COL_NUMBER:
        std::stable_sort(this->m_trans.begin(), this->m_trans.end(), SorterByTRANSACTIONNUMBER());
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
        std::stable_sort(this->m_trans.begin(), this->m_trans.end()
            , [&](const Model_Checking::Data& x, const Model_Checking::Data& y)
        {
            return Model_Checking::amount(x, this->m_AccountID) >= Model_Checking::amount(y, this->m_AccountID);
        });
        break;
    case TransactionListCtrl::COL_DEPOSIT:
        std::stable_sort(this->m_trans.begin(), this->m_trans.end()
            , [&](const Model_Checking::Data& x, const Model_Checking::Data& y)
        {
            return Model_Checking::amount(x, this->m_AccountID) < Model_Checking::amount(y, this->m_AccountID);
        });
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

    int interval = GetTickCount();
    const auto& trans = Model_Account::transaction(this->m_account);

    std::map <wxString, int> speed;
    speed["1. Get Data"] += GetTickCount() - interval;

#if 0
    copy(trans.begin(), trans.end(), back_inserter(m_trans));
#else
    for (const auto& tran : trans)
    {
        double transaction_amount = Model_Checking::amount(tran, m_AccountID);
        if (Model_Checking::status(tran) != Model_Checking::VOID_)
            account_balance_ += transaction_amount;
        if (Model_Checking::status(tran) != Model_Checking::VOID_)
            reconciled_balance_ += transaction_amount;
        //reconciled_balance_ += Model_Checking::reconciled(tran, m_AccountID);
        speed["2. Balances calc"] += GetTickCount() - interval;

        if (transFilterActive_)
        {
            interval = GetTickCount();
            if (!transFilterDlg_->checkAll(tran, m_AccountID))
            {
                speed["3. Filter"] += GetTickCount() - interval;
                continue;
            }
            speed["3. Filter"] += GetTickCount() - interval;
        }
        else
        {
            interval = GetTickCount();
            if (!Model_Checking::TRANSDATE(tran)
                .IsBetween(quickFilterBeginDate_, quickFilterEndDate_))
            {
                speed["3. Filter"] += GetTickCount() - interval;
                continue;
            }
            speed["3. Filter"] += GetTickCount() - interval;
        }

        if (!m_listCtrlAccount->showDeletedTransactions_ && Model_Checking::status(tran) == Model_Checking::VOID_)
            continue;

        interval = GetTickCount();
        Model_Checking::Full_Data full_tran(tran);
        full_tran.BALANCE = account_balance_;
        speed["4. Get Full_Data"] += GetTickCount() - interval;

        full_tran.DEPOSIT = transaction_amount;
        full_tran.WITHDRAWAL = transaction_amount;


        interval = GetTickCount();
        if (Model_Checking::TRANSFER == Model_Checking::type(tran))
        {
            bool transfer_to = tran.ACCOUNTID == this->m_AccountID;
            full_tran.PAYEENAME = transfer_to ? "> " + full_tran.TOACCOUNTNAME : "< " + full_tran.ACCOUNTNAME;
        }

        filteredBalance_ += transaction_amount;
        this->m_trans.push_back(full_tran);
        speed["5. Transfer prefix + push_back"] += GetTickCount() - interval;
    }
#endif
    wxLogDebug("-----------------------------------------------------------------\n Records: %s", wxString() << trans.size());
    for (const auto& item : speed)
    {
        wxLogDebug("%s - %s ms", item.first, wxString::Format("%i", item.second));
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
        if (Model_Checking::status(tran) != Model_Checking::VOID_)
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
    long i = 0;
    for (const auto & tran : m_trans)
    {
        if (trans_id == tran.TRANSID && trans_id > 0)
        {
            m_listCtrlAccount->m_selectedIndex = i;
            break;
        }
        ++i;
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
    int border = 1;
    wxSizerFlags flags, flagsExpand;
    flags.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxALL, border);
    flagsExpand.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxEXPAND).Border(wxALL, border).Proportion(1);

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer9);

    /* ---------------------- */
    wxPanel* headerPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition,
        wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);
    itemBoxSizer9->Add(headerPanel, flags);

    wxBoxSizer* itemBoxSizerVHeader = new wxBoxSizer(wxVERTICAL);
    headerPanel->SetSizer(itemBoxSizerVHeader);

    wxGridSizer* itemBoxSizerVHeader2 = new wxGridSizer(0, 1, 5, 20);
    itemBoxSizerVHeader->Add(itemBoxSizerVHeader2);

    header_text_ = new wxStaticText( headerPanel, wxID_STATIC, "");
    int font_size = this->GetFont().GetPointSize() + 2;
    header_text_->SetFont(wxFont(font_size, wxSWISS, wxNORMAL, wxBOLD, FALSE, ""));
    itemBoxSizerVHeader2->Add(header_text_, flags);

    wxBoxSizer* itemBoxSizerHHeader2 = new wxBoxSizer(wxHORIZONTAL);
    wxFlexGridSizer* itemFlexGridSizerHHeader2 = new wxFlexGridSizer(5, 1, 1);
    itemBoxSizerVHeader2->Add(itemBoxSizerHHeader2);
    itemBoxSizerHHeader2->Add(itemFlexGridSizerHHeader2);

    wxBitmap itemStaticBitmap(rightarrow_xpm);
    bitmapMainFilter_ = new wxStaticBitmap(headerPanel, wxID_ANY
        , itemStaticBitmap);
    itemFlexGridSizerHHeader2->Add(bitmapMainFilter_, flags);
    bitmapMainFilter_->Connect(wxID_ANY, wxEVT_RIGHT_DOWN
        , wxMouseEventHandler(mmCheckingPanel::OnFilterResetToViewAll), NULL, this);
    bitmapMainFilter_->Connect(wxID_ANY, wxEVT_LEFT_DOWN
        , wxMouseEventHandler(mmCheckingPanel::OnMouseLeftDown), NULL, this);

    stxtMainFilter_ = new wxStaticText( headerPanel, wxID_ANY, "", wxDefaultPosition, wxSize(200, -1));
    itemFlexGridSizerHHeader2->Add(stxtMainFilter_, flags);

    itemFlexGridSizerHHeader2->AddSpacer(20);

    bitmapTransFilter_ = new wxStaticBitmap( headerPanel, ID_PANEL_CHECKING_STATIC_BITMAP_FILTER
        , itemStaticBitmap);
    itemFlexGridSizerHHeader2->Add(bitmapTransFilter_, flags);
    bitmapTransFilter_->Connect(wxID_ANY, wxEVT_LEFT_DOWN
        , wxMouseEventHandler(mmCheckingPanel::OnFilterTransactions), NULL, this);
    bitmapTransFilter_->Connect(wxID_ANY, wxEVT_RIGHT_DOWN
        , wxMouseEventHandler(mmCheckingPanel::OnFilterTransactions), NULL, this);

    statTextTransFilter_ = new wxStaticText( headerPanel, wxID_ANY,
        _("Transaction Filter"));
    itemFlexGridSizerHHeader2->Add(statTextTransFilter_, flags);

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
    itemBoxSizerVHeader->Add(balances_header, flagsExpand);
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
        , ID_SPLITTERWINDOW, wxDefaultPosition, wxSize(200, 200)
        , wxSP_3DBORDER | wxSP_3DSASH | wxNO_BORDER);

    wxSize imageSize(16, 16);
    m_imageList.reset(new wxImageList(imageSize.GetWidth(), imageSize.GetHeight()));
    m_imageList->Add(wxImage(reconciled_xpm).Scale(16, 16));
    m_imageList->Add(wxImage(void_xpm).Scale(16, 16));
    m_imageList->Add(wxImage(flag_xpm).Scale(16, 16));
    m_imageList->Add(wxImage(unreconciled_xpm).Scale(16, 16));
    m_imageList->Add(wxImage(uparrow_xpm).Scale(16, 16));
    m_imageList->Add(wxImage(downarrow_xpm).Scale(16, 16));
    m_imageList->Add(wxImage(duplicate_xpm).Scale(16, 16));
    m_imageList->Add(wxImage(trash_xpm).Scale(16, 16));

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

    itemBoxSizer9->Add(itemSplitterWindow10, flagsExpand);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemPanel12->SetSizer(itemBoxSizer4);

    wxBoxSizer* itemButtonsSizer = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer4->Add(itemButtonsSizer, flags);

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

    wxSearchCtrl* searchCtrl = new wxSearchCtrl(itemPanel12
        , wxID_FIND, wxEmptyString, wxDefaultPosition
        , wxSize(100, btnDuplicate_->GetSize().GetHeight())
        , wxTE_PROCESS_ENTER | wxTE_PROCESS_TAB, wxDefaultValidator, _("Search"));
    searchCtrl->SetHint(_("Search"));
    itemButtonsSizer->Add(searchCtrl, 0, wxCENTER, 1);
    searchCtrl->SetToolTip(_("Enter any string to find it in the nearest transaction notes"));

    //Infobar-mini
    info_panel_mini_ = new wxStaticText( itemPanel12, wxID_STATIC, "");
    itemButtonsSizer->Add(info_panel_mini_, 1, wxGROW | wxTOP | wxLEFT, 5);

    //Infobar
    info_panel_ = new wxStaticText(itemPanel12
        , wxID_STATIC, "", wxDefaultPosition, wxSize(200, -1), wxTE_MULTILINE | wxTE_WORDWRAP);
    itemBoxSizer4->Add(info_panel_, flagsExpand);
    //Show tips when no any transaction selected
    showTips();
}

//----------------------------------------------------------------------------
void mmCheckingPanel::setAccountSummary()
{
    Model_Account::Data *account = Model_Account::instance().get(m_AccountID);

    if (account)
        header_text_->SetLabel(wxString::Format(_("Account View : %s"), account->ACCOUNTNAME));

    bool show_displayed_balance_ = (transFilterActive_ || (currentView_ != VIEW_TRANS_ALL_STR));
    wxStaticText* header = (wxStaticText*)FindWindow(ID_PANEL_CHECKING_STATIC_BALHEADER1);
    header->SetLabel(Model_Account::toCurrency(account_balance_, account));
    header = (wxStaticText*)FindWindow(ID_PANEL_CHECKING_STATIC_BALHEADER2);
    header->SetLabel(Model_Account::toCurrency(reconciled_balance_, account));
    header = (wxStaticText*)FindWindow(ID_PANEL_CHECKING_STATIC_BALHEADER3);
    header->SetLabel(Model_Account::toCurrency(account_balance_ - reconciled_balance_, account));
    header = (wxStaticText*)FindWindow(ID_PANEL_CHECKING_STATIC_BALHEADER4);
    header->SetLabel(show_displayed_balance_
        ? _("Displayed Bal: ")
        : "                                 ");
    header = (wxStaticText*)FindWindow(ID_PANEL_CHECKING_STATIC_BALHEADER5);
    header->SetLabel(show_displayed_balance_
        ? Model_Account::toCurrency(filteredBalance_, account)
        : "                                 ");
}

//----------------------------------------------------------------------------
void mmCheckingPanel::enableEditDeleteButtons(bool en)
{
    if (m_listCtrlAccount->GetSelectedItemCount()>1)
    {
        btnEdit_->Enable(false);
        btnDelete_->Enable(true);
        btnDuplicate_->Enable(false);
    }
    else
    {
        btnEdit_->Enable(en);
        btnDelete_->Enable(en);
        btnDuplicate_->Enable(en);
    }
}
//----------------------------------------------------------------------------

void mmCheckingPanel::updateExtraTransactionData(int selIndex)
{
    if (selIndex > -1)
    {
        enableEditDeleteButtons(true);
        const Model_Checking::Full_Data& tran = this->m_trans.at(selIndex);
        info_panel_->SetLabel(tran.NOTES);
        wxString miniStr;
        miniStr = getMiniInfoStr(selIndex);

        //Show only first line but full string set as tooltip
        if (miniStr.Find("\n") > 1 && !miniStr.IsEmpty())
        {
            info_panel_mini_->SetLabel(miniStr.substr(0,miniStr.Find("\n")) + " ...");
            info_panel_mini_->SetToolTip(miniStr);
        }
        else
        {
            info_panel_mini_->SetLabel(miniStr);
            info_panel_mini_->SetToolTip(miniStr);
        }

    }
    else
    {
        info_panel_mini_->SetLabel("");
        enableEditDeleteButtons(false);
        showTips();
    }
}
//----------------------------------------------------------------------------
void mmCheckingPanel::showTips()
{
    info_panel_->SetLabel(getRandomTip());
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
   mmPlayTransactionSound();
}
//----------------------------------------------------------------------------

void mmCheckingPanel::OnEditTransaction(wxCommandEvent& event)
{
    m_listCtrlAccount->OnEditTransaction(event);
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

void mmCheckingPanel::initViewTransactionsHeader()
{
    wxString vTrans = Model_Setting::instance().GetStringSetting("VIEWTRANSACTIONS", VIEW_TRANS_ALL_STR);
    int def_view_selection = menu_labels().Index(vTrans);
    currentView_ = Model_Infotable::instance().GetIntInfo(wxString::Format("CHECK_FILTER_ID_%d", m_AccountID), def_view_selection);

    SetTransactionFilterState(currentView_ == VIEW_TRANS_ALL_STR);
    stxtMainFilter_->SetLabel(wxGetTranslation(menu_labels()[currentView_]));
}
//----------------------------------------------------------------------------
void mmCheckingPanel::initFilterSettings()
{
    mmDateRange* date_range = 0;

    if (currentView_ == MENU_VIEW_ALLTRANSACTIONS)
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

    if (currentView_ == VIEW_TRANS_ALL_STR)
    {
        event.Skip();
        return;
    }

    currentView_ = MENU_VIEW_ALLTRANSACTIONS;
    stxtMainFilter_->SetLabel(menu_labels()[currentView_]);
    SetTransactionFilterState(true);
    initFilterSettings();

    m_listCtrlAccount->m_selectedIndex = -1;
    m_listCtrlAccount->refreshVisualList();

}

void mmCheckingPanel::OnViewPopupSelected(wxCommandEvent& event)
{
    currentView_ = event.GetId() - wxID_HIGHEST;

    if (currentView_ == MENU_VIEW_ALLTRANSACTIONS)
        transFilterActive_ = false;

    stxtMainFilter_->SetLabel(wxGetTranslation(menu_labels()[currentView_]));
    SetTransactionFilterState(currentView_ == VIEW_TRANS_ALL_STR);

    m_listCtrlAccount->m_selectedIndex = -1;

    Model_Infotable::instance().Set(wxString::Format("CHECK_FILTER_ID_%ld", (long)m_AccountID), currentView_);
    initFilterSettings();
    m_listCtrlAccount->refreshVisualList(m_listCtrlAccount->m_selectedID);

}

void mmCheckingPanel::DeleteViewedTransactions()
{
    for (const auto& tran: this->m_trans)
    {
        // remove also removes any split transactions
        Model_Checking::instance().remove(tran.TRANSID);
        if (m_listCtrlAccount->m_selectedForCopy == tran.TRANSID) m_listCtrlAccount->m_selectedForCopy = -1;
    }
}

void mmCheckingPanel::DeleteFlaggedTransactions(const wxString& status)
{
    for (const auto& tran: this->m_trans)
    {
        if (tran.STATUS == status)
        {
            // remove also removes any split transactions
            Model_Checking::instance().remove(tran.TRANSID);
            if (m_listCtrlAccount->m_selectedForCopy == tran.TRANSID) m_listCtrlAccount->m_selectedForCopy = -1;
        }
    }
}

void mmCheckingPanel::OnFilterTransactions(wxMouseEvent& event)
{
    int e = event.GetEventType();

    wxBitmap bitmapFilterIcon(rightarrow_xpm);

    if (e == wxEVT_LEFT_DOWN) {
        transFilterDlg_->setAccountToolTip("Select account used in transfer transactions");
        if (transFilterDlg_->ShowModal() == wxID_OK && transFilterDlg_->somethingSelected())
        {
            transFilterActive_ = true;
            wxBitmap activeBitmapFilterIcon(tipicon_xpm);
            bitmapFilterIcon = activeBitmapFilterIcon;
        }
        else
        {
            transFilterActive_ = false;
        }

    } else {
        if (transFilterActive_ == false) return;
        transFilterActive_ = false;
    }

    wxImage pic = bitmapFilterIcon.ConvertToImage();
    bitmapTransFilter_->SetBitmap(pic);
    SetTransactionFilterState(true);

    m_listCtrlAccount->refreshVisualList();
}


const wxString mmCheckingPanel::getItem(long item, long column)
{
    if (item < 0 || item >= (int)m_trans.size()) return "";

    const Model_Checking::Full_Data& tran = this->m_trans.at(item);
    switch (column)
    {
    case TransactionListCtrl::COL_DATE:
        return mmGetDateForDisplay(Model_Checking::TRANSDATE(tran));
    case TransactionListCtrl::COL_NUMBER:
        return tran.TRANSACTIONNUMBER;
    case TransactionListCtrl::COL_PAYEE_STR:
        return tran.PAYEENAME;
    case TransactionListCtrl::COL_STATUS:
        return tran.STATUS;
    case TransactionListCtrl::COL_CATEGORY:
    {
                                              if (Model_Checking::splittransaction(tran).empty())
                                                  return  Model_Category::instance().full_name(tran.CATEGID, tran.SUBCATEGID);
                                              else
                                                  return "...";
    }
    case TransactionListCtrl::COL_WITHDRAWAL:
        return tran.WITHDRAWAL <= 0 ? Model_Currency::toString(tran.WITHDRAWAL, this->m_currency) : "";
    case TransactionListCtrl::COL_DEPOSIT:
        return tran.DEPOSIT > 0 ? Model_Currency::toString(tran.DEPOSIT, this->m_currency) : "";
    case TransactionListCtrl::COL_BALANCE:
        return Model_Currency::toString(tran.BALANCE, this->m_currency);
    case TransactionListCtrl::COL_NOTES:
        return tran.NOTES;
    default:
        return "";
    }
}

void mmCheckingPanel::OnSearchTxtEntered(wxCommandEvent& /*event*/)
{
    //event.GetString() does not working. It seems wxWidgets issue
    //wxString searchString = event.GetString();

    wxSearchCtrl* st = (wxSearchCtrl*)FindWindow(wxID_FIND);
    wxString search_string = st->GetValue().Lower();
    if (search_string.IsEmpty()) return;

    double amount= 0, deposit = 0, withdrawal = 0;
    bool valid_amount = Model_Currency::fromString(search_string, amount, Model_Account::currency(Model_Account::instance().get(this->m_AccountID)));
    bool withdrawal_only = false;
    if (valid_amount && amount < 0)
    {
        amount = -amount;
        withdrawal_only = true;
    }

    long last = m_listCtrlAccount->GetItemCount();
    long selectedItem = m_listCtrlAccount->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (selectedItem < 0) //nothing selected
        selectedItem = m_listCtrlAccount->g_asc ? last - 1 : 0;

    search_string.Append("*");
    while (selectedItem >= 0 && selectedItem <= last)
    {
        m_listCtrlAccount->g_asc ?  selectedItem-- : selectedItem++;
        const wxString t = getItem(selectedItem, m_listCtrlAccount->COL_NOTES);
        if (valid_amount)  Model_Currency::fromString(getItem(selectedItem, m_listCtrlAccount->COL_DEPOSIT), deposit, 0);
        if (valid_amount)  Model_Currency::fromString(getItem(selectedItem, m_listCtrlAccount->COL_WITHDRAWAL), withdrawal, 0);
        if (t.Lower().Matches(search_string)
            || (valid_amount && amount == deposit && !withdrawal_only)
            || (valid_amount && amount == withdrawal))
        {
            //First of all any items should be unselected
            long cursel = m_listCtrlAccount->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            if (cursel != wxNOT_FOUND)
                m_listCtrlAccount->SetItemState(cursel, !wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
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
    int transType = Model_Checking::type(tran);

    Model_Checking::Data *transaction = Model_Checking::instance().get(transID);
    Model_Splittransaction::Data_Set splits = Model_Checking::splittransaction(transaction);
    SplitTransactionDialog splitTransDialog(this
        , &splits
        , transType
        , m_AccountID);

    splitTransDialog.SetDisplaySplitCategories();
    splitTransDialog.ShowModal();
}

// Refresh account screen with new details
void mmCheckingPanel::DisplayAccountDetails(int accountID)
{
    m_AccountID = accountID;
    m_account = Model_Account::instance().get(accountID);
    if (m_account)
        m_currency = Model_Account::currency(m_account);
    this->windowsFreezeThaw();   // prevent screen updates while controls being repopulated

    initViewTransactionsHeader();

    m_listCtrlAccount->m_selectedIndex = -1;
    m_listCtrlAccount->refreshVisualList();
    showTips();

    this->windowsFreezeThaw();     // Enable screen refresh with new data.
}

void mmCheckingPanel::RefreshList()
{
    m_listCtrlAccount->refreshVisualList();
}

void mmCheckingPanel::SetTransactionFilterState(bool active)
{
    /*bitmapTransFilter_->Enable(active || transFilterActive_);
    statTextTransFilter_->Enable(active || transFilterActive_);
    bitmapMainFilter_->Enable(!transFilterActive_);
    stxtMainFilter_->Enable(!transFilterActive_);*/
}

void mmCheckingPanel::SetSelectedTransaction(int transID)
{
    m_listCtrlAccount->refreshVisualList(transID);
    m_listCtrlAccount->SetFocus();
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
    m_asc(true),
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

    showDeletedTransactions_ = Model_Setting::instance().GetBoolSetting("SHOW_DELETED_TRANS", true);
}

//----------------------------------------------------------------------------
void TransactionListCtrl::createColumns(mmListCtrl &lst)
{
    lst.InsertColumn(COL_DATE, "      " + _("Date"), wxLIST_FORMAT_LEFT
        , Model_Setting::instance().GetIntSetting(wxString::Format("CHECK_COL%i_WIDTH", COL_DATE), 112));
    lst.InsertColumn(COL_NUMBER, _("Number"), wxLIST_FORMAT_LEFT
        , Model_Setting::instance().GetIntSetting(wxString::Format("CHECK_COL%i_WIDTH", COL_NUMBER), 70));
    lst.InsertColumn(COL_PAYEE_STR, "   " + _("Payee"), wxLIST_FORMAT_LEFT
        , Model_Setting::instance().GetIntSetting(wxString::Format("CHECK_COL%i_WIDTH", COL_PAYEE_STR), 150));
    lst.InsertColumn(COL_STATUS, _("Status"), wxLIST_FORMAT_LEFT
        , Model_Setting::instance().GetIntSetting(wxString::Format("CHECK_COL%i_WIDTH", COL_STATUS), -2));
    lst.InsertColumn(COL_CATEGORY, _("Category"), wxLIST_FORMAT_LEFT
        , Model_Setting::instance().GetIntSetting(wxString::Format("CHECK_COL%i_WIDTH", COL_CATEGORY), 150));
    lst.InsertColumn(COL_WITHDRAWAL, _("Withdrawal"), wxLIST_FORMAT_RIGHT
        , Model_Setting::instance().GetIntSetting(wxString::Format("CHECK_COL%i_WIDTH", COL_WITHDRAWAL), -2));
    lst.InsertColumn(COL_DEPOSIT, _("Deposit"), wxLIST_FORMAT_RIGHT
        , Model_Setting::instance().GetIntSetting(wxString::Format("CHECK_COL%i_WIDTH", COL_DEPOSIT), -2));
    lst.InsertColumn(COL_BALANCE, _("Balance"), wxLIST_FORMAT_RIGHT
        , Model_Setting::instance().GetIntSetting(wxString::Format("CHECK_COL%i_WIDTH", COL_BALANCE), -2));
    lst.InsertColumn(COL_NOTES, _("Notes"), wxLIST_FORMAT_LEFT
        , Model_Setting::instance().GetIntSetting(wxString::Format("CHECK_COL%i_WIDTH", COL_NOTES), 250));
}

void TransactionListCtrl::OnListItemSelected(wxListEvent& event)
{
    m_selectedIndex = event.GetIndex();
    m_cp->updateExtraTransactionData(m_selectedIndex);
    topItemIndex_ = GetTopItem() + GetCountPerPage() - 1;

    if (m_cp->m_listCtrlAccount->GetSelectedItemCount()>1)
        m_cp->btnEdit_->Enable(false);

    m_cp->m_listCtrlAccount->m_selectedID = m_cp->m_trans[m_selectedIndex].TRANSID;
}
//----------------------------------------------------------------------------

void TransactionListCtrl::OnListItemDeselected(wxListEvent& /*event*/)
{
//    long deselected = event.GetIndex();

    m_selectedIndex = -1;
    m_cp->updateExtraTransactionData(m_selectedIndex);

}

void TransactionListCtrl::OnItemResize(wxListEvent& event)
{
    int i = event.GetColumn();
    wxString parameter_name = wxString::Format("CHECK_COL%d_WIDTH", i);
    int current_width = m_cp->m_listCtrlAccount->GetColumnWidth(i);
    Model_Setting::instance().Set(parameter_name, current_width);
}

void TransactionListCtrl::OnListLeftClick(wxMouseEvent& event)
{
    if (m_selectedIndex > -1)
    {
        if (m_cp->m_listCtrlAccount->GetItemState(m_selectedIndex, wxLIST_STATE_SELECTED) == 0)
            m_cp->updateExtraTransactionData(-1);
    }
    event.Skip();
}

void TransactionListCtrl::OnListRightClick(wxMouseEvent& event)
{
    long selectedIndex = m_selectedIndex;
    if (m_selectedIndex > -1)
    {
        if (m_cp->m_listCtrlAccount->GetItemState(m_selectedIndex, wxLIST_STATE_SELECTED) == 0)
            selectedIndex = -1;
    }

    bool hide_menu_item = (selectedIndex < 0);
    bool type_transfer = false;
    bool have_category = false;
    if (m_selectedIndex > -1)
    {
        const Model_Checking::Full_Data& tran = m_cp->m_trans.at(m_selectedIndex);
        if (Model_Checking::type(tran) == Model_Checking::TRANSFER)
            type_transfer = true;
        if (tran.CATEGID > -1)
            have_category = true;
    }
    wxMenu menu;
    menu.Append(MENU_TREEPOPUP_NEW2, _("&New Transaction"));

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

    menu.AppendSeparator();

    wxString menu_item_label = showDeletedTransactions_ ? _("Hide Deleted (Void)") : _("Show Deleted (Void)");
    menu.AppendCheckItem(MENU_TREEPOPUP_SHOWTRASH, menu_item_label);

    wxMenu* subGlobalOpMenuDelete = new wxMenu();
    subGlobalOpMenuDelete->Append(MENU_TREEPOPUP_DELETE2, _("&Delete Transaction"));
    if (hide_menu_item) subGlobalOpMenuDelete->Enable(MENU_TREEPOPUP_DELETE2, false);
    subGlobalOpMenuDelete->AppendSeparator();
    subGlobalOpMenuDelete->Append(MENU_TREEPOPUP_DELETE_VIEWED, _("Delete all transactions in current view"));
    subGlobalOpMenuDelete->Append(MENU_TREEPOPUP_DELETE_FLAGGED, _("Delete Viewed \"Follow Up\" Trans."));
    menu.Append(MENU_TREEPOPUP_DELETE2, _("&Delete "), subGlobalOpMenuDelete);

    menu.AppendSeparator();

    menu.Append(MENU_TREEPOPUP_MARKRECONCILED, _("Mark As &Reconciled"));
    if (hide_menu_item) menu.Enable(MENU_TREEPOPUP_MARKRECONCILED, false);
    menu.Append(MENU_TREEPOPUP_MARKUNRECONCILED, _("Mark As &Unreconciled"));
    if (hide_menu_item) menu.Enable(MENU_TREEPOPUP_MARKUNRECONCILED, false);
    menu.Append(MENU_TREEPOPUP_MARKVOID, _("Mark As &Void"));
    if (hide_menu_item) menu.Enable(MENU_TREEPOPUP_MARKVOID, false);
    menu.Append(MENU_TREEPOPUP_MARK_ADD_FLAG_FOLLOWUP, _("Mark For &Followup"));
    if (hide_menu_item) menu.Enable(MENU_TREEPOPUP_MARK_ADD_FLAG_FOLLOWUP, false);
    menu.Append(MENU_TREEPOPUP_MARKDUPLICATE, _("Mark As &Duplicate"));
    if (hide_menu_item) menu.Enable(MENU_TREEPOPUP_MARKDUPLICATE, false);
    menu.AppendSeparator();

    wxMenu* subGlobalOpMenu = new wxMenu();
    subGlobalOpMenu->Append(MENU_TREEPOPUP_MARKRECONCILED_ALL, _("as Reconciled"));
    subGlobalOpMenu->Append(MENU_TREEPOPUP_MARKUNRECONCILED_ALL, _("as Unreconciled"));
    subGlobalOpMenu->Append(MENU_TREEPOPUP_MARKVOID_ALL, _("as Void"));
    subGlobalOpMenu->Append(MENU_TREEPOPUP_MARK_ADD_FLAG_FOLLOWUP_ALL, _("as needing Followup"));
    subGlobalOpMenu->Append(MENU_TREEPOPUP_MARKDUPLICATE_ALL, _("as Duplicate"));
    menu.Append(MENU_SUBMENU_MARK_ALL, _("Mark all being viewed"), subGlobalOpMenu);

    PopupMenu(&menu, event.GetPosition());
}
//----------------------------------------------------------------------------
void TransactionListCtrl::OnShowChbClick(wxCommandEvent& /*event*/)
{
    showDeletedTransactions_ = !showDeletedTransactions_;
    Model_Setting::instance().Set("SHOW_DELETED_TRANS", showDeletedTransactions_);
    refreshVisualList(m_selectedID);
}

void TransactionListCtrl::OnMarkTransaction(wxCommandEvent& event)
{
    int evt = event.GetId();
    wxString status = "";
    if (evt ==  MENU_TREEPOPUP_MARKRECONCILED)             status = "R";
    else if (evt == MENU_TREEPOPUP_MARKUNRECONCILED)       status = "";
    else if (evt == MENU_TREEPOPUP_MARKVOID)               status = "V";
    else if (evt == MENU_TREEPOPUP_MARK_ADD_FLAG_FOLLOWUP) status = "F";
    else if (evt == MENU_TREEPOPUP_MARKDUPLICATE)          status = "D";
    else wxASSERT(false);

    bool bVoidTransaction = false;
    if (status == "V")
        bVoidTransaction = true;
    Model_Checking::Data *trx = Model_Checking::instance().get(m_cp->m_trans[m_selectedIndex].TRANSID);
    if (trx)
    {
        if (trx->STATUS == "V")
            bVoidTransaction = true;
        m_cp->m_trans[m_selectedIndex].STATUS = status;
        trx->STATUS = status;
        Model_Checking::instance().save(trx);
    }

    if ((m_cp->transFilterActive_ && m_cp->transFilterDlg_->getStatusCheckBox()) || bVoidTransaction)
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
    else if (status == "M")
    {
        wxMessageDialog msgDlg(this
            ,_("Do you really want to delete all the \"Follow Up\" transactions shown?")
            , _("Confirm Transaction Deletion")
            , wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION);
        if (msgDlg.ShowModal() == wxID_YES)
        {
            m_cp->DeleteFlaggedTransactions("F");
        }
    }
    else
    {
        for (auto& tran: m_cp->m_trans)
            tran.STATUS = status;
        Model_Checking::instance().save(m_cp->m_trans);
    }

    refreshVisualList();
}
//----------------------------------------------------------------------------

void TransactionListCtrl::OnColClick(wxListEvent& event)
{
    if(0 > event.GetColumn() || event.GetColumn() >= COL_MAX) return;

    /* Clear previous column image */
    setColumnImage(m_sortCol, -1);

    if (g_sortcol == event.GetColumn()) m_asc = !m_asc; // toggle sort order
    g_asc = m_asc;

    m_sortCol = toEColumn(event.GetColumn());
    g_sortcol = m_sortCol;

    setColumnImage(m_sortCol, m_asc ? ICON_ASC : ICON_DESC);
    Model_Setting::instance().Set("CHECK_ASC", (g_asc ? 1 : 0));
    Model_Setting::instance().Set("CHECK_SORT_COL", g_sortcol);

    m_cp->m_listCtrlAccount->refreshVisualList(m_selectedID);

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
    if(column == COL_DATE)
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
        if (user_colour_id == 1)      return (wxListItemAttr*)&m_attr11;
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
    this->OnCopy(event);
    this->OnPaste(event);
    this->OnEditTransaction(event);
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
    if (Model_Checking::type(copy) != Model_Checking::TRANSFER) copy->ACCOUNTID = m_cp->m_AccountID;
    int transactionID = Model_Checking::instance().save(copy);

    Model_Splittransaction::Data_Set copy_split;
    for (const auto& split_item : Model_Checking::splittransaction(tran))
    {
        Model_Splittransaction::Data *copy_split_item = Model_Splittransaction::instance().clone(&split_item);
        copy_split_item->TRANSID = transactionID;
        copy_split.push_back(*copy_split_item);
    }
    Model_Splittransaction::instance().save(copy_split);

    return transactionID;
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
    else if ((wxGetKeyState(WXK_DELETE)|| wxGetKeyState(WXK_NUMPAD_DELETE)) && status != "V")
    {
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_TREEPOPUP_MARKVOID);
        OnMarkTransaction(evt);
    }
    else if (wxGetKeyState(WXK_DELETE)|| wxGetKeyState(WXK_NUMPAD_DELETE))
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
        , wxYES_NO | wxNO_DEFAULT | wxICON_ERROR);

    if (msgDlg.ShowModal() == wxID_YES)
    {
        long x = 0;
        for (const auto& i : m_cp->m_trans)
        {
            long transID = i.TRANSID;
            if (m_cp->m_listCtrlAccount->GetItemState(x, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED)
            {
                // remove also removes any split transactions
                Model_Checking::instance().remove(transID);
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

void TransactionListCtrl::OnNewTransaction(wxCommandEvent& /*event*/)
{
    mmTransDialog dlg(this, m_cp->m_AccountID, 0);
    if (dlg.ShowModal() == wxID_OK)
    {
        refreshVisualList(dlg.getTransactionID());
    }
}
//----------------------------------------------------------------------------

void TransactionListCtrl::OnSetUserColour(wxCommandEvent& event)
{
    int user_colour_id = event.GetId();
    user_colour_id -= MENU_ON_SET_UDC0;
    wxLogDebug("id: %i", user_colour_id);

    Model_Checking::Data * transaction = Model_Checking::instance().get(m_selectedID);
    if (transaction)
    {
        transaction->FOLLOWUPID = user_colour_id;
        Model_Checking::instance().save(transaction);
        m_cp->m_trans[m_selectedIndex].FOLLOWUPID = user_colour_id;
        RefreshItems(m_selectedIndex, m_selectedIndex);
    }
}
//----------------------------------------------------------------------------

void TransactionListCtrl::refreshVisualList(int trans_id)
{
    this->SetEvtHandlerEnabled(false);
    Hide();

    // decide whether top or down icon needs to be shown
    setColumnImage(g_sortcol, g_asc ? ICON_ASC : ICON_DESC);
    m_cp->filterTable();
    m_cp->sortTable();
    SetItemCount(m_cp->m_trans.size());
    m_cp->markSelectedTransaction(trans_id);

    if (topItemIndex_ >= (long)m_cp->m_trans.size())
        topItemIndex_ = g_asc ? (long)m_cp->m_trans.size() - 1 : 0;
    if (m_selectedIndex > (long)m_cp->m_trans.size() - 1) m_selectedIndex = -1;
    if (topItemIndex_ < m_selectedIndex) topItemIndex_ = m_selectedIndex;

    Refresh();

    if (m_selectedIndex >= 0 && !m_cp->m_trans.empty())
    {
        SetItemState(m_selectedIndex, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        SetItemState(m_selectedIndex, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
        if (topItemIndex_ < 0 || (topItemIndex_ - m_selectedIndex) > GetCountPerPage())
            topItemIndex_ = m_selectedIndex;
        EnsureVisible(topItemIndex_);
    }

    Show();
    m_cp->setAccountSummary();
    m_cp->updateExtraTransactionData(m_selectedIndex);
    this->SetEvtHandlerEnabled(true);
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
    if (m_selectedIndex < 0) return;

    if (m_cp->m_trans[m_selectedIndex].CATEGID < 0)
        m_cp->DisplaySplitCategories(m_cp->m_trans[m_selectedIndex].TRANSID);

}

//----------------------------------------------------------------------------
void TransactionListCtrl::OnListItemActivated(wxListEvent& /*event*/)
{
    if (m_selectedIndex < 0) return;

    wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_TREEPOPUP_EDIT2);
    AddPendingEvent(evt);
}

//----------------------------------------------------------------------------
wxString mmCheckingPanel::getMiniInfoStr(int selIndex) const
{
    //TODO: Some additional info may be helpfull
    const Model_Checking::Full_Data& tran = this->m_trans.at(selIndex);
    wxDateTime date = Model_Checking::TRANSDATE(tran);
    wxString infoStr = wxGetTranslation(date.GetWeekDayName(date.GetWeekDay()));
    return infoStr;
}
