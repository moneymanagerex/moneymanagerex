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
#include "transdialog.h"
#include "splittransactionsdialog.h"
#include "mmex.h"
#include "constants.h"
#include "mmCurrencyFormatter.h"
#include "validators.h"
#include "model/Model_Infotable.h"
#include "model/Model_Setting.h"
//----------------------------------------------------------------------------
#include <wx/srchctrl.h>
#include <algorithm>
//----------------------------------------------------------------------------

enum EColumn
{
    COL_DATE_OR_TRANSACTION_ID = 0,
    COL_TRANSACTION_NUMBER,
    COL_PAYEE_STR,
    COL_STATUS,
    COL_CATEGORY,
    COL_WITHDRAWAL,
    COL_DEPOSIT,
    COL_BALANCE,
    COL_NOTES,
    COL_MAX, // number of columns
    COL_DEF_SORT = COL_DATE_OR_TRANSACTION_ID
};
//----------------------------------------------------------------------------

enum EIcons
{
    ICON_RECONCILED,
    ICON_VOID,
    ICON_FOLLOWUP,
    ICON_NONE,
    ICON_DESC,
    ICON_ASC,
    ICON_DUPLICATE
};
//----------------------------------------------------------------------------
EColumn g_sortcol = COL_DEF_SORT; // index of column to sort
bool g_asc = true; // asc\desc sorting
//----------------------------------------------------------------------------

EColumn toEColumn(long col)
{
    EColumn res = COL_DEF_SORT;
    if (col >= 0 && col < COL_MAX) res = static_cast<EColumn>(col);

    return res;
}
//----------------------------------------------------------------------------

/*
    Adds columns to list controls and setup their initial widths.
*/
void createColumns(wxListCtrl &lst)
{
    const wxString def_data[3*COL_MAX] =
    {
        wxString(wxTRANSLATE("Date")).Prepend("      "), "80", "L",
        wxTRANSLATE("Number"), "-2", "L",
        wxString(wxTRANSLATE("Payee")).Prepend("   "), "150", "L",
        wxTRANSLATE("Status"), "-2", "L",
        wxTRANSLATE("Category"), "-2", "L",
        wxTRANSLATE("Withdrawal"), "-2", "R",
        wxTRANSLATE("Deposit"), "-2", "R",
        wxTRANSLATE("Balance"), "-2", "R",
        wxTRANSLATE("Notes"), "200", "L"
    };

    for (int i = 0; i < COL_MAX; ++i)
    {
        const wxString col_name = wxGetTranslation(def_data[3*i+0]);
        const wxString def_width = def_data[3*i+1];
        const wxString def_format = def_data[3*i+2];

        wxString name = wxString::Format("CHECK_COL%d_WIDTH", i);
        wxString val = Model_Setting::instance().GetStringSetting(name, def_width);
        long width = -1;
        int format = wxLIST_FORMAT_RIGHT;
        if (def_format == "L") format = wxLIST_FORMAT_LEFT;
        lst.InsertColumn((long)i, col_name, format);
        if (val.ToLong(&width)) lst.SetColumnWidth(i, (int)width);
    }
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
class TransactionListCtrl : public wxListCtrl
{
public:
    TransactionListCtrl(mmCheckingPanel *cp, wxWindow *parent
        , const wxWindowID id, const wxPoint& pos,const wxSize& size, long style);

    bool getSortOrder() const { return m_asc; }
    EColumn getSortColumn() const { return m_sortCol; }

    void setSortOrder(bool asc) { m_asc = asc; }
    void setSortColumn(EColumn col) { m_sortCol = col; }

    void setColumnImage(EColumn col, int image);

    void OnNewTransaction(wxCommandEvent& event);
    void OnDeleteTransaction(wxCommandEvent& event);
    void OnEditTransaction(wxCommandEvent& event);
    void OnDuplicateTransaction(wxCommandEvent& event);
    void OnMoveTransaction(wxCommandEvent& event);
    /// Displays the split categories for the selected transaction
    void OnViewSplitTransaction(wxCommandEvent& event);
    long m_selectedIndex;
    long m_selectedForCopy;
    long m_selectedID;
    void refreshVisualList(int trans_id = -1);

private:
    DECLARE_NO_COPY_CLASS(TransactionListCtrl)
    DECLARE_EVENT_TABLE()

    mmCheckingPanel *m_cp;

    wxListItemAttr m_attr1; // style1
    wxListItemAttr m_attr2; // style2
    wxListItemAttr m_attr3; // style, for future dates
    wxListItemAttr m_attr4; // style, for future dates

    EColumn m_sortCol;
    bool m_asc;

    /* required overrides for virtual style list control */
    virtual wxString OnGetItemText(long item, long column) const;
    virtual int OnGetItemColumnImage(long item, long column) const;
    virtual wxListItemAttr *OnGetItemAttr(long item) const;

    void OnListRightClick(wxMouseEvent& event);
    void OnListLeftClick(wxMouseEvent& event);
    void OnItemResize(wxListEvent& event);
    void OnListItemSelected(wxListEvent& event);
    void OnListItemDeselected(wxListEvent& event);
    void OnListItemActivated(wxListEvent& event);
    void OnMarkTransaction(wxCommandEvent& event);
    void OnMarkAllTransactions(wxCommandEvent& event);
    void OnListKeyDown(wxListEvent& event);
    void OnChar(wxKeyEvent& event);
    int OnMarkTransactionDB(const wxString& status);
    void OnCopy(wxCommandEvent& WXUNUSED(event));
    void OnPaste(wxCommandEvent& WXUNUSED(event));

    /* Sort Columns */
    void OnColClick(wxListEvent& event);

    /// Called when moving a transaction to a new account.
    int DestinationAccountID();
    long topItemIndex_;
};
//----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(mmCheckingPanel, wxPanel)
    EVT_BUTTON(wxID_NEW,         mmCheckingPanel::OnNewTransaction)
    EVT_BUTTON(wxID_EDIT,        mmCheckingPanel::OnEditTransaction)
    EVT_BUTTON(wxID_DELETE,      mmCheckingPanel::OnDeleteTransaction)
    EVT_BUTTON(wxID_DUPLICATE,    mmCheckingPanel::OnDuplicateTransaction)
    EVT_MENU(wxID_ANY, mmCheckingPanel::OnViewPopupSelected)
    EVT_SEARCHCTRL_SEARCH_BTN(wxID_FIND, mmCheckingPanel::OnSearchTxtEntered)
    EVT_TEXT_ENTER(wxID_FIND, mmCheckingPanel::OnSearchTxtEntered)
END_EVENT_TABLE()
//----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(TransactionListCtrl, wxListCtrl)

    EVT_LIST_ITEM_SELECTED(ID_PANEL_CHECKING_LISTCTRL_ACCT, TransactionListCtrl::OnListItemSelected)
    EVT_LIST_ITEM_DESELECTED(ID_PANEL_CHECKING_LISTCTRL_ACCT, TransactionListCtrl::OnListItemDeselected)
    EVT_LIST_ITEM_ACTIVATED(ID_PANEL_CHECKING_LISTCTRL_ACCT, TransactionListCtrl::OnListItemActivated)
    //EVT_LIST_ITEM_RIGHT_CLICK(ID_PANEL_CHECKING_LISTCTRL_ACCT, TransactionListCtrl::OnItemRightClick)
    EVT_RIGHT_DOWN(TransactionListCtrl::OnListRightClick)
    EVT_LEFT_DOWN(TransactionListCtrl::OnListLeftClick)
    EVT_LIST_COL_END_DRAG(ID_PANEL_CHECKING_LISTCTRL_ACCT, TransactionListCtrl::OnItemResize)
    EVT_LIST_COL_CLICK(ID_PANEL_CHECKING_LISTCTRL_ACCT, TransactionListCtrl::OnColClick)
    EVT_LIST_KEY_DOWN(ID_PANEL_CHECKING_LISTCTRL_ACCT,  TransactionListCtrl::OnListKeyDown)

    EVT_MENU(MENU_TREEPOPUP_MARKRECONCILED,   TransactionListCtrl::OnMarkTransaction)
    EVT_MENU(MENU_TREEPOPUP_MARKUNRECONCILED, TransactionListCtrl::OnMarkTransaction)
    EVT_MENU(MENU_TREEPOPUP_MARKVOID,         TransactionListCtrl::OnMarkTransaction)
    EVT_MENU(MENU_TREEPOPUP_MARK_ADD_FLAG_FOLLOWUP, TransactionListCtrl::OnMarkTransaction)
    EVT_MENU(MENU_TREEPOPUP_MARKDUPLICATE,          TransactionListCtrl::OnMarkTransaction)

    EVT_MENU(MENU_TREEPOPUP_MARKRECONCILED_ALL,         TransactionListCtrl::OnMarkAllTransactions)
    EVT_MENU(MENU_TREEPOPUP_MARKUNRECONCILED_ALL,       TransactionListCtrl::OnMarkAllTransactions)
    EVT_MENU(MENU_TREEPOPUP_MARKVOID_ALL,               TransactionListCtrl::OnMarkAllTransactions)
    EVT_MENU(MENU_TREEPOPUP_MARK_ADD_FLAG_FOLLOWUP_ALL, TransactionListCtrl::OnMarkAllTransactions)
    EVT_MENU(MENU_TREEPOPUP_MARKDUPLICATE_ALL,          TransactionListCtrl::OnMarkAllTransactions)
    EVT_MENU(MENU_TREEPOPUP_DELETE_VIEWED,          TransactionListCtrl::OnMarkAllTransactions)
    EVT_MENU(MENU_TREEPOPUP_DELETE_FLAGGED,          TransactionListCtrl::OnMarkAllTransactions)

    EVT_MENU(MENU_TREEPOPUP_NEW,                TransactionListCtrl::OnNewTransaction)
    EVT_MENU(MENU_TREEPOPUP_DELETE,             TransactionListCtrl::OnDeleteTransaction)
    EVT_MENU(MENU_TREEPOPUP_EDIT,               TransactionListCtrl::OnEditTransaction)
    EVT_MENU(MENU_TREEPOPUP_MOVE,               TransactionListCtrl::OnMoveTransaction)

    EVT_MENU(MENU_ON_COPY_TRANSACTION,      TransactionListCtrl::OnCopy)
    EVT_MENU(MENU_ON_PASTE_TRANSACTION,     TransactionListCtrl::OnPaste)
    EVT_MENU(MENU_ON_NEW_TRANSACTION,       TransactionListCtrl::OnNewTransaction)
    EVT_MENU(MENU_ON_DUPLICATE_TRANSACTION, TransactionListCtrl::OnDuplicateTransaction)

    EVT_MENU(MENU_TREEPOPUP_VIEW_SPLIT_CATEGORIES, TransactionListCtrl::OnViewSplitTransaction)

    EVT_CHAR(TransactionListCtrl::OnChar)

END_EVENT_TABLE();

//----------------------------------------------------------------------------

mmCheckingPanel::mmCheckingPanel(
    mmCoreDB* core, int accountID,
    wxWindow *parent, wxWindowID winid,
    const wxPoint& pos, const wxSize& size, long style, const wxString& name
    )
    : mmPanelBase(core)
    , filteredBalance_(0.0)
    , m_listCtrlAccount()
    , m_AccountID(accountID)
{
    wxASSERT(core_);
    Create(parent, winid, pos, size, style, name);
}
//----------------------------------------------------------------------------

/*
    We cannon use OnClose() event because this class deletes
    via DestroyChildren() of its parent.
*/
mmCheckingPanel::~mmCheckingPanel()
{
    wxGetApp().m_frame->SetCheckingAccountPageInactive();
}
//----------------------------------------------------------------------------

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
    transFilterDlg_    = new mmFilterTransactionsDialog(core_, this);
    initViewTransactionsHeader();
    initFilterSettings();

    m_listCtrlAccount->refreshVisualList();
    this->windowsFreezeThaw();

    return true;
}

void mmCheckingPanel::sortTable()
{
    std::sort(this->m_trans.begin(), this->m_trans.end()); // default sorter
    if (g_sortcol == COL_DATE_OR_TRANSACTION_ID)
    {
        std::stable_sort(this->m_trans.begin(), this->m_trans.end()
            , [] (const mmBankTransaction* x, const mmBankTransaction* y)
            { return x->date_ < y->date_; });
    }
    else if (g_sortcol == COL_TRANSACTION_NUMBER)
    {
        std::stable_sort(this->m_trans.begin(), this->m_trans.end()
            , [] (const mmBankTransaction* x, const mmBankTransaction* y)
            { return x->transNum_ < y->transNum_; });
    }
    else if (g_sortcol == COL_PAYEE_STR)
    {
        std::stable_sort(this->m_trans.begin(), this->m_trans.end()
            , [] (const mmBankTransaction* x, const mmBankTransaction* y)
            { return x->payeeStr_ < y->payeeStr_; });
    }
    else if (g_sortcol == COL_STATUS)
    {
        std::stable_sort(this->m_trans.begin(), this->m_trans.end()
            , [] (const mmBankTransaction* x, const mmBankTransaction* y)
            { return x->status_ < y->status_; });
    }
    else if (g_sortcol == COL_CATEGORY)
    {
        std::stable_sort(this->m_trans.begin(), this->m_trans.end()
            , [] (const mmBankTransaction* x, const mmBankTransaction* y)
            { return x->fullCatStr_ < y->fullCatStr_; });
    }
    else if (g_sortcol == COL_WITHDRAWAL)
    {
        std::stable_sort(this->m_trans.begin(), this->m_trans.end()
            , [] (const mmBankTransaction* x, const mmBankTransaction* y)
        { return x->withdrawal_amt_ < y->withdrawal_amt_; });
    }
    else if (g_sortcol == COL_DEPOSIT)
    {
        std::stable_sort(this->m_trans.begin(), this->m_trans.end()
            , [] (const mmBankTransaction* x, const mmBankTransaction* y)
        { return x->deposit_amt_ < y->deposit_amt_; });
    }
    else if (g_sortcol == COL_BALANCE)
    {
        std::stable_sort(this->m_trans.begin(), this->m_trans.end()
            , [] (const mmBankTransaction* x, const mmBankTransaction* y)
            { return x->balance_ < y->balance_; });
    }
    else if (g_sortcol == COL_NOTES)
    {
        std::stable_sort(this->m_trans.begin(), this->m_trans.end()
            , [] (const mmBankTransaction* x, const mmBankTransaction* y)
            { return x->notes_ < y->notes_; });
    }

    if (!g_asc) std::reverse(this->m_trans.begin(), this->m_trans.end());
}

void mmCheckingPanel::filterTable()
{
    if (transFilterActive_ &&transFilterDlg_->somethingSelected())
    {
        std::vector<mmBankTransaction*>::iterator iter;
        for (iter = m_trans.begin(); iter != m_trans.end(); )
        {
            if (transFilterDlg_->getAccountCheckBox()
                    && transFilterDlg_->getAccountID() != (*iter)->toAccountID_)
                iter = m_trans.erase(iter);
            else if (transFilterDlg_->getDateRangeCheckBox()
                    && (transFilterDlg_->getFromDateCtrl().GetDateOnly() > (*iter)->date_.GetDateOnly() 
                    && transFilterDlg_->getToDateControl().GetDateOnly() < (*iter)->date_.GetDateOnly()))
                iter = m_trans.erase(iter);
            else if (transFilterDlg_->getPayeeCheckBox()
                    && transFilterDlg_->userPayeeStr() != (*iter)->payeeStr_)
                iter = m_trans.erase(iter);
            else if (transFilterDlg_->getCategoryCheckBox()
                    && !(*iter)->containsCategory(transFilterDlg_->getCategoryID()
                    ,transFilterDlg_->getSubCategoryID(), transFilterDlg_->getSubCategoryID() < 0))
                iter = m_trans.erase(iter);
            else if (transFilterDlg_->getStatusCheckBox()
                    && transFilterDlg_->getStatus() != (*iter)->status_)
                iter = m_trans.erase(iter);
            else if (transFilterDlg_->getTypeCheckBox()
                    && !transFilterDlg_->getType().Contains((*iter)->transType_))
                iter = m_trans.erase(iter);
            else if (transFilterDlg_->getAmountRangeCheckBox()
                    && !(transFilterDlg_->getAmountMin() <= (*iter)->amt_ && transFilterDlg_->getAmountMax() >= (*iter)->amt_))
                iter = m_trans.erase(iter);
            else if (transFilterDlg_->getNumberCheckBox()
                    && transFilterDlg_->getNumber().Trim().Lower() != (*iter)->transNum_.Lower())
                iter = m_trans.erase(iter);
            else if (transFilterDlg_->getNotesCheckBox()
                    && !(*iter)->notes_.Lower().Matches(transFilterDlg_->getNotes().Trim().Lower()))
                iter = m_trans.erase(iter);
            else
                ++iter;
        }
    }
    else
    {
        std::vector<mmBankTransaction*>::iterator iter;
        for (iter = m_trans.begin(); iter != m_trans.end(); )
        {
            //TODO: What about future dates?
            if (quickFilterBeginDate_ <= (*iter)->date_.GetDateOnly() 
                    && quickFilterEndDate_ >= (*iter)->date_.GetDateOnly())
                ++iter;
            else
                iter = m_trans.erase(iter);
        }
    }
}

void mmCheckingPanel::markSelectedTransaction(int trans_id)
{
    long i = 0;
    for (const auto & pTrans : m_trans)
    {
        if (trans_id == pTrans->transactionID() && trans_id > 0) {
            m_listCtrlAccount->m_selectedIndex = i;
            break;
        }
        ++i;
    }

    if (m_trans.size() > 0 && m_listCtrlAccount->m_selectedIndex < 0)
    {
        if (g_asc)
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
            int id = MENU_VIEW_ALLTRANSACTIONS;
            for (const auto& i : DATE_PRESETTINGS)
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

void mmCheckingPanel::initVirtualListControl(int /*trans_id*/)
{
    //Initialization
    account_balance_ = 0.0, reconciled_balance_ = 0.0;
    core_->bTransactionList_.LoadAccountTransactions(m_AccountID, account_balance_, reconciled_balance_);
    filteredBalance_ = 0.0;
    // clear everything
    m_trans = core_->bTransactionList_.accountTransactions_;
    m_listCtrlAccount->DeleteAllItems();

    setAccountSummary();
    // decide whether top or down icon needs to be shown
    m_listCtrlAccount->setColumnImage(g_sortcol, g_asc ? ICON_ASC : ICON_DESC);
    sortTable();
    filterTable();
    m_listCtrlAccount->SetItemCount(m_trans.size());
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
    wxPanel* headerPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition,
        wxDefaultSize, wxNO_BORDER|wxTAB_TRAVERSAL );
    itemBoxSizer9->Add(headerPanel, flags);

    wxBoxSizer* itemBoxSizerVHeader = new wxBoxSizer(wxVERTICAL);
    headerPanel->SetSizer(itemBoxSizerVHeader);

    wxGridSizer* itemBoxSizerVHeader2 = new wxGridSizer(0,1,5,20);
    itemBoxSizerVHeader->Add(itemBoxSizerVHeader2);

    header_text_ = new wxStaticText( headerPanel, wxID_STATIC, "");
    int font_size = this->GetFont().GetPointSize() + 2;
    header_text_->SetFont(wxFont(font_size, wxSWISS, wxNORMAL, wxBOLD, FALSE, ""));
    itemBoxSizerVHeader2->Add(header_text_, flags);

    wxBoxSizer* itemBoxSizerHHeader2 = new wxBoxSizer(wxHORIZONTAL);
    wxFlexGridSizer* itemFlexGridSizerHHeader2 = new wxFlexGridSizer(5,1,1);
    itemBoxSizerVHeader2->Add(itemBoxSizerHHeader2);
    itemBoxSizerHHeader2->Add(itemFlexGridSizerHHeader2);

    wxBitmap itemStaticBitmap(rightarrow_xpm);
    bitmapMainFilter_ = new wxStaticBitmap( headerPanel, wxID_ANY,
        itemStaticBitmap);
    itemFlexGridSizerHHeader2->Add(bitmapMainFilter_, flags);
    bitmapMainFilter_->Connect(wxID_ANY, wxEVT_RIGHT_DOWN
        , wxMouseEventHandler(mmCheckingPanel::OnFilterResetToViewAll), NULL, this);
    bitmapMainFilter_->Connect(wxID_ANY, wxEVT_LEFT_DOWN
        , wxMouseEventHandler(mmCheckingPanel::OnMouseLeftDown), NULL, this);

    stxtMainFilter_ = new wxStaticText( headerPanel, wxID_ANY, "", wxDefaultPosition, wxSize(200, -1));
    itemFlexGridSizerHHeader2->Add(stxtMainFilter_, flags);

    itemFlexGridSizerHHeader2->AddSpacer(20);

    bitmapTransFilter_ = new wxStaticBitmap( headerPanel, ID_PANEL_CHECKING_STATIC_BITMAP_FILTER,
        itemStaticBitmap);
    itemFlexGridSizerHHeader2->Add(bitmapTransFilter_, flags);
    bitmapTransFilter_->Connect(wxID_ANY, wxEVT_LEFT_DOWN
        , wxMouseEventHandler(mmCheckingPanel::OnFilterTransactions), NULL, this);
    bitmapTransFilter_->Connect(wxID_ANY, wxEVT_RIGHT_DOWN
        , wxMouseEventHandler(mmCheckingPanel::OnFilterTransactions), NULL, this);

    statTextTransFilter_ = new wxStaticText( headerPanel, wxID_ANY,
        _("Transaction Filter"));
    itemFlexGridSizerHHeader2->Add(statTextTransFilter_, flags);
    SetTransactionFilterState(false);

    wxStaticText* itemStaticText12 = new wxStaticText( headerPanel,
            ID_PANEL_CHECKING_STATIC_BALHEADER1, "$", wxDefaultPosition, wxSize(120,-1));
    wxStaticText* itemStaticText14 = new wxStaticText( headerPanel,
            ID_PANEL_CHECKING_STATIC_BALHEADER2, "$", wxDefaultPosition, wxSize(120,-1));
    wxStaticText* itemStaticText16 = new wxStaticText( headerPanel,
            ID_PANEL_CHECKING_STATIC_BALHEADER3, "$", wxDefaultPosition, wxSize(120,-1));
    wxStaticText* itemStaticText17 = new wxStaticText( headerPanel,
            ID_PANEL_CHECKING_STATIC_BALHEADER4, _("Displayed Bal: "));
    wxStaticText* itemStaticText18 = new wxStaticText( headerPanel,
            ID_PANEL_CHECKING_STATIC_BALHEADER5, "$", wxDefaultPosition, wxSize(120,-1));

    wxFlexGridSizer* balances_header = new wxFlexGridSizer(0,8,5,10);
    itemBoxSizerVHeader->Add(balances_header);
    balances_header->Add(new wxStaticText(headerPanel, wxID_STATIC, _("Account Bal: ")));
    balances_header->Add(itemStaticText12);
    balances_header->Add(new wxStaticText(headerPanel,  wxID_STATIC, _("Reconciled Bal: ")));
    balances_header->Add(itemStaticText14);
    balances_header->Add(new wxStaticText(headerPanel, wxID_STATIC, _("Diff: ")));
    balances_header->Add(itemStaticText16);
    balances_header->Add(itemStaticText17);
    balances_header->Add(itemStaticText18);

    /* ---------------------- */

    wxSplitterWindow* itemSplitterWindow10 = new wxSplitterWindow( this,
        ID_SPLITTERWINDOW, wxDefaultPosition, wxSize(200, 200),
        wxSP_3DBORDER|wxSP_3DSASH|wxNO_BORDER );

    wxSize imageSize(16, 16);
    m_imageList.reset(new wxImageList(imageSize.GetWidth(), imageSize.GetHeight()));
    m_imageList->Add(wxImage(reconciled_xpm).Scale(16, 16));
    m_imageList->Add(wxImage(void_xpm).Scale(16, 16));
    m_imageList->Add(wxImage(flag_xpm).Scale(16, 16));
    m_imageList->Add(wxImage(unreconciled_xpm).Scale(16, 16));
    m_imageList->Add(wxImage(uparrow_xpm).Scale(16, 16));
    m_imageList->Add(wxImage(downarrow_xpm).Scale(16, 16));
    m_imageList->Add(wxImage(duplicate_xpm).Scale(16, 16));

    m_listCtrlAccount = new TransactionListCtrl( this, itemSplitterWindow10,
        ID_PANEL_CHECKING_LISTCTRL_ACCT, wxDefaultPosition, wxDefaultSize,
        wxLC_REPORT | wxLC_HRULES | wxLC_VRULES | wxLC_VIRTUAL | wxLC_SINGLE_SEL);

    m_listCtrlAccount->SetImageList(m_imageList.get(), wxIMAGE_LIST_SMALL);
    m_listCtrlAccount->setSortOrder(g_asc);
    m_listCtrlAccount->setSortColumn(g_sortcol);
    m_listCtrlAccount->SetFocus();

    createColumns(*m_listCtrlAccount);

    // load the global variables
    long val = COL_DEF_SORT;
    wxString strVal = Model_Setting::instance().GetStringSetting("CHECK_SORT_COL", wxString() << val);
    if (strVal.ToLong(&val)) g_sortcol = toEColumn(val);
    // --
    val = 1; // asc sorting default
    strVal = Model_Setting::instance().GetStringSetting("CHECK_ASC", wxString() << val);
    if (strVal.ToLong(&val)) g_asc = val != 0;

    // --
    m_listCtrlAccount->setSortColumn(g_sortcol);
    m_listCtrlAccount->setSortOrder(g_asc);
    m_listCtrlAccount->setColumnImage(m_listCtrlAccount->getSortColumn()
        , m_listCtrlAccount->getSortOrder() ? ICON_ASC : ICON_DESC); // asc\desc sort mark (arrow)

    wxPanel *itemPanel12 = new wxPanel(itemSplitterWindow10, ID_PANEL1
        , wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxTAB_TRAVERSAL);

    itemSplitterWindow10->SplitHorizontally(m_listCtrlAccount, itemPanel12);
    itemSplitterWindow10->SetMinimumPaneSize(100);
    itemSplitterWindow10->SetSashGravity(1.0);

    itemBoxSizer9->Add(itemSplitterWindow10, flagsExpand);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemPanel12->SetSizer(itemBoxSizer4);

    wxBoxSizer* itemButtonsSizer = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer4->Add(itemButtonsSizer, flags);

    btnNew_ = new wxButton(itemPanel12, wxID_NEW, _("&New"));
    btnNew_->SetToolTip(_("New Transaction"));
    itemButtonsSizer->Add(btnNew_, 0, wxRIGHT, 5);

    btnEdit_ = new wxButton(itemPanel12, wxID_EDIT, _("&Edit"));
    btnEdit_->SetToolTip(_("Edit selected transaction"));
    itemButtonsSizer->Add(btnEdit_, 0, wxRIGHT, 5);
    btnEdit_->Enable(false);

    btnDelete_ = new wxButton(itemPanel12, wxID_DELETE, _("&Delete"));
    btnDelete_->SetToolTip(_("Delete selected transaction"));
    itemButtonsSizer->Add(btnDelete_, 0, wxRIGHT, 5);
    btnDelete_->Enable(false);

    btnDuplicate_ = new wxButton(itemPanel12, wxID_DUPLICATE, _("D&uplicate"));
    btnDuplicate_->SetToolTip(_("Duplicate selected transaction"));
    itemButtonsSizer->Add(btnDuplicate_, 0, wxRIGHT, 5);
    btnDuplicate_->Enable(false);

    wxSearchCtrl* searchCtrl = new wxSearchCtrl(itemPanel12
        , wxID_FIND, wxEmptyString, wxDefaultPosition
        , wxSize(100, btnDuplicate_->GetSize().GetHeight())
        , wxTE_PROCESS_ENTER|wxTE_PROCESS_TAB, wxDefaultValidator, _("Search"));
    itemButtonsSizer->Add(searchCtrl, 0, wxCENTER, 1);
    searchCtrl->SetToolTip(_("Enter any string to find it in the nearest transaction notes"));

    //Infobar-mini
    info_panel_mini_ = new wxStaticText( itemPanel12, wxID_STATIC, "");
    itemButtonsSizer->Add(info_panel_mini_, 1, wxGROW|wxTOP|wxLEFT, 5);

    //Infobar
    info_panel_ = new wxStaticText( itemPanel12,
        wxID_STATIC, "", wxDefaultPosition, wxSize(200,-1), wxTE_MULTILINE|wxTE_WORDWRAP);
    itemBoxSizer4->Add(info_panel_, flagsExpand);
    //Show tips when no any transaction selected
    showTips();
}

//----------------------------------------------------------------------------
void mmCheckingPanel::setAccountSummary()
{
    mmAccount* pAccount = core_->accountList_.GetAccountSharedPtr(m_AccountID);
    core_->accountList_.getCurrencySharedPtr(m_AccountID)->loadCurrencySettings();

    header_text_->SetLabel(wxString::Format(_("Account View : %s"), pAccount->name_));

    bool show_displayed_balance_ = (transFilterActive_ || (currentView_ != VIEW_TRANS_ALL_STR));
    wxStaticText* header = (wxStaticText*)FindWindow(ID_PANEL_CHECKING_STATIC_BALHEADER1);
    header->SetLabel(CurrencyFormatter::float2Money(account_balance_));
    header = (wxStaticText*)FindWindow(ID_PANEL_CHECKING_STATIC_BALHEADER2);
    header->SetLabel(CurrencyFormatter::float2Money(reconciled_balance_));
    header = (wxStaticText*)FindWindow(ID_PANEL_CHECKING_STATIC_BALHEADER3);
    header->SetLabel(CurrencyFormatter::float2Money(account_balance_ - reconciled_balance_));
    header = (wxStaticText*)FindWindow(ID_PANEL_CHECKING_STATIC_BALHEADER4);
    header->SetLabel(show_displayed_balance_
        ? _("Displayed Bal: ")
        : "                                 ");
    header = (wxStaticText*)FindWindow(ID_PANEL_CHECKING_STATIC_BALHEADER5);
    header->SetLabel(show_displayed_balance_
        ? CurrencyFormatter::float2Money(filteredBalance_)
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
        info_panel_->SetLabel(wxString() << m_trans[selIndex]->notes_);
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
        showTips() ;
    }
}
//----------------------------------------------------------------------------
wxString mmCheckingPanel::getMiniInfoStr(int selIndex) const
{
    int accountId = m_trans[selIndex]->accountID_;
    int toaccountId = m_trans[selIndex]->toAccountID_;
    wxString intoaccStr = core_->accountList_.GetAccountName(toaccountId);
    wxString fromaccStr = core_->accountList_.GetAccountName(accountId);
    int basecurrencyid = Model_Infotable::instance().GetBaseCurrencyId();
    wxString transcodeStr = m_trans[selIndex]->transType_;

    double amount = m_trans[selIndex]->amt_;
    wxString amountStr;

    mmCurrency* pCurrency = core_->accountList_.getCurrencySharedPtr(accountId);
    int currencyid = pCurrency->currencyID_;
    //TODO: FIXME: If base currency does not set bug may happens
    if (basecurrencyid == -1) basecurrencyid = currencyid;
    wxString curpfxStr = pCurrency->pfxSymbol_;
    wxString cursfxStr = pCurrency->sfxSymbol_;
    double convrate = core_->accountList_.getAccountBaseCurrencyConvRate(accountId);

    wxString infoStr = "";
    if (transcodeStr == TRANS_TYPE_TRANSFER_STR)
    {
        double toconvrate = core_->accountList_.getAccountBaseCurrencyConvRate(toaccountId);
        mmCurrency* pCurrencyPtr = core_->accountList_.getCurrencySharedPtr(toaccountId);
        wxASSERT(pCurrencyPtr);
        wxString tocurpfxStr = pCurrencyPtr->pfxSymbol_;
        wxString tocursfxStr = pCurrencyPtr->sfxSymbol_;

        int tocurrencyid = pCurrencyPtr->currencyID_;
        double toamount = m_trans[selIndex]->toAmt_;
        double convertion = 0.0;
        if (toamount != 0.0 && amount != 0.0)
            convertion = ( convrate < toconvrate ? amount/toamount : toamount/amount);
        wxString toamountStr, convertionStr;

        CurrencyFormatter::instance().loadSettings(*pCurrencyPtr);
        toamountStr = CurrencyFormatter::float2Money(toamount);
        convertionStr = CurrencyFormatter::float2String(convertion);

        pCurrencyPtr = core_->accountList_.getCurrencySharedPtr(accountId);
        wxASSERT(pCurrencyPtr);
        CurrencyFormatter::instance().loadSettings(*pCurrencyPtr);
        amountStr = CurrencyFormatter::float2Money(amount);
        //if (currencyid == basecurrencyid)
        convertionStr = CurrencyFormatter::float2String(convertion);

        infoStr << amountStr << " ";
        if (amount!=toamount || tocurrencyid != currencyid)
            infoStr << "-> "  << toamountStr << " ";
        infoStr << wxString::Format(_("From %s to %s"), fromaccStr, intoaccStr);

        if (tocurrencyid != currencyid)
        {
            infoStr << " ( ";
            if (accountId == m_AccountID && convrate < toconvrate)
                infoStr  << tocurpfxStr << "1" << tocursfxStr << " = " << curpfxStr << convertionStr << cursfxStr << " ";
            else if (accountId == m_AccountID && convrate > toconvrate)
                infoStr << curpfxStr << "1" << cursfxStr << " = " << tocurpfxStr << convertionStr << tocursfxStr << " ";
            else if (accountId != m_AccountID && convrate < toconvrate)
                infoStr << tocurpfxStr << "1" << tocursfxStr << " = " << curpfxStr << convertionStr << cursfxStr << " ";
            else
                infoStr << curpfxStr << "1" << cursfxStr << " = " << tocurpfxStr << convertionStr << tocursfxStr << " ";
            infoStr << " )";
        }
    }
    else //For deposits and withdrawals calculates amount in base currency
    {
        //if (split_)
        {
            mmSplitTransactionEntries* splits = m_trans[selIndex]->splitEntries_;
            m_trans[selIndex]->getSplitTransactions(splits);

            for (const auto &i : splits->entries_)
            {
                amount = i->splitAmount_;
                if (m_trans[selIndex]->transType_ != TRANS_TYPE_DEPOSIT_STR)
                    amount = -amount;
                amountStr = CurrencyFormatter::float2Money(amount);
                infoStr << core_->categoryList_.GetFullCategoryString(
                    i->categID_, i->subCategID_)
                    << " = "
                    << amountStr
                    << "\n";
            }
        }

        if (currencyid != basecurrencyid) //Show nothing if account currency is base
        {
            //load settings for base currency
            wxString currencyName = core_->currencyList_.getCurrencyName(basecurrencyid);
            mmCurrency* pCurrencyBase = core_->currencyList_.getCurrencySharedPtr(currencyName);
            wxASSERT(pCurrencyBase);
            wxString basecuramountStr;
            mmDBWrapper::loadCurrencySettings(core_->db_.get(), pCurrencyBase->currencyID_);
            basecuramountStr = CurrencyFormatter::float2Money(amount*convrate);

            pCurrencyBase = core_->accountList_.getCurrencySharedPtr(accountId);
            wxASSERT(pCurrencyBase);
            CurrencyFormatter::instance().loadSettings(*pCurrencyBase);
            amountStr = CurrencyFormatter::float2Money(amount);

            //output
            infoStr << amountStr << " = " << basecuramountStr;
        }
    }
    return infoStr;
}
//---------------------------
void mmCheckingPanel::showTips()
{
    info_panel_->SetLabel(Tips(TIPS_BANKS));
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
    currentView_   = Model_Infotable::instance().GetStringInfo(wxString::Format("CHECK_FILTER_ID_%d", m_AccountID), vTrans);

    SetTransactionFilterState(currentView_ == VIEW_TRANS_ALL_STR);
    stxtMainFilter_->SetLabel(wxGetTranslation(currentView_));
}
//----------------------------------------------------------------------------
void mmCheckingPanel::initFilterSettings()
{
    date_range_ = new mmAllTime;

    if (currentView_ == VIEW_TRANS_ALL_STR)
        date_range_ = new mmAllTime;
    else if (currentView_ == VIEW_TRANS_TODAY_STR)
        date_range_ = new mmToday;
    else if (currentView_ == VIEW_TRANS_CURRENT_MONTH_STR)
        date_range_ = new mmCurrentMonth;
    else if (currentView_ == VIEW_TRANS_LAST_30_DAYS_STR)
        date_range_ = new mmLast30Days;
    else if (currentView_ == VIEW_TRANS_LAST_90_DAYS_STR)
        date_range_ = new mmLast90Days;
    else if (currentView_ == VIEW_TRANS_LAST_MONTH_STR)
        date_range_ = new mmLastMonth;
    else if (currentView_ == VIEW_TRANS_LAST_3MONTHS_STR)
        date_range_ = new mmLast3Months;
    else if (currentView_ == VIEW_TRANS_CURRENT_YEAR_STR)
        date_range_ = new mmCurrentYear;
    else if (currentView_ == VIEW_TRANS_LAST_12MONTHS_STR)
        date_range_ = new mmLast12Months;

    quickFilterBeginDate_ = date_range_->start_date();
    quickFilterEndDate_ = date_range_->end_date();
}
void mmCheckingPanel::OnFilterResetToViewAll(wxMouseEvent& event) {

    if (currentView_ == VIEW_TRANS_ALL_STR)
    {
        event.Skip();
        return;
    }

    stxtMainFilter_->SetLabel(_("View All transactions"));
    currentView_ = VIEW_TRANS_ALL_STR;
    SetTransactionFilterState(true);
    initFilterSettings();

    m_listCtrlAccount->m_selectedIndex = -1;
    m_listCtrlAccount->refreshVisualList();

}

void mmCheckingPanel::OnViewPopupSelected(wxCommandEvent& event)
{
    int evt =  event.GetId();

    if (evt ==  MENU_VIEW_ALLTRANSACTIONS)
    {
        currentView_ = VIEW_TRANS_ALL_STR;
        transFilterActive_ = false;
    }
    else if (evt == MENU_VIEW_TODAY)
        currentView_ = VIEW_TRANS_TODAY_STR;
    else if (evt == MENU_VIEW_CURRENTMONTH)
        currentView_ = VIEW_TRANS_CURRENT_MONTH_STR;
    else if (evt == MENU_VIEW_LAST30)
        currentView_ = VIEW_TRANS_LAST_30_DAYS_STR;
    else if (evt == MENU_VIEW_LAST90)
        currentView_ = VIEW_TRANS_LAST_90_DAYS_STR;
    else if (evt == MENU_VIEW_LAST3MONTHS)
        currentView_ = VIEW_TRANS_LAST_3MONTHS_STR;
    else if (evt == MENU_VIEW_LASTMONTH)
        currentView_ = VIEW_TRANS_LAST_MONTH_STR;
    else if (evt == MENU_VIEW_CURRENTYEAR)
        currentView_ = VIEW_TRANS_CURRENT_YEAR_STR;
    else if (evt == MENU_VIEW_LAST12MONTHS)
        currentView_ = VIEW_TRANS_LAST_12MONTHS_STR;
    else
        wxASSERT(false);

    stxtMainFilter_->SetLabel(wxGetTranslation(currentView_));
    SetTransactionFilterState(currentView_ == VIEW_TRANS_ALL_STR);

    m_listCtrlAccount->m_selectedIndex = -1;

    Model_Infotable::instance().Set(wxString::Format("CHECK_FILTER_ID_%ld", (long)m_AccountID), currentView_);
    initFilterSettings();
    m_listCtrlAccount->refreshVisualList(m_listCtrlAccount->m_selectedID);
     
}

void mmCheckingPanel::DeleteViewedTransactions()
{
    core_->db_.get()->Begin();
    for (const auto& i : m_trans)
    {
        if (m_listCtrlAccount->m_selectedForCopy == (long)i->transactionID())
            m_listCtrlAccount->m_selectedForCopy = -1;
        core_->bTransactionList_.deleteTransaction(m_AccountID, i->transactionID());
    }
    core_->db_.get()->Commit();
}

void mmCheckingPanel::DeleteFlaggedTransactions(const wxString& status)
{
    for (const auto& i : m_trans)
    {
        if (i->status_ == status)
        {
            if (m_listCtrlAccount->m_selectedForCopy == (long)i->transactionID())
                m_listCtrlAccount->m_selectedForCopy = -1;
            core_->bTransactionList_.deleteTransaction(m_AccountID, i->transactionID());
        }
    }
}

void mmCheckingPanel::OnFilterTransactions(wxMouseEvent& event)
{
    int e = event.GetEventType();

    wxBitmap bitmapFilterIcon(rightarrow_xpm);

    if (e == wxEVT_LEFT_DOWN) {
        transFilterDlg_->setAccountToolTip("Select account used in transfer transactions");
        if (transFilterDlg_->ShowModal() == wxID_OK)
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

//----------------------------------------------------------------------------

void TransactionListCtrl::OnListItemSelected(wxListEvent& event)
{
    m_selectedIndex = event.GetIndex();
    m_cp->updateExtraTransactionData(m_selectedIndex);

    if (m_cp->m_listCtrlAccount->GetSelectedItemCount()>1)
        m_cp->btnEdit_->Enable(false);

    m_cp->m_listCtrlAccount->m_selectedID = m_cp->m_trans[m_selectedIndex]->transactionID();
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
    wxMenu menu;
    menu.Append(MENU_TREEPOPUP_NEW, _("&New Transaction"));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_EDIT, _("&Edit Transaction"));
    if (hide_menu_item) menu.Enable(MENU_TREEPOPUP_EDIT, false);
    menu.Append(MENU_ON_COPY_TRANSACTION, _("&Copy Transaction"));
    if (hide_menu_item) menu.Enable(MENU_ON_COPY_TRANSACTION, false);
    menu.Append(MENU_ON_DUPLICATE_TRANSACTION, _("D&uplicate Transaction"));
    if (hide_menu_item) menu.Enable(MENU_ON_DUPLICATE_TRANSACTION, false);
    menu.Append(MENU_TREEPOPUP_MOVE, _("&Move Transaction"));
    if (hide_menu_item || (m_cp->core_->accountList_.getNumBankAccounts() < 2) 
        || m_cp->m_trans[m_selectedIndex]->transType_ == TRANS_TYPE_TRANSFER_STR)
        menu.Enable(MENU_TREEPOPUP_MOVE, false);
    menu.Append(MENU_ON_PASTE_TRANSACTION, _("&Paste Transaction"));
    if (m_selectedForCopy < 0) menu.Enable(MENU_ON_PASTE_TRANSACTION, false);

    menu.AppendSeparator();

    menu.Append(MENU_TREEPOPUP_VIEW_SPLIT_CATEGORIES, _("&View Split Categories"));
    if (hide_menu_item || (m_cp->m_trans[m_selectedIndex]->categID_ > -1))
        menu.Enable(MENU_TREEPOPUP_VIEW_SPLIT_CATEGORIES, false);

    menu.AppendSeparator();

    wxMenu* subGlobalOpMenuDelete = new wxMenu();
    subGlobalOpMenuDelete->Append(MENU_TREEPOPUP_DELETE, _("&Delete Transaction"));
    if (hide_menu_item) subGlobalOpMenuDelete->Enable(MENU_TREEPOPUP_DELETE, false);
    subGlobalOpMenuDelete->AppendSeparator();
    subGlobalOpMenuDelete->Append(MENU_TREEPOPUP_DELETE_VIEWED, _("Delete all transactions in current view"));
    subGlobalOpMenuDelete->Append(MENU_TREEPOPUP_DELETE_FLAGGED, _("Delete Viewed \"Follow Up\" Trans."));
    menu.Append(MENU_TREEPOPUP_DELETE, _("&Delete"), subGlobalOpMenuDelete);

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

int TransactionListCtrl::OnMarkTransactionDB(const wxString& status)
{
    if (m_selectedIndex < 0) return -1;

    int transID = m_cp->m_trans[m_selectedIndex]->transactionID();
    if (mmDBWrapper::updateTransactionWithStatus(*m_cp->getDb(), transID, status))
        m_cp->m_trans[m_selectedIndex]->status_ = status;

    return transID;
}
//----------------------------------------------------------------------------

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

    int transID = OnMarkTransactionDB(status);

    refreshVisualList(transID);
    //TODO: blinkings may be avoided
    //m_cp->m_listCtrlAccount->RefreshItems(m_selectedIndex, m_selectedIndex);
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
            , wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION);
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
            m_cp->core_->db_.get()->Begin();
            m_cp->DeleteFlaggedTransactions("F");
            m_cp->core_->db_.get()->Commit();
        }
    }
    else
    {
        m_cp->core_->db_.get()->Begin();

        for (const auto& i : m_cp->m_trans)
        {
            int transID = i->transactionID();
            if (mmDBWrapper::updateTransactionWithStatus(*m_cp->getDb(), transID, status))
                i->status_ = status;
        }

        m_cp->core_->db_.get()->Commit();
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
    Model_Setting::instance().Set("CHECK_ASC", g_asc);
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

wxString mmCheckingPanel::getItem(long item, long column) const
{
    wxString cell_value = "";

    bool ok = !m_trans.empty() &&
              ( item >= 0 ) &&
              ( item < static_cast<long>(m_trans.size()) ) &&
              m_trans[item];

    if (ok)
    {
        const mmBankTransaction &t = *m_trans[item];

        if (column == COL_DATE_OR_TRANSACTION_ID) cell_value = mmGetDateForDisplay(t.date_);
        else if (column == COL_TRANSACTION_NUMBER) cell_value = t.transNum_;
        else if (column == COL_PAYEE_STR) cell_value = t.arrow_ + t.payeeStr_;
        else if (column == COL_STATUS) cell_value = t.status_;
        else if (column == COL_CATEGORY) cell_value = t.fullCatStr_.IsEmpty() ? "..." : t.fullCatStr_;
        else if (column == COL_WITHDRAWAL)
            cell_value = (t.withdrawal_amt_ >= 0) ? CurrencyFormatter::float2String(t.withdrawal_amt_) : "";
        else if (column == COL_DEPOSIT)
            cell_value = (t.deposit_amt_ > 0) ? CurrencyFormatter::float2String(t.deposit_amt_) : "";
        else if (column == COL_BALANCE)
            cell_value = CurrencyFormatter::float2String(t.balance_);
        else if (column == COL_NOTES) cell_value = t.notes_;
        else
            wxASSERT(false);
    }

    return cell_value;
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
    if (m_cp->m_trans.size() < 1) return ICON_NONE;

    int res = -1;
    if(column == COL_DATE_OR_TRANSACTION_ID)
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
    wxASSERT(m_cp);
    wxASSERT(item >= 0);

    size_t idx = item;
    bool ok = m_cp && idx < m_cp->m_trans.size();

    mmBankTransaction *tr = ok ? m_cp->m_trans[idx] : 0;
    bool in_the_future = tr && tr->date_.GetDateOnly() > wxDateTime::Now().GetDateOnly();

    if (in_the_future) // apply alternating background pattern
    {
        return item % 2 ? (wxListItemAttr*)&m_attr3 : (wxListItemAttr*)&m_attr4;
    }

    return item % 2 ? (wxListItemAttr*)&m_attr1 : (wxListItemAttr*)&m_attr2;

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

    m_selectedForCopy = m_cp->m_trans[m_selectedIndex]->transactionID();
}
//----------------------------------------------------------------------------

void TransactionListCtrl::OnPaste(wxCommandEvent& WXUNUSED(event))
{
    if (m_selectedForCopy < 0) return;

    bool useOriginalDate = Model_Setting::instance().GetBoolSetting(INIDB_USE_ORG_DATE_COPYPASTE, false);

    mmBankTransaction* pCopiedTrans =
        m_cp->core_->bTransactionList_.copyTransaction(m_selectedForCopy, m_cp->m_AccountID, useOriginalDate);

    int transID = pCopiedTrans->transactionID();
    topItemIndex_ = m_selectedIndex;
    refreshVisualList(transID);
}
//----------------------------------------------------------------------------

void TransactionListCtrl::OnListKeyDown(wxListEvent& event)
{
    if (wxGetKeyState(WXK_COMMAND) || wxGetKeyState(WXK_ALT) || wxGetKeyState(WXK_CONTROL)
        || m_selectedIndex == -1) {
        event.Skip();
        return;
    }

    //find the topmost visible item - this will be used to set
    // where to display the list again after refresh
    topItemIndex_ = GetTopItem() + GetCountPerPage() -1;

    //Read status of the selected transaction
    wxString status = m_cp->m_trans[m_selectedIndex]->status_;

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
    else if (wxGetKeyState(WXK_DELETE)|| wxGetKeyState(WXK_NUMPAD_DELETE))
    {
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_TREEPOPUP_DELETE);
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
        , wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION);

    if (msgDlg.ShowModal() == wxID_YES)
    {
        m_cp->core_->db_.get()->Begin();
        long x = 0;
        for (const auto& i : m_cp->m_trans)
        {
            long transID = i->transactionID();
            if (m_cp->m_listCtrlAccount->GetItemState(x, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED)
            {
                m_cp->core_->bTransactionList_.deleteTransaction(m_cp->m_AccountID, transID);
                if (x <= topItemIndex_) topItemIndex_--;
                m_selectedIndex--;
                if (m_selectedForCopy == transID) m_selectedForCopy = -1;
            }
        x++;
        }
        m_cp->core_->db_.get()->Commit();

        refreshVisualList();
    }
}
//----------------------------------------------------------------------------

void TransactionListCtrl::OnEditTransaction(wxCommandEvent& /*event*/)
{
    if (m_selectedIndex < 0) return;
    
    topItemIndex_ = GetTopItem() + GetCountPerPage() -1;

    mmTransDialog dlg(m_cp->core_, m_cp->m_AccountID,
       m_cp->m_trans[m_selectedIndex], true, this);
    dlg.ShowModal();

    refreshVisualList(m_cp->m_trans[m_selectedIndex]->transactionID());
}
//----------------------------------------------------------------------------

void TransactionListCtrl::OnNewTransaction(wxCommandEvent& /*event*/)
{
    mmTransDialog dlg(m_cp->core_, m_cp->m_AccountID, NULL, false, this);

    topItemIndex_ = GetTopItem() + GetCountPerPage() -1;

    if ( dlg.ShowModal() == wxID_OK )
    {
        int transID = dlg.getTransID();
        refreshVisualList(transID);
    }
}
//----------------------------------------------------------------------------

void TransactionListCtrl::OnDuplicateTransaction(wxCommandEvent& /*event*/)
{
    if (m_selectedIndex < 0) return;

    topItemIndex_ = GetTopItem() + GetCountPerPage() -1;

    wxDateTime transTime = m_cp->m_trans[m_selectedIndex]->date_;
    mmTransDialog dlg(m_cp->core_, m_cp->m_AccountID,
        m_cp->m_trans[m_selectedIndex], true, this);

    dlg.SetDialogToDuplicateTransaction();
    if ( dlg.ShowModal() == wxID_OK )
    {
        int transID = dlg.getTransID();
        refreshVisualList(transID);
    }
}
//----------------------------------------------------------------------------

void TransactionListCtrl::refreshVisualList(int trans_id)
{
    m_cp->initVirtualListControl();
    m_cp->markSelectedTransaction(trans_id);


    if (topItemIndex_ >= (long)m_cp->m_trans.size())
        topItemIndex_ = g_asc ? (long)m_cp->m_trans.size() - 1 : 0;

    if (m_selectedIndex >= (long)m_cp->m_trans.size() || m_selectedIndex < 0)
        m_selectedIndex = g_asc ? (long)m_cp->m_trans.size() - 1 : 0;

    if (topItemIndex_ < m_selectedIndex) topItemIndex_ = m_selectedIndex;

    if (m_cp->m_trans.size() > 0) {
        RefreshItems(0, m_cp->m_trans.size() - 1);
    }
    else
        m_selectedIndex = -1;

    if (m_selectedIndex >= 0 && m_cp->m_trans.size() > 0)
    {
        SetItemState(m_selectedIndex, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        SetItemState(m_selectedIndex, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
        if (topItemIndex_ < 0 || (topItemIndex_ - m_selectedIndex) > GetCountPerPage()) 
            topItemIndex_ = m_selectedIndex;
        EnsureVisible(topItemIndex_);
    }
    //debuger
    //wxLogDebug(wxString::Format("+trx id:%ld | top:%ld | selected:%ld", trans_id, topItemIndex_, m_selectedIndex));

    m_cp->updateExtraTransactionData(m_selectedIndex);
}

//  Called when moving a transaction to a new account.
int TransactionListCtrl::DestinationAccountID()
{
    wxString source_name = m_cp->core_->accountList_.GetAccountName(m_cp->m_AccountID);
    wxString headerMsg = _("Moving Transaction from ") + source_name + _(" to...");

    wxArrayString dest_name_list = m_cp->core_->accountList_.getAccountsName(m_cp->m_AccountID);
    wxSingleChoiceDialog scd(this, _("Select the destination Account "), headerMsg , dest_name_list);

    int dest_account_id = -1;
    if (scd.ShowModal() == wxID_OK)
    {
        wxString dest_account_name = scd.GetStringSelection();
        dest_account_id = m_cp->core_->accountList_.GetAccountId(dest_account_name);
    }

    return dest_account_id;
}

void TransactionListCtrl::OnMoveTransaction(wxCommandEvent& /*event*/)
{
    if (m_selectedIndex < 0) return;

    int toAccountID = DestinationAccountID();
    if (toAccountID != -1)
    {
        mmBankTransaction* pTransaction;
        pTransaction = m_cp->core_->bTransactionList_.getBankTransactionPtr(
            m_cp->m_trans[m_selectedIndex]->transactionID()
        );

        // Looking at transaction from A end. Transaction is a deposit, withdrawal or transfer.
        if (m_cp->m_AccountID == pTransaction->accountID_)
            pTransaction->accountID_ = toAccountID;

        // Looking at transaction from b end. Transaction is a transfer
        if (m_cp->m_AccountID == pTransaction->toAccountID_)
            pTransaction->toAccountID_ = toAccountID;

        // Update the transaction
        m_cp->core_->bTransactionList_.UpdateTransaction(pTransaction);
        refreshVisualList();
    }
}

//----------------------------------------------------------------------------
void TransactionListCtrl::OnViewSplitTransaction(wxCommandEvent& /*event*/)
{
    if (m_selectedIndex < 0) return;

    if (m_cp->m_trans[m_selectedIndex]->categID_ < 0)
        m_cp->DisplaySplitCategories(m_cp->m_trans[m_selectedIndex]->transactionID());

}

//----------------------------------------------------------------------------
void TransactionListCtrl::OnListItemActivated(wxListEvent& /*event*/)
{
    if (m_selectedIndex < 0) return;

    wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_TREEPOPUP_EDIT);
    AddPendingEvent(evt);
}

//----------------------------------------------------------------------------

TransactionListCtrl::TransactionListCtrl(
    mmCheckingPanel *cp,
    wxWindow *parent,
    const wxWindowID id,
    const wxPoint& pos,
    const wxSize& size,
    long style
) :
    wxListCtrl(parent, id, pos, size, style | wxWANTS_CHARS),
    m_cp(cp),
    m_selectedIndex(-1),
    m_selectedForCopy(-1),
    m_attr1(*wxBLACK, mmColors::listAlternativeColor0, wxNullFont),
    m_attr2(*wxBLACK, mmColors::listAlternativeColor1, wxNullFont),
    m_attr3(mmColors::listFutureDateColor, mmColors::listAlternativeColor0, wxNullFont),
    m_attr4(mmColors::listFutureDateColor, mmColors::listAlternativeColor1, wxNullFont),
    m_sortCol(COL_DEF_SORT),
    m_asc(true)
{
    wxASSERT(m_cp);

    const wxAcceleratorEntry entries[] =
    {
        wxAcceleratorEntry(wxACCEL_CTRL, 'C', MENU_ON_COPY_TRANSACTION),
        wxAcceleratorEntry(wxACCEL_CTRL, 'V', MENU_ON_PASTE_TRANSACTION),
        wxAcceleratorEntry(wxACCEL_ALT,  'N', MENU_ON_NEW_TRANSACTION),
        wxAcceleratorEntry(wxACCEL_CTRL, 'D', MENU_ON_DUPLICATE_TRANSACTION)
    };

    wxAcceleratorTable tab(sizeof(entries)/sizeof(*entries), entries);
    SetAcceleratorTable(tab);
}
//----------------------------------------------------------------------------

std::shared_ptr<wxSQLite3Database> mmCheckingPanel::getDb() const
{
    wxASSERT(core_);
    return core_->db_;
}

//----------------------------------------------------------------------------
void mmCheckingPanel::OnSearchTxtEntered(wxCommandEvent& /*event*/)
{
    //event.GetString() does not working. It seems wxWidgets issue
    //wxString searchString = event.GetString();

    wxSearchCtrl* st = (wxSearchCtrl*)FindWindow(wxID_FIND);
    wxString search_string = st->GetValue().Lower();
    if (search_string.IsEmpty()) return;

    double amount= 0, deposit = 0, withdrawal = 0;
    bool valid_amount =  CurrencyFormatter::formatCurrencyToDouble(search_string, amount);
    bool withdrawal_only = false;
    if (valid_amount && amount < 0)
    {
        amount = -amount;
        withdrawal_only = true;
    }

    long last = m_listCtrlAccount->GetItemCount();
    long selectedItem = m_listCtrlAccount->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (selectedItem < 0) //nothing selected
        selectedItem = g_asc ? last - 1 : 0;

    search_string.Append("*");
    while (selectedItem >= 0 && selectedItem <= last)
    {
        g_asc ?  selectedItem-- : selectedItem++;
        const wxString t = getItem(selectedItem, COL_NOTES);
        if (valid_amount)  CurrencyFormatter::formatCurrencyToDouble(getItem(selectedItem, COL_DEPOSIT), deposit);
        if (valid_amount)  CurrencyFormatter::formatCurrencyToDouble(getItem(selectedItem, COL_WITHDRAWAL), withdrawal);
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
    wxString transTypeStr = core_->bTransactionList_.getBankTransactionPtr(transID)->transType_;
    int transType = 0;
    if (transTypeStr== TRANS_TYPE_DEPOSIT_STR)  transType = 1;
    if (transTypeStr== TRANS_TYPE_TRANSFER_STR) transType = 2;

    SplitTransactionDialog splitTransDialog(
        core_,
        core_->bTransactionList_.getBankTransactionPtr(transID)->splitEntries_,
        transType,
        this
    );
    splitTransDialog.SetDisplaySplitCategories();
    splitTransDialog.ShowModal();
}

// Refresh account screen with new details
void mmCheckingPanel::DisplayAccountDetails(int accountID)
{
    m_AccountID = accountID;
    this->windowsFreezeThaw();   // prevent screen updates while controls being repopulated

    initViewTransactionsHeader();

    m_listCtrlAccount->m_selectedIndex = -1;
    m_listCtrlAccount->refreshVisualList();
    showTips();

    this->windowsFreezeThaw();     // Enable screen refresh with new data.
}

void mmCheckingPanel::SetTransactionFilterState(bool active)
{
    bitmapTransFilter_->Enable(active || transFilterActive_);
    statTextTransFilter_->Enable(active || transFilterActive_);
    bitmapMainFilter_->Enable(!transFilterActive_);
    stxtMainFilter_->Enable(!transFilterActive_);
}

void mmCheckingPanel::SetSelectedTransaction(int transID)
{
    m_listCtrlAccount->refreshVisualList(transID);
    m_listCtrlAccount->SetFocus();
}

