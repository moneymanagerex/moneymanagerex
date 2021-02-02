/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2013, 2014, 2020, 2021 Nikolay Akimov

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

#include "attachmentdialog.h"
#include "billsdepositsdialog.h"
#include "constants.h"
#include "filtertransdialog.h"
#include "images_list.h"
#include "mmchecking_list.h"
#include "mmcheckingpanel.h"
#include "mmex.h"
#include "mmframe.h"
#include "mmSimpleDialogs.h"
#include "paths.h"
#include "splittransactionsdialog.h"
#include "transactionsupdatedialog.h"
#include "transdialog.h"
#include "util.h"
#include "validators.h"
#include "model/allmodel.h"
#include <wx/clipbrd.h>


#include <wx/srchctrl.h>
#include <algorithm>
#include <wx/sound.h>

//----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(TransactionListCtrl, mmListCtrl)
EVT_LIST_ITEM_SELECTED(wxID_ANY, TransactionListCtrl::OnListItemSelected)
EVT_LIST_ITEM_ACTIVATED(wxID_ANY, TransactionListCtrl::OnListItemActivated)
EVT_LIST_ITEM_FOCUSED(wxID_ANY, TransactionListCtrl::OnListItemFocused)
EVT_RIGHT_DOWN(TransactionListCtrl::OnMouseRightClick)
EVT_LEFT_DOWN(TransactionListCtrl::OnListLeftClick)
EVT_LIST_KEY_DOWN(wxID_ANY, TransactionListCtrl::OnListKeyDown)

EVT_MENU_RANGE(MENU_TREEPOPUP_MARKRECONCILED
    , MENU_TREEPOPUP_MARKDELETE, TransactionListCtrl::OnMarkTransaction)

    EVT_MENU_RANGE(MENU_TREEPOPUP_MARKRECONCILED_ALL
        , MENU_TREEPOPUP_DELETE_UNRECONCILED, TransactionListCtrl::OnMarkAllTransactions)

    EVT_MENU_RANGE(MENU_TREEPOPUP_NEW_WITHDRAWAL, MENU_TREEPOPUP_NEW_DEPOSIT, TransactionListCtrl::OnNewTransaction)
    EVT_MENU(MENU_TREEPOPUP_NEW_TRANSFER, TransactionListCtrl::OnNewTransferTransaction)
    EVT_MENU(MENU_TREEPOPUP_DELETE2, TransactionListCtrl::OnDeleteTransaction)
    EVT_MENU(MENU_TREEPOPUP_EDIT2, TransactionListCtrl::OnEditTransaction)
    EVT_MENU(MENU_TREEPOPUP_MOVE2, TransactionListCtrl::OnMoveTransaction)

    EVT_MENU(MENU_ON_COPY_TRANSACTION, TransactionListCtrl::OnCopy)
    EVT_MENU(MENU_ON_PASTE_TRANSACTION, TransactionListCtrl::OnPaste)
    EVT_MENU(MENU_ON_NEW_TRANSACTION, TransactionListCtrl::OnNewTransaction)
    EVT_MENU(MENU_ON_DUPLICATE_TRANSACTION, TransactionListCtrl::OnDuplicateTransaction)
    EVT_MENU_RANGE(MENU_ON_SET_UDC0, MENU_ON_SET_UDC7, TransactionListCtrl::OnSetUserColour)

    EVT_MENU(MENU_TREEPOPUP_VIEW_SPLIT_CATEGORIES, TransactionListCtrl::OnViewSplitTransaction)
    EVT_MENU(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, TransactionListCtrl::OnOrganizeAttachments)
    EVT_MENU(MENU_TREEPOPUP_CREATE_REOCCURANCE, TransactionListCtrl::OnCreateReoccurance)
    EVT_CHAR(TransactionListCtrl::OnChar)

    wxEND_EVENT_TABLE();

//----------------------------------------------------------------------------

void TransactionListCtrl::sortTable()
{
    switch (g_sortcol)
    {
    case TransactionListCtrl::COL_ID:
        std::stable_sort(this->m_trans.begin(), this->m_trans.end(), SorterByTRANSID());
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
    default:
        break;
    }

    if (!g_asc)
        std::reverse(this->m_trans.begin(), this->m_trans.end());

    RefreshItems(0, m_trans.size() - 1);
}

TransactionListCtrl::TransactionListCtrl(
    mmCheckingPanel *cp,
    wxWindow *parent,
    const wxWindowID id
) :
    mmListCtrl(parent, id),
    m_cp(cp),
    m_selectedIndex(-1),
    m_selectedForCopy(-1),
    m_attr1(new wxListItemAttr(*wxBLACK, mmColors::listAlternativeColor0, wxNullFont)),
    m_attr2(new wxListItemAttr(*wxBLACK, mmColors::listAlternativeColor1, wxNullFont)),
    m_attr3(new wxListItemAttr(mmColors::listFutureDateColor, mmColors::listAlternativeColor0, wxNullFont)),
    m_attr4(new wxListItemAttr(mmColors::listFutureDateColor, mmColors::listAlternativeColor1, wxNullFont)),
    m_attr11(new wxListItemAttr(*wxBLACK, mmColors::userDefColor1, wxNullFont)),
    m_attr12(new wxListItemAttr(*wxBLACK, mmColors::userDefColor2, wxNullFont)),
    m_attr13(new wxListItemAttr(*wxBLACK, mmColors::userDefColor3, wxNullFont)),
    m_attr14(new wxListItemAttr(*wxBLACK, mmColors::userDefColor4, wxNullFont)),
    m_attr15(new wxListItemAttr(*wxBLACK, mmColors::userDefColor5, wxNullFont)),
    m_attr16(new wxListItemAttr(*wxYELLOW, mmColors::userDefColor6, wxNullFont)),
    m_attr17(new wxListItemAttr(*wxYELLOW, mmColors::userDefColor7, wxNullFont)),
    m_sortCol(COL_DEF_SORT),
    g_sortcol(COL_DEF_SORT),
    m_prevSortCol(COL_DEF_SORT),
    g_asc(true),
    m_selectedID(-1),
    m_topItemIndex(-1)
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

    wxAcceleratorTable tab(sizeof(entries) / sizeof(*entries), entries);
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

    m_col_width = "CHECK_COL%d_WIDTH";

    m_default_sort_column = COL_DEF_SORT;
    m_today = wxDateTime::Today().FormatISODate();

    SetSingleStyle(wxLC_SINGLE_SEL, false);
}

TransactionListCtrl::~TransactionListCtrl()
{}

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

    if (GetSelectedItemCount() > 1)
        m_cp->enableEditDeleteButtons(true);

    m_selectedID = m_trans[m_selectedIndex].TRANSID;
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
    // Workaround for wxWidgets bug #4541 which affects MSW build
    if ((m_selectedIndex >= 0) && (index != m_selectedIndex) && event.ShiftDown())
    {
        // Note: GetSelectedItemCount() does not return correct count at this time
        // so we can't call enableEditDeleteButtons() or updateExtraTransactionData()
        m_cp->m_btnDuplicate->Enable(false);
        m_cp->m_btnAttachment->Enable(false);
    }
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
        const Model_Checking::Full_Data& tran = m_trans.at(m_selectedIndex);
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

    int i = GetSelectedItemCount();
    menu.Append(MENU_TREEPOPUP_EDIT2, wxPLURAL("&Edit Transaction", "&Edit Transactions", i));
    if (hide_menu_item) menu.Enable(MENU_TREEPOPUP_EDIT2, false);

    menu.Append(MENU_ON_COPY_TRANSACTION, wxPLURAL("&Copy Transaction", "&Copy Transactions", i));
    if (hide_menu_item) menu.Enable(MENU_ON_COPY_TRANSACTION, false);

    menu.Append(MENU_ON_PASTE_TRANSACTION, _("&Paste Transaction"));
    if (m_selectedForCopy < 0) menu.Enable(MENU_ON_PASTE_TRANSACTION, false);

    menu.Append(MENU_ON_DUPLICATE_TRANSACTION, _("D&uplicate Transaction"));
    if (hide_menu_item || multiselect) menu.Enable(MENU_ON_DUPLICATE_TRANSACTION, false);

    menu.Append(MENU_TREEPOPUP_MOVE2, _("&Move Transaction"));
    if (hide_menu_item || multiselect || type_transfer || (Model_Account::money_accounts_num() < 2) || is_foreign)
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
    subGlobalOpMenuDelete->Append(MENU_TREEPOPUP_DELETE2, wxPLURAL("&Delete selected transaction", "&Delete selected transactions", i));
    if (hide_menu_item) subGlobalOpMenuDelete->Enable(MENU_TREEPOPUP_DELETE2, false);
    subGlobalOpMenuDelete->AppendSeparator();
    subGlobalOpMenuDelete->Append(MENU_TREEPOPUP_DELETE_VIEWED, _("Delete all transactions in current view"));
    subGlobalOpMenuDelete->Append(MENU_TREEPOPUP_DELETE_FLAGGED, _("Delete Viewed \"Follow Up\" Transactions"));
    subGlobalOpMenuDelete->Append(MENU_TREEPOPUP_DELETE_UNRECONCILED, _("Delete Viewed \"Unreconciled\" Transactions"));
    menu.Append(MENU_TREEPOPUP_DELETE2, _("&Delete "), subGlobalOpMenuDelete);

    menu.AppendSeparator();

    wxMenu* subGlobalOpMenuMark = new wxMenu();
    subGlobalOpMenuMark->Append(MENU_TREEPOPUP_MARKRECONCILED, _("as Reconciled"));
    if (hide_menu_item) subGlobalOpMenuMark->Enable(MENU_TREEPOPUP_MARKRECONCILED, false);
    subGlobalOpMenuMark->Append(MENU_TREEPOPUP_MARKUNRECONCILED, _("as Unreconciled"));
    if (hide_menu_item) subGlobalOpMenuMark->Enable(MENU_TREEPOPUP_MARKUNRECONCILED, false);
    subGlobalOpMenuMark->Append(MENU_TREEPOPUP_MARKVOID, _("as Void"));
    if (hide_menu_item) subGlobalOpMenuMark->Enable(MENU_TREEPOPUP_MARKVOID, false);
    subGlobalOpMenuMark->Append(MENU_TREEPOPUP_MARK_ADD_FLAG_FOLLOWUP, _("as Followup"));
    if (hide_menu_item) subGlobalOpMenuMark->Enable(MENU_TREEPOPUP_MARK_ADD_FLAG_FOLLOWUP, false);
    subGlobalOpMenuMark->Append(MENU_TREEPOPUP_MARKDUPLICATE, _("as Duplicate"));
    if (hide_menu_item) subGlobalOpMenuMark->Enable(MENU_TREEPOPUP_MARKDUPLICATE, false);
    menu.Append(wxID_ANY, _("Mark all being selected"), subGlobalOpMenuMark);

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
    int evt = event.GetId();
    bool bRefreshRequired = false;
    wxString org_status = "";
    wxString status = "";
    switch (evt)
    {
    case MENU_TREEPOPUP_MARKRECONCILED:         status = "R"; break;
    case MENU_TREEPOPUP_MARKUNRECONCILED:       status = ""; break;
    case MENU_TREEPOPUP_MARKVOID:               status = "V"; break;
    case MENU_TREEPOPUP_MARK_ADD_FLAG_FOLLOWUP: status = "F"; break;
    case MENU_TREEPOPUP_MARKDUPLICATE:          status = "D"; break;
    default: wxASSERT(false);
    }

    Model_Checking::instance().Savepoint();

    for (int row = 0; row < GetItemCount(); row++)
    {
        if (GetItemState(row, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED)
        {
            bRefreshRequired |= (status == "V") || (m_trans[row].STATUS == "V");
            m_trans[row].STATUS = status;
            Model_Checking::instance().save(&m_trans[row]);
        }
    }

    Model_Checking::instance().ReleaseSavepoint();

    if ((m_cp->m_transFilterActive && m_cp->m_trans_filter_dlg->getStatusCheckBox())
        || bRefreshRequired)
    {
        refreshVisualList(m_trans[m_selectedIndex].TRANSID);
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
    int evt = event.GetId();
    wxString status = "";
    if (evt == MENU_TREEPOPUP_MARKRECONCILED_ALL)             status = "R";
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
            , _("Do you really want to delete all the transactions shown?")
            , _("Confirm Transaction Deletion")
            , wxYES_NO | wxNO_DEFAULT | wxICON_ERROR);
        if (msgDlg.ShowModal() == wxID_YES)
        {
            DeleteViewedTransactions();
        }
    }
    else if (status == "M" || status == "0")
    {
        const wxString statusStr = (status == "M" ? _("Follow Up") : _("Unreconciled"));
        const wxString shotStatusStr = (status == "M" ? "F" : "");
        wxMessageDialog msgDlg(this
            , wxString::Format(_("Do you really want to delete all the \"%s\" transactions shown?"), statusStr)
            , _("Confirm Transaction Deletion")
            , wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION);
        if (msgDlg.ShowModal() == wxID_YES)
        {
            DeleteFlaggedTransactions(shotStatusStr);
        }
    }
    else
    {
        for (auto& tran : m_trans)
        {
            tran.NOTES.Replace(mmAttachmentManage::GetAttachmentNoteSign(), wxEmptyString, true);
            tran.STATUS = status;
        }
        Model_Checking::instance().save(m_trans);
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
    if (m_trans.empty()) return ICON_NONE;

    int res = -1;
    if (column == COL_IMGSTATUS)
    {
        res = ICON_NONE;
        wxString status = m_cp->getItem(item, COL_STATUS);
        if (status.length() > 1)
            status = status.Mid(2, 1);
        if (status == "F")
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
    if (item < 0 || item >= static_cast<int>(m_trans.size())) return 0;

    const Model_Checking::Full_Data& tran = m_trans[item];
    bool in_the_future = (tran.TRANSDATE > m_today);

    // apply alternating background pattern
    int user_colour_id = tran.FOLLOWUPID;
    if (user_colour_id < 0) user_colour_id = 0;
    else if (user_colour_id > 7) user_colour_id = 0;

    if (user_colour_id == 0) {
        if (in_the_future) {
            return (item % 2 ? m_attr3.get() : m_attr4.get());
        }
        return (item % 2 ? m_attr1.get() : m_attr2.get());
    }

    switch (user_colour_id)
    {
    case 1:
        return m_attr11.get();
    case 2:
        return m_attr12.get();
    case 3:
        return m_attr13.get();
    case 4:
        return m_attr14.get();
    case 5:
        return m_attr15.get();
    case 6:
        return m_attr16.get();
    case 7:
        return m_attr17.get();
    }
    return (item % 2 ? m_attr1.get() : m_attr2.get());
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
        wxGetKeyState(WXK_TAB) ||
        wxGetKeyState(WXK_RETURN) ||
        wxGetKeyState(WXK_NUMPAD_ENTER) ||
        wxGetKeyState(WXK_SPACE) ||
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
        m_selectedForCopy = m_trans[m_selectedIndex].TRANSID;

    if (wxTheClipboard->Open())
    {
        const wxString seperator = "\t";
        wxString data = "";
        if (GetSelectedItemCount() > 1)
        {
            for (int row = 0; row < GetItemCount(); row++)
            {
                if (GetItemState(row, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED)
                {
                    for (int column = 0; column < static_cast<int>(m_columns.size()); column++)
                    {
                        if (GetColumnWidth(column) > 0) {
                            data += inQuotes(OnGetItemText(row, column), seperator);
                            data += seperator;
                        }
                    }
                    data += "\n";
                }
            }
        }
        else
        {
            for (int column = 0; column < static_cast<int>(m_columns.size()); column++)
            {
                if (GetColumnWidth(column) > 0)
                    data += OnGetItemText(m_selectedIndex, column) + seperator;
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

    int transaction_id = m_trans[m_selectedIndex].TRANSID;
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
    int transaction_id = m_trans[m_selectedIndex].TRANSID;
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
    wxString status = m_trans[m_selectedIndex].STATUS;

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
    int sel = GetSelectedItemCount();
    if (sel < 1) return;

    m_topItemIndex = GetTopItem() + GetCountPerPage() - 1;

    Model_Checking::Data checking_entry = m_trans[m_selectedIndex];
    if (TransactionLocked(checking_entry.TRANSDATE)) {
        return;
    }

    //ask if they really want to delete
    const wxString text = wxString::Format(
        wxPLURAL("Do you really want to delete the selected transaction?"
            , "Do you really want to delete %i selected transactions?", sel)
        , sel);
    wxMessageDialog msgDlg(this
        , text
        , _("Confirm Transaction Deletion")
        , wxYES_NO | wxYES_DEFAULT | wxICON_ERROR);

    if (msgDlg.ShowModal() == wxID_YES)
    {
        long x = 0;
        for (const auto& i : m_trans)
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
                mmAttachmentManage::DeleteAllAttachments(Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION), transID);
                if (x <= m_topItemIndex) m_topItemIndex--;
                if (!m_trans.empty() && m_selectedIndex > 0) m_selectedIndex--;
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
                    _("Locked transaction to date: %s\n\n"
                        "Reconciled transactions.")
                    , mmGetDateForDisplay(m_cp->m_account->STATEMENTDATE)))
                    , _("MMEX Transaction Check"), wxOK | wxICON_WARNING);
                return true;
            }
        }
    }
    return false;
}

void TransactionListCtrl::OnEditTransaction(wxCommandEvent& /*event*/)
{
    if (m_selectedIndex < 0) return;

    if (GetSelectedItemCount() > 1)
    {
        std::vector<int> v;
        long x = 0;
        for (const auto& i : m_trans)
        {
            if (GetItemState(x, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED)
            {
                v.push_back(i.TRANSID);
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


    Model_Checking::Full_Data& checking_entry = m_trans[m_selectedIndex];
    int transaction_id = checking_entry.TRANSID;

    if (TransactionLocked(checking_entry.TRANSDATE))
    {
        return;
    }

    mmTransDialog dlg(this, m_cp->m_AccountID, transaction_id, m_cp->m_account_balance);
    if (dlg.ShowModal() == wxID_OK)
    {
        refreshVisualList(transaction_id);
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

void TransactionListCtrl::OnNewTransferTransaction(wxCommandEvent& /*event*/)
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
        m_trans[m_selectedIndex].FOLLOWUPID = user_colour_id;
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
    SetItemCount(m_trans.size());
    Show();
    sortTable();
    markSelectedTransaction(trans_id);

    long i = static_cast<long>(m_trans.size());
    if (m_topItemIndex >= i)
        m_topItemIndex = g_asc ? i - 1 : 0;
    if (m_selectedIndex > i - 1) m_selectedIndex = -1;
    if (m_topItemIndex < m_selectedIndex) m_topItemIndex = m_selectedIndex;


    if (m_selectedIndex >= 0 && !m_trans.empty())
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
    SetFocus();
}

void TransactionListCtrl::OnMoveTransaction(wxCommandEvent& /*event*/)
{
    if ((m_selectedIndex < 0) || (GetSelectedItemCount() > 1)) return;

    Model_Checking::Data checking_entry = m_trans[m_selectedIndex];
    if (TransactionLocked(checking_entry.TRANSDATE)) {
        return;
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

        Model_Checking::Full_Data& tran = m_trans[m_selectedIndex];
        tran.ACCOUNTID = dest_account_id;
        Model_Checking::instance().save(&tran);
        refreshVisualList();
    }
}

//----------------------------------------------------------------------------
void TransactionListCtrl::OnViewSplitTransaction(wxCommandEvent& /*event*/)
{
    if ((m_selectedIndex > -1) && (GetSelectedItemCount() == 1)) {
        const Model_Checking::Full_Data& tran = m_trans.at(m_selectedIndex);
        if (tran.has_split())
            m_cp->DisplaySplitCategories(tran.TRANSID);
    }
}

//----------------------------------------------------------------------------
void TransactionListCtrl::OnOrganizeAttachments(wxCommandEvent& /*event*/)
{
    if ((m_selectedIndex < 0) || (GetSelectedItemCount() > 1)) return;

    wxString RefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
    int RefId = m_trans[m_selectedIndex].TRANSID;

    mmAttachmentDialog dlg(this, RefType, RefId);
    dlg.ShowModal();

    refreshVisualList(RefId);
}

//----------------------------------------------------------------------------
void TransactionListCtrl::OnCreateReoccurance(wxCommandEvent& /*event*/)
{
    if ((m_selectedIndex < 0) || (GetSelectedItemCount() > 1)) return;

    mmBDDialog dlg(this, 0, false, false);
    dlg.SetDialogParameters(m_trans[m_selectedIndex]);
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

void TransactionListCtrl::markSelectedTransaction(int trans_id)
{
    if (trans_id > 0)
    {
        m_selectedIndex = -1;
        m_selectedID = -1;
        long i = 0;
        for (const auto & tran : m_trans)
        {
            //reset any selected items in the list
            if (GetItemState(i, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED)
            {
                SetItemState(i, 0, wxLIST_STATE_SELECTED);
            }
            // discover where the transaction has ended up in the list
            if (trans_id == tran.TRANSID)
            {
                m_selectedIndex = i;
                // set the selected ID to this transaction.
                m_selectedID = trans_id;
            }
            ++i;
        }
    }

    if (!m_trans.empty() && m_selectedIndex < 0)
    {
        long i = static_cast<long>(m_trans.size()) - 1;
        if (!g_asc)
            i = 0;
        EnsureVisible(i);
        m_selectedIndex = i;
        m_selectedID = m_trans[i].TRANSID;
    }
    else
    {
        m_cp->enableEditDeleteButtons(false);
        m_cp->showTips();
    }
}

void TransactionListCtrl::DeleteViewedTransactions()
{
    Model_Checking::instance().Savepoint();
    for (const auto& tran : this->m_trans)
    {
        if (Model_Checking::foreignTransaction(tran))
        {
            Model_Translink::RemoveTranslinkEntry(tran.TRANSID);
        }

        // remove also removes any split transactions
        Model_Checking::instance().remove(tran.TRANSID);
        mmAttachmentManage::DeleteAllAttachments(Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION), tran.TRANSID);
        if (m_selectedForCopy == tran.TRANSID) m_selectedForCopy = -1;
    }
    Model_Checking::instance().ReleaseSavepoint();
}

void TransactionListCtrl::DeleteFlaggedTransactions(const wxString& status)
{
    Model_Checking::instance().Savepoint();
    for (const auto& tran : this->m_trans)
    {
        if (tran.STATUS == status)
        {
            // remove also removes any split transactions
            Model_Checking::instance().remove(tran.TRANSID);
            mmAttachmentManage::DeleteAllAttachments(Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION), tran.TRANSID);
            if (m_selectedForCopy == tran.TRANSID) m_selectedForCopy = -1;
        }
    }
    Model_Checking::instance().ReleaseSavepoint();
}

void TransactionListCtrl::OnListItemFocused(wxListEvent& WXUNUSED(event))
{
    long count = this->GetSelectedItemCount();
    if (count < 2)
        return;

    long x = 0;
    wxString maxDate, minDate;
    double balance = 0;
    for (const auto& i : m_trans)
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

    int days = max_date.Subtract(min_date).GetDays();

    wxString msg;
    Model_Account::Data *account = Model_Account::instance().get(m_cp->m_AccountID);
    msg = wxString::Format(_("Transactions selected: %ld"), count);
    msg += "\n";
    msg += wxString::Format(_("Selected transactions balance: %s")
        , Model_Account::toCurrency(balance, account));
    msg += "\n";
    msg += wxString::Format(_("Days between selected transactions: %d"), days);

    m_cp->m_info_panel->SetLabelText(msg);

    m_cp->m_btnDuplicate->Enable(false);
    m_cp->m_btnAttachment->Enable(false);
}

//----------------------------------------------------------------------------
