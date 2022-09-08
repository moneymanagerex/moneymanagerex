/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2014 - 2022 Nikolay Akimov
 Copyright (C) 2021, 2022 Mark Whalley (mark@ipx.co.uk)

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
#include "billsdepositspanel.h"
#include "billsdepositsdialog.h"
#include "constants.h"
#include "images_list.h"

#include "model/allmodel.h"

enum
{
    MENU_TREEPOPUP_NEW = wxID_HIGHEST + 1300,
    MENU_TREEPOPUP_EDIT,
    MENU_TREEPOPUP_DUPLICATE,
    MENU_TREEPOPUP_DELETE,
    MENU_POPUP_BD_ENTER_OCCUR,
    MENU_POPUP_BD_SKIP_OCCUR,
    MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS,
    MENU_ON_SET_UDC0,
    MENU_ON_SET_UDC1,
    MENU_ON_SET_UDC2,
    MENU_ON_SET_UDC3,
    MENU_ON_SET_UDC4,
    MENU_ON_SET_UDC5,
    MENU_ON_SET_UDC6,
    MENU_ON_SET_UDC7
};

const wxString BILLSDEPOSITS_REPEATS[] =
{
    wxTRANSLATE("None"),
    wxTRANSLATE("Weekly"),
    wxTRANSLATE("Fortnightly"),
    wxTRANSLATE("Monthly"),
    wxTRANSLATE("Every 2 Months"),
    wxTRANSLATE("Quarterly"),
    wxTRANSLATE("Half-Yearly"),
    wxTRANSLATE("Yearly"),
    wxTRANSLATE("Four Months"),
    wxTRANSLATE("Four Weeks"),
    wxTRANSLATE("Daily"),
    wxTRANSLATE("In %s Days"),
    wxTRANSLATE("In %s Months"),
    wxTRANSLATE("Every %s Days"),
    wxTRANSLATE("Every %s Months"),
    wxTRANSLATE("Monthly (last day)"),
    wxTRANSLATE("Monthly (last business day)")

};

/*******************************************************/
wxBEGIN_EVENT_TABLE(mmBillsDepositsPanel, wxPanel)
    EVT_BUTTON(wxID_NEW, mmBillsDepositsPanel::OnNewBDSeries)
    EVT_BUTTON(wxID_EDIT, mmBillsDepositsPanel::OnEditBDSeries)
    EVT_BUTTON(wxID_DUPLICATE, mmBillsDepositsPanel::OnDuplicateBDSeries)    
    EVT_BUTTON(wxID_DELETE, mmBillsDepositsPanel::OnDeleteBDSeries)
    EVT_BUTTON(wxID_PASTE, mmBillsDepositsPanel::OnEnterBDTransaction)
    EVT_BUTTON(wxID_IGNORE, mmBillsDepositsPanel::OnSkipBDTransaction)
    EVT_BUTTON(wxID_FILE, mmBillsDepositsPanel::OnOpenAttachment)
    EVT_BUTTON(wxID_FILE2, mmBillsDepositsPanel::OnFilterTransactions)
wxEND_EVENT_TABLE()
/*******************************************************/
wxBEGIN_EVENT_TABLE(billsDepositsListCtrl, mmListCtrl)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY,   billsDepositsListCtrl::OnListItemActivated)
    EVT_RIGHT_DOWN(billsDepositsListCtrl::OnItemRightClick)
    EVT_LEFT_DOWN(billsDepositsListCtrl::OnListLeftClick)
    EVT_LIST_ITEM_SELECTED(wxID_ANY, billsDepositsListCtrl::OnListItemSelected)

    EVT_MENU(MENU_TREEPOPUP_NEW,              billsDepositsListCtrl::OnNewBDSeries)
    EVT_MENU(MENU_TREEPOPUP_EDIT,             billsDepositsListCtrl::OnEditBDSeries)
    EVT_MENU(MENU_TREEPOPUP_DUPLICATE,        billsDepositsListCtrl::OnDuplicateBDSeries)
    EVT_MENU(MENU_TREEPOPUP_DELETE,           billsDepositsListCtrl::OnDeleteBDSeries)
    EVT_MENU(MENU_POPUP_BD_ENTER_OCCUR,       billsDepositsListCtrl::OnEnterBDTransaction)
    EVT_MENU(MENU_POPUP_BD_SKIP_OCCUR,        billsDepositsListCtrl::OnSkipBDTransaction)
    EVT_MENU(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, billsDepositsListCtrl::OnOrganizeAttachments)
    EVT_MENU_RANGE(MENU_ON_SET_UDC0, MENU_ON_SET_UDC7, billsDepositsListCtrl::OnSetUserColour)

    EVT_LIST_KEY_DOWN(wxID_ANY,   billsDepositsListCtrl::OnListKeyDown)
wxEND_EVENT_TABLE()
/*******************************************************/

billsDepositsListCtrl::billsDepositsListCtrl(mmBillsDepositsPanel* bdp, wxWindow *parent, wxWindowID winid)
: mmListCtrl(parent, winid)
, m_bdp(bdp)
{
    mmThemeMetaColour(this, meta::COLOR_LISTPANEL);

    const wxAcceleratorEntry entries[] =
    {
        wxAcceleratorEntry(wxACCEL_CTRL, 'N', MENU_TREEPOPUP_NEW),   
        wxAcceleratorEntry(wxACCEL_CTRL, 'E', MENU_TREEPOPUP_EDIT), 
        wxAcceleratorEntry(wxACCEL_CTRL, 'U', MENU_TREEPOPUP_DUPLICATE), 
        wxAcceleratorEntry(wxACCEL_CTRL, 'D', MENU_TREEPOPUP_DELETE),               
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

    // load the global variables
    m_selected_col = Model_Setting::instance().GetIntSetting("BD_SORT_COL", m_bdp->col_sort());
    m_asc = Model_Setting::instance().GetBoolSetting("BD_ASC", true);

    m_columns.push_back(PANEL_COLUMN(" ", 25, wxLIST_FORMAT_LEFT));
    m_columns.push_back(PANEL_COLUMN(_("ID"), wxLIST_AUTOSIZE, wxLIST_FORMAT_RIGHT));
    m_columns.push_back(PANEL_COLUMN(_("Date Paid"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT));
    m_columns.push_back(PANEL_COLUMN(_("Date Due"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT));
    m_columns.push_back(PANEL_COLUMN(_("Account"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT));
    m_columns.push_back(PANEL_COLUMN(_("Payee"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT));
    m_columns.push_back(PANEL_COLUMN(_("Status"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT));
    m_columns.push_back(PANEL_COLUMN(_("Category"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT));
    m_columns.push_back(PANEL_COLUMN(_("Type"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT));
    m_columns.push_back(PANEL_COLUMN(_("Amount"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT));
    m_columns.push_back(PANEL_COLUMN(_("Frequency"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT));
    m_columns.push_back(PANEL_COLUMN(_("Repetitions"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT));
    m_columns.push_back(PANEL_COLUMN(_("Autorepeat"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT));
    m_columns.push_back(PANEL_COLUMN(_("Payment"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT));
    m_columns.push_back(PANEL_COLUMN(_("Number"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT));
    m_columns.push_back(PANEL_COLUMN(_("Notes"), 150, wxLIST_FORMAT_LEFT));

    m_col_width = "BD_COL%d_WIDTH";
    m_default_sort_column = m_bdp->col_sort();
    
    for (const auto& entry : m_columns)
    {
        int count = GetColumnCount();
        InsertColumn(count
            , entry.HEADER
            , entry.FORMAT
            , Model_Setting::instance().GetIntSetting(wxString::Format(m_col_width, count), entry.WIDTH));
    }
}

billsDepositsListCtrl::~billsDepositsListCtrl()
{
    wxLogDebug("Exit billsDepositsListCtrl");
}

void billsDepositsListCtrl::OnColClick(wxListEvent& event)
{
    int ColumnNr;
    if (event.GetId() != MENU_HEADER_SORT)
        ColumnNr = event.GetColumn();
    else
        ColumnNr = m_ColumnHeaderNbr;
    if (0 > ColumnNr || ColumnNr >= m_bdp->getColumnsNumber() || ColumnNr == 0) return;

    if (m_selected_col == ColumnNr && event.GetId() != MENU_HEADER_SORT) m_asc = !m_asc;

    wxListItem item;
    item.SetMask(wxLIST_MASK_IMAGE);
    item.SetImage(-1);
    SetColumn(m_selected_col, item);

    m_selected_col = ColumnNr;

    Model_Setting::instance().Set("BD_ASC", m_asc);
    Model_Setting::instance().Set("BD_SORT_COL", m_selected_col);

    if (m_selected_row >= 0) 
        refreshVisualList(m_bdp->initVirtualListControl(m_bdp->bills_[m_selected_row].BDID));
    else
        refreshVisualList(m_bdp->initVirtualListControl(-1));
}

mmBillsDepositsPanel::mmBillsDepositsPanel(wxWindow *parent, wxWindowID winid
    , const wxPoint& pos, const wxSize& size, long style, const wxString& name)
{
    m_today = wxDate::Today();
    this->tips_.Add(_("MMEX allows regular payments to be set up as transactions. These transactions can also be regular deposits,"
        " or transfers that will occur at some future time. These transactions act as a reminder that an event is about to occur,"
        " and appears on the Home Page 14 days before the transaction is due. "));
    this->tips_.Add(_("Tip: These transactions can be set up to activate - allowing the user to adjust any values on the due date."));

    Create(parent, winid, pos, size, style, name);
}

bool mmBillsDepositsPanel::Create(wxWindow *parent
    , wxWindowID winid, const wxPoint& pos
    , const wxSize& size, long style, const wxString& name)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create(parent, winid, pos, size, style, name);

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    /* Set up the transaction filter.  The transFilter dialog will be destroyed
       when the checking panel is destroyed. */
    transFilterActive_ = false;
    transFilterDlg_ = new mmFilterTransactionsDialog(this, -1, false);

    initVirtualListControl();

    Model_Usage::instance().pageview(this);

    return TRUE;
}

mmBillsDepositsPanel::~mmBillsDepositsPanel()
{
}

void mmBillsDepositsPanel::CreateControls()
{
    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer9);

    /* ---------------------- */
    wxPanel* headerPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition
        , wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);
    itemBoxSizer9->Add(headerPanel, g_flagsBorder1V);

    wxBoxSizer* itemBoxSizerVHeader = new wxBoxSizer(wxVERTICAL);
    headerPanel->SetSizer(itemBoxSizerVHeader);

    wxStaticText* itemStaticText9 = new wxStaticText(headerPanel, wxID_ANY
        , _("Recurring Transactions"));
    itemStaticText9->SetFont(this->GetFont().Larger().Bold());
    itemBoxSizerVHeader->Add(itemStaticText9, g_flagsBorder1V);

    wxBoxSizer* itemBoxSizerHHeader2 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizerVHeader->Add(itemBoxSizerHHeader2);

    m_bitmapTransFilter = new wxButton(headerPanel, wxID_FILE2);
    m_bitmapTransFilter->SetBitmap(mmBitmap(png::TRANSFILTER, mmBitmapButtonSize));
    m_bitmapTransFilter->SetLabel(_("Transaction Filter"));
    itemBoxSizerHHeader2->Add(m_bitmapTransFilter, g_flagsBorder1H);

    /* ---------------------- */
    wxSplitterWindow* itemSplitterWindowBillsDeposit = new wxSplitterWindow(this
        , wxID_ANY, wxDefaultPosition, wxSize(200, 200)
        , wxSP_3DBORDER | wxSP_3DSASH | wxNO_BORDER);

    m_imageList = createImageList();
    m_imageList->Add(mmBitmap(png::FOLLOW_UP));
    m_imageList->Add(mmBitmap(png::RUN_AUTO));
    m_imageList->Add(mmBitmap(png::RUN));
    m_imageList->Add(mmBitmap(png::UPARROW));
    m_imageList->Add(mmBitmap(png::DOWNARROW));

    listCtrlAccount_ = new billsDepositsListCtrl(this, itemSplitterWindowBillsDeposit);

    listCtrlAccount_->SetImageList(m_imageList.get(), wxIMAGE_LIST_SMALL);

    wxPanel* bdPanel = new wxPanel(itemSplitterWindowBillsDeposit, wxID_ANY
        , wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);
    mmThemeMetaColour(bdPanel, meta::COLOR_LISTPANEL);

    itemSplitterWindowBillsDeposit->SplitHorizontally(listCtrlAccount_, bdPanel);
    itemSplitterWindowBillsDeposit->SetMinimumPaneSize(100);
    itemSplitterWindowBillsDeposit->SetSashGravity(1.0);
    itemBoxSizer9->Add(itemSplitterWindowBillsDeposit, g_flagsExpandBorder1);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    bdPanel->SetSizer(itemBoxSizer4);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer4->Add(itemBoxSizer5, g_flagsBorder1V);

    wxButton* itemButtonNew = new wxButton(bdPanel, wxID_NEW, _("&New "));
    mmToolTip(itemButtonNew, _("New Recurring Transaction"));
    itemBoxSizer5->Add(itemButtonNew, 0, wxRIGHT, 5);

    wxButton* itemButton81 = new wxButton(bdPanel, wxID_EDIT, _("&Edit "));
    mmToolTip(itemButton81, _("Edit Recurring Transaction"));
    itemBoxSizer5->Add(itemButton81, 0, wxRIGHT, 5);
    itemButton81->Enable(false);

    wxButton* itemButton82 = new wxButton(bdPanel, wxID_DUPLICATE, _("D&uplicate "));
    mmToolTip(itemButton82, _("Duplicate Recurring Transaction"));
    itemBoxSizer5->Add(itemButton82, 0, wxRIGHT, 5);
    itemButton82->Enable(false);

    wxButton* itemButton7 = new wxButton(bdPanel, wxID_DELETE, _("&Delete "));
    mmToolTip(itemButton7, _("Delete Recurring Transaction"));
    itemBoxSizer5->Add(itemButton7, 0, wxRIGHT, 5);
    itemButton7->Enable(false);

    wxButton* itemButton8 = new wxButton(bdPanel, wxID_PASTE, _("Ente&r"));
    mmToolTip(itemButton8, _("Enter Next Recurring Transaction Occurrence"));
    itemBoxSizer5->Add(itemButton8, 0, wxRIGHT, 5);
    itemButton8->Enable(false);

    wxButton* buttonSkipTrans = new wxButton(bdPanel, wxID_IGNORE, _("&Skip"));
    mmToolTip(buttonSkipTrans, _("Skip Next Recurring Transaction Occurrence"));
    itemBoxSizer5->Add(buttonSkipTrans, 0, wxRIGHT, 5);
    buttonSkipTrans->Enable(false);

    wxBitmapButton* btnAttachment_ = new wxBitmapButton(bdPanel, wxID_FILE
        , mmBitmap(png::CLIP, mmBitmapButtonSize), wxDefaultPosition
        , wxSize(30, itemButton8->GetSize().GetY()));
    mmToolTip(btnAttachment_, _("Open attachments"));
    itemBoxSizer5->Add(btnAttachment_, 0, wxRIGHT, 5);
    btnAttachment_->Enable(false);

    //Infobar-mini
    m_infoTextMini = new wxStaticText(bdPanel, wxID_STATIC, "");
    itemBoxSizer5->Add(m_infoTextMini, 1, wxGROW | wxTOP | wxLEFT, 5);

    //Infobar
    m_infoText = new wxStaticText(bdPanel, wxID_ANY, ""
        , wxPoint(-1, -1), wxSize(200, -1), wxNO_BORDER | wxTE_MULTILINE | wxTE_WORDWRAP | wxST_NO_AUTORESIZE);
    itemBoxSizer4->Add(m_infoText, g_flagsExpandBorder1);

    mmBillsDepositsPanel::updateBottomPanelData(-1);
}

int mmBillsDepositsPanel::initVirtualListControl(int id)
{
    listCtrlAccount_->DeleteAllItems();

    wxListItem item;
    item.SetMask(wxLIST_MASK_IMAGE);
    item.SetImage(listCtrlAccount_->m_asc ? ICON_UPARROW : ICON_DOWNARROW);
    listCtrlAccount_->SetColumn(listCtrlAccount_->m_selected_col, item);

    bills_.clear();
    const auto split = Model_Budgetsplittransaction::instance().get_all();
    for (const Model_Billsdeposits::Data& data
        : Model_Billsdeposits::instance().all(Model_Billsdeposits::COL_NEXTOCCURRENCEDATE))
    {
        if (transFilterActive_ && !transFilterDlg_->mmIsRecordMatches(data, split))
            continue;

        Model_Billsdeposits::Full_Data r(data);
        bills_.push_back(r);
    }

    sortTable();

    int cnt = 0, selected_item = -1;
    for (const auto& entry: bills_)
    {
        if (id == entry.BDID)
        {
            selected_item = cnt;
            break;
        }
        ++cnt;
    }

    listCtrlAccount_->SetItemCount(static_cast<long>(bills_.size()));
    return selected_item;
}

void mmBillsDepositsPanel::OnNewBDSeries(wxCommandEvent& event)
{
  listCtrlAccount_->OnNewBDSeries(event);
}

void mmBillsDepositsPanel::OnEditBDSeries(wxCommandEvent& event)
{
    listCtrlAccount_->OnEditBDSeries(event);
}

void mmBillsDepositsPanel::OnDuplicateBDSeries(wxCommandEvent& event)
{
    listCtrlAccount_->OnDuplicateBDSeries(event);
}

void mmBillsDepositsPanel::OnDeleteBDSeries(wxCommandEvent& event)
{
    listCtrlAccount_->OnDeleteBDSeries(event);
}

void mmBillsDepositsPanel::OnEnterBDTransaction(wxCommandEvent& event)
{
    listCtrlAccount_->OnEnterBDTransaction(event);
}

void mmBillsDepositsPanel::OnSkipBDTransaction(wxCommandEvent& event)
{
    listCtrlAccount_->OnSkipBDTransaction(event);
    listCtrlAccount_->SetFocus();
}

void mmBillsDepositsPanel::OnOpenAttachment(wxCommandEvent& event)
{
    listCtrlAccount_->OnOpenAttachment(event);
    listCtrlAccount_->SetFocus();
}

/*******************************************************/
void billsDepositsListCtrl::OnItemRightClick(wxMouseEvent& event)
{
    if (m_selected_row > -1)
        SetItemState(m_selected_row, 0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
    int Flags = wxLIST_HITTEST_ONITEM;
    m_selected_row = HitTest(wxPoint(event.m_x, event.m_y), Flags);

    if (m_selected_row >= 0)
    {
        SetItemState(m_selected_row, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        SetItemState(m_selected_row, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
    }
    m_bdp->updateBottomPanelData(m_selected_row);
    bool item_active = (m_selected_row >= 0);
    wxMenu menu;
    menu.Append(MENU_POPUP_BD_ENTER_OCCUR, __(wxTRANSLATE("Enter next Occurrence")));
    menu.AppendSeparator();
    menu.Append(MENU_POPUP_BD_SKIP_OCCUR, _("Skip next Occurrence"));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_NEW, __(wxTRANSLATE("&New Recurring Transaction")));
    menu.Append(MENU_TREEPOPUP_EDIT, __(wxTRANSLATE("&Edit Recurring Transaction")));
    menu.Append(MENU_TREEPOPUP_DUPLICATE, __(wxTRANSLATE("D&uplicate Recurring Transaction")));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_DELETE, __(wxTRANSLATE("&Delete Recurring Transaction")));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, __(wxTRANSLATE("&Organize Attachments")));
    
    menu.Enable(MENU_POPUP_BD_ENTER_OCCUR, item_active);
    menu.Enable(MENU_POPUP_BD_SKIP_OCCUR, item_active);
    menu.Enable(MENU_TREEPOPUP_EDIT, item_active);
    menu.Enable(MENU_TREEPOPUP_DUPLICATE, item_active);
    menu.Enable(MENU_TREEPOPUP_DELETE, item_active);
    menu.Enable(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, item_active);

    PopupMenu(&menu, event.GetPosition());
    this->SetFocus();
}

wxString mmBillsDepositsPanel::getItem(long item, long column)
{
    const Model_Billsdeposits::Full_Data& bill = this->bills_.at(item);
    switch (column)
    {
    case COL_ID:
        return wxString::Format("%i", bill.BDID).Trim();
    case COL_PAYMENT_DATE:
        return mmGetDateForDisplay(bill.TRANSDATE);
    case COL_DUE_DATE:
        return mmGetDateForDisplay(bill.NEXTOCCURRENCEDATE);
    case COL_ACCOUNT:
        return bill.ACCOUNTNAME;
    case COL_PAYEE:
        return bill.real_payee_name();
    case COL_STATUS:
        return bill.STATUS;
    case COL_CATEGORY:
        return bill.CATEGNAME;
    case COL_TYPE:
        return wxGetTranslation(bill.TRANSCODE);
    case COL_AMOUNT:
        return Model_Account::toCurrency(bill.TRANSAMOUNT, Model_Account::instance().get(bill.ACCOUNTID));
    case COL_FREQUENCY:
        return GetFrequency(&bill);
    case COL_REPEATS:
        if (bill.NUMOCCURRENCES == -1)
            return L"\x221E";
        else
            return wxString::Format("%i", bill.NUMOCCURRENCES).Trim();
    case COL_AUTO:
    {
        int repeats = bill.REPEATS;
        wxString repeatSTR = _("Manual");
        if (repeats >= BD_REPEATS_MULTIPLEX_BASE)
        {
            repeats -= BD_REPEATS_MULTIPLEX_BASE;
            repeatSTR = _("Suggested");
            if (repeats >= BD_REPEATS_MULTIPLEX_BASE)
            {
                repeats -= BD_REPEATS_MULTIPLEX_BASE;
                repeatSTR = _("Automated");
            }
        }
        return repeatSTR;
    }
    case COL_DAYS:
        return GetRemainingDays(&bill);
    case COL_NUMBER:
        return bill.TRANSACTIONNUMBER;
    case COL_NOTES:
    {
        wxString value = bill.NOTES;
        value.Replace("\n", " ");
        if (Model_Attachment::NrAttachments(Model_Attachment::reftype_desc(Model_Attachment::BILLSDEPOSIT), bill.BDID))
            value.Prepend(mmAttachmentManage::GetAttachmentNoteSign());
        return value;
    }
    default:
        return wxEmptyString;
    }
}

const wxString mmBillsDepositsPanel::GetFrequency(const Model_Billsdeposits::Data* item) const
{
    int repeats = item->REPEATS;
    // DeMultiplex the Auto Executable fields.
    if (repeats >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute User Acknowlegement required
        repeats -= BD_REPEATS_MULTIPLEX_BASE;
    if (repeats >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute Silent mode
        repeats -= BD_REPEATS_MULTIPLEX_BASE;

    wxString text = wxGetTranslation(BILLSDEPOSITS_REPEATS[repeats]);
    if (repeats > 10 && repeats < 15)
        text = wxString::Format(text, wxString::Format("%d", item->NUMOCCURRENCES));
    return text;
}

const wxString mmBillsDepositsPanel::GetRemainingDays(const Model_Billsdeposits::Data* item) const
{
    int repeats = item->REPEATS;
    // DeMultiplex the Auto Executable fields.
    if (repeats >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute User Acknowlegement required
        repeats -= BD_REPEATS_MULTIPLEX_BASE;
    if (repeats >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute Silent mode
        repeats -= BD_REPEATS_MULTIPLEX_BASE;

    int daysRemaining = Model_Billsdeposits::TRANSDATE(item)
        .Subtract(this->getToday()).GetDays();
    int daysOverdue = Model_Billsdeposits::NEXTOCCURRENCEDATE(item)
        .Subtract(this->getToday()).GetDays();
    wxString text = wxString::Format(wxPLURAL("%d day remaining", "%d days remaining", daysRemaining), daysRemaining);

    if (daysRemaining == 0)
    {
        if (((repeats > 10) && (repeats < 15)) && (item->NUMOCCURRENCES < 0))
            text = _("Inactive");
    }

    if (daysRemaining < 0)
    {
        text = wxString::Format(wxPLURAL("%d day delay!", "%d days delay!", -daysRemaining), -daysRemaining);
        if (((repeats > 10) && (repeats < 15)) && (item->NUMOCCURRENCES < 0))
            text = _("Inactive");
    }

    if (daysOverdue < 0)
    {
        text = wxString::Format(wxPLURAL("%d day overdue!", "%d days overdue!", -daysOverdue), -daysOverdue);
        if (((repeats > 10) && (repeats < 15)) && (item->NUMOCCURRENCES < 0))
            text = _("Inactive");
    }

    return text;
}

wxString billsDepositsListCtrl::OnGetItemText(long item, long column) const
{
    return m_bdp->getItem(item, column);
}

void billsDepositsListCtrl::OnListItemSelected(wxListEvent& event)
{
    m_selected_row = event.GetIndex();
    m_bdp->updateBottomPanelData(m_selected_row);
}

void billsDepositsListCtrl::OnListLeftClick(wxMouseEvent& event)
{
    int Flags = wxLIST_HITTEST_ONITEM;
    long index = HitTest(wxPoint(event.m_x, event.m_y), Flags);
    if (index == -1)
    {
        m_selected_row = -1;
        m_bdp->updateBottomPanelData(m_selected_row);
    }
    event.Skip();
}

int billsDepositsListCtrl::OnGetItemImage(long item) const
{
    bool bd_repeat_user = false;
    bool bd_repeat_auto = false;
    int repeats = m_bdp->bills_[item].REPEATS;
    // DeMultiplex the Auto Executable fields.
    if (repeats >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute User Acknowlegement required
    {
        repeats -= BD_REPEATS_MULTIPLEX_BASE;
        bd_repeat_user = true;
    }
    if (repeats >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute Silent mode
    {
        repeats -= BD_REPEATS_MULTIPLEX_BASE;
        bd_repeat_auto = true;
    }

    int daysRemaining = Model_Billsdeposits::NEXTOCCURRENCEDATE(m_bdp->bills_[item])
        .Subtract(m_bdp->getToday()).GetDays();
    wxString daysRemainingStr = wxString::Format(wxPLURAL("%d day remaining", "%d days remaining", daysRemaining), daysRemaining);

    if (daysRemaining == 0)
    {
        if (((repeats > 10) && (repeats < 15)) && (m_bdp->bills_[item].NUMOCCURRENCES < 0))
            daysRemainingStr = _("Inactive");
    }

    if (daysRemaining < 0)
    {
        daysRemainingStr = wxString::Format(wxPLURAL("%d day overdue!", "%d days overdue!", std::abs(daysRemaining)), std::abs(daysRemaining));
        if (((repeats > 10) && (repeats < 15)) && (m_bdp->bills_[item].NUMOCCURRENCES < 0))
            daysRemainingStr = _("Inactive");
    }

    /* Returns the icon to be shown for each entry */
    if (daysRemainingStr == _("Inactive")) return -1;
    if (daysRemaining < 0) return mmBillsDepositsPanel::ICON_FOLLOWUP;
    if (bd_repeat_auto) return mmBillsDepositsPanel::ICON_RUN_AUTO;
    if (bd_repeat_user) return mmBillsDepositsPanel::ICON_RUN;

    return -1;
}

void billsDepositsListCtrl::OnListKeyDown(wxListEvent& event)
{
  switch ( event.GetKeyCode() )
    {
        case WXK_DELETE:
            {
                wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED,
                    MENU_TREEPOPUP_DELETE);
                OnDeleteBDSeries(evt);
            }
            break;

        default:
            event.Skip();
    }
}

void billsDepositsListCtrl::OnNewBDSeries(wxCommandEvent& /*event*/)
{
    mmBDDialog dlg(this, 0, false, false);
    if ( dlg.ShowModal() == wxID_OK )
        refreshVisualList(m_bdp->initVirtualListControl(dlg.GetTransID()));
}

void billsDepositsListCtrl::OnEditBDSeries(wxCommandEvent& /*event*/)
{
    if (m_selected_row == -1) return;

    mmBDDialog dlg(this, m_bdp->bills_[m_selected_row].BDID, false, false);
    if ( dlg.ShowModal() == wxID_OK )
        refreshVisualList(m_bdp->initVirtualListControl(dlg.GetTransID()));
}

void billsDepositsListCtrl::OnDuplicateBDSeries(wxCommandEvent& /*event*/)
{
    if (m_selected_row == -1) return;

    mmBDDialog dlg(this, m_bdp->bills_[m_selected_row].BDID, true, false);
    if ( dlg.ShowModal() == wxID_OK )
        refreshVisualList(m_bdp->initVirtualListControl(dlg.GetTransID()));
}

void billsDepositsListCtrl::OnDeleteBDSeries(wxCommandEvent& WXUNUSED(event))
{
    if (m_bdp->bills_.empty()) return;
    if (m_selected_row < 0) return;

    wxMessageDialog msgDlg(this, _("Do you really want to delete the recurring transaction?")
        , _("Confirm Deletion")
        , wxYES_NO | wxNO_DEFAULT | wxICON_ERROR);
    if (msgDlg.ShowModal() == wxID_YES)
    {
        int BdId = m_bdp->bills_[m_selected_row].BDID;
        Model_Billsdeposits::instance().remove(BdId);
        mmAttachmentManage::DeleteAllAttachments(Model_Attachment::reftype_desc(Model_Attachment::BILLSDEPOSIT), BdId);
        m_bdp->do_delete_custom_values(-BdId);
        m_bdp->initVirtualListControl();
        refreshVisualList(m_selected_row);
    }
}

void billsDepositsListCtrl::OnEnterBDTransaction(wxCommandEvent& /*event*/)
{
    if (m_selected_row == -1) return;

    int id = m_bdp->bills_[m_selected_row].BDID;
    mmBDDialog dlg(this, id, false, true);
    if ( dlg.ShowModal() == wxID_OK )
    {
        if (++m_selected_row < m_bdp->bills_.size())
            id = m_bdp->bills_[m_selected_row].BDID;
        refreshVisualList(m_bdp->initVirtualListControl(id));
    }
}

void billsDepositsListCtrl::OnSkipBDTransaction(wxCommandEvent& /*event*/)
{
    if (m_selected_row == -1) return;

    int id = m_bdp->bills_[m_selected_row].BDID;
    Model_Billsdeposits::instance().completeBDInSeries(id);
    if (++m_selected_row < m_bdp->bills_.size())
        id = m_bdp->bills_[m_selected_row].BDID;
    refreshVisualList(m_bdp->initVirtualListControl(id));
}

void billsDepositsListCtrl::OnOrganizeAttachments(wxCommandEvent& /*event*/)
{
    if (m_selected_row == -1) return;

    int RefId = m_bdp->bills_[m_selected_row].BDID;
    const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::BILLSDEPOSIT);

    mmAttachmentDialog dlg(this, RefType, RefId);
    dlg.ShowModal();

    refreshVisualList(m_bdp->initVirtualListControl(RefId));
}

void billsDepositsListCtrl::OnOpenAttachment(wxCommandEvent& WXUNUSED(event))
{
    if (m_selected_row == -1) return;
    int RefId = m_bdp->bills_[m_selected_row].BDID;
    const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::BILLSDEPOSIT);

    mmAttachmentManage::OpenAttachmentFromPanelIcon(this, RefType, RefId);
    refreshVisualList(m_bdp->initVirtualListControl(RefId));
}

void billsDepositsListCtrl::OnListItemActivated(wxListEvent& WXUNUSED(event))
{
    if (m_selected_row == -1) return;

    mmBDDialog dlg(this, m_bdp->bills_[m_selected_row].BDID, false, false);
    if ( dlg.ShowModal() == wxID_OK )
        refreshVisualList(m_bdp->initVirtualListControl(dlg.GetTransID()));
}

void mmBillsDepositsPanel::updateBottomPanelData(int selIndex)
{
    enableEditDeleteButtons(selIndex >= 0);
    if (selIndex != -1)
    {
        m_infoTextMini->SetLabelText(Model_Category::full_name(bills_[selIndex].CATEGID, bills_[selIndex].SUBCATEGID));
        m_infoText->SetLabelText(bills_[selIndex].NOTES);
    }
}

void mmBillsDepositsPanel::enableEditDeleteButtons(bool en)
{
    wxButton* bE = static_cast<wxButton*>(FindWindow(wxID_EDIT));
    wxButton* bD = static_cast<wxButton*>(FindWindow(wxID_DELETE));
    wxButton* bDup = static_cast<wxButton*>(FindWindow(wxID_DUPLICATE));
    wxButton* bN = static_cast<wxButton*>(FindWindow(wxID_PASTE));
    wxButton* bS = static_cast<wxButton*>(FindWindow(wxID_IGNORE));
    wxButton* bA = static_cast<wxButton*>(FindWindow(wxID_FILE));
    if (bE) bE->Enable(en);
    if (bD) bD->Enable(en);
    if (bDup) bDup->Enable(en);
    if (bN) bN->Enable(en);
    if (bS) bS->Enable(en);
    if (bA) bA->Enable(en);

    m_infoText->SetLabelText(this->tips());
    m_infoTextMini->ClearBackground();
}

void mmBillsDepositsPanel::sortTable()
{
    std::sort(bills_.begin(), bills_.end());
    switch (listCtrlAccount_->m_selected_col)
    {
    case COL_ID:
        std::stable_sort(bills_.begin(), bills_.end(), SorterByBDID());
        break;
    case COL_PAYMENT_DATE:
        std::stable_sort(bills_.begin(), bills_.end(), SorterByTRANSDATE());
        break;
    case COL_DUE_DATE:
        std::stable_sort(bills_.begin(), bills_.end(), SorterByNEXTOCCURRENCEDATE());
        break;
    case COL_ACCOUNT:
        std::stable_sort(bills_.begin(), bills_.end(), SorterByACCOUNTNAME());
        break;
    case COL_PAYEE:
        std::stable_sort(bills_.begin(), bills_.end(), SorterByPAYEENAME());
        break;
    case COL_STATUS:
        std::stable_sort(bills_.begin(), bills_.end(), SorterBySTATUS());
        break;
    case COL_CATEGORY:
        std::stable_sort(bills_.begin(), bills_.end(), SorterByCATEGNAME());
        break;
    case COL_TYPE:
        std::stable_sort(bills_.begin(), bills_.end(), SorterByTRANSCODE());
        break;
    case COL_AMOUNT:
        std::stable_sort(bills_.begin(), bills_.end(), SorterByTRANSAMOUNT());
        break;
    case COL_FREQUENCY:
        std::stable_sort(bills_.begin(), bills_.end()
            , [&](const Model_Billsdeposits::Full_Data& x, const Model_Billsdeposits::Full_Data& y)
        {
            wxString x_text = this->GetFrequency(&x);
            wxString y_text = this->GetFrequency(&y);
            return x_text < y_text;
        });
        break;
    case COL_REPEATS:
        std::stable_sort(bills_.begin(), bills_.end(), SorterByREPEATS());
        break;
    case COL_DAYS:
        std::stable_sort(bills_.begin(), bills_.end()
            , [&](const Model_Billsdeposits::Data& x, const Model_Billsdeposits::Data& y)
        {
            bool x_useText = false;
            wxString x_text = this->GetRemainingDays(&x);
            long x_num = 0;
            if (isdigit(x_text[0]))
            {
                x_num = wxAtoi(x_text);
                wxString removed;
                if (x_text.EndsWith(wxString("!"), &removed))
                    x_num = -x_num;
            }
            else
                x_useText = true;

            bool y_useText = false;
            wxString y_text = this->GetRemainingDays(&y);
            long y_num = 0;
            if (isdigit(y_text[0]))
            {
                y_num = wxAtoi(y_text);
                wxString removed;
                if (y_text.EndsWith(wxString("!"), &removed))
                    y_num = -y_num;
            }
            else
                y_useText = true;
            return ((!x_useText && !y_useText) ? x_num < y_num : x_text < y_text);
        });
        break;
    case COL_NOTES:
        std::stable_sort(bills_.begin(), bills_.end(), SorterByNOTES());
        break;
    default:
        break;
    }
    if (!listCtrlAccount_->m_asc) std::reverse(bills_.begin(), bills_.end());
}

wxString mmBillsDepositsPanel::tips()
{
    return this->tips_[rand() % this->tips_.GetCount()];
}

void billsDepositsListCtrl::refreshVisualList(int selected_index)
{

    if (selected_index >= static_cast<long>(m_bdp->bills_.size()) || selected_index < 0)
        selected_index = - 1;
    if (!m_bdp->bills_.empty()) {
        RefreshItems(0, m_bdp->bills_.size() - 1);
    }
    else
        selected_index = -1;

    if (selected_index >= 0 && !m_bdp->bills_.empty())
    {
        SetItemState(selected_index, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        SetItemState(selected_index, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
        EnsureVisible(selected_index);
    }
    m_selected_row = selected_index;
    m_bdp->updateBottomPanelData(selected_index);
}

void billsDepositsListCtrl::RefreshList()
{
    if (m_bdp->bills_.size() == 0) return;
    int id = -1;
    if (m_selected_row != -1)
    {
        id = m_bdp->bills_[m_selected_row].BDID;
    }
    refreshVisualList(m_bdp->initVirtualListControl(id));
}

wxListItemAttr* billsDepositsListCtrl::OnGetItemAttr(long item) const
{
    if (item < 0 || item >= static_cast<int>(m_bdp->bills_.size())) return 0;

    int color_id = m_bdp->bills_[item].FOLLOWUPID;

    static std::map<int, wxSharedPtr<wxListItemAttr> > cache;
    if (color_id > 0)
    {
        color_id = std::min(7, color_id);
        const auto it = cache.find(color_id);
        if (it != cache.end())
            return it->second.get();
        else {
            switch (color_id)
            {
            case 1: cache[color_id] = new wxListItemAttr(*bestFontColour(mmColors::userDefColor1), mmColors::userDefColor1, wxNullFont); break;
            case 2: cache[color_id] = new wxListItemAttr(*bestFontColour(mmColors::userDefColor2), mmColors::userDefColor2, wxNullFont); break;
            case 3: cache[color_id] = new wxListItemAttr(*bestFontColour(mmColors::userDefColor3), mmColors::userDefColor3, wxNullFont); break;
            case 4: cache[color_id] = new wxListItemAttr(*bestFontColour(mmColors::userDefColor4), mmColors::userDefColor4, wxNullFont); break;
            case 5: cache[color_id] = new wxListItemAttr(*bestFontColour(mmColors::userDefColor5), mmColors::userDefColor5, wxNullFont); break;
            case 6: cache[color_id] = new wxListItemAttr(*bestFontColour(mmColors::userDefColor6), mmColors::userDefColor6, wxNullFont); break;
            case 7: cache[color_id] = new wxListItemAttr(*bestFontColour(mmColors::userDefColor7), mmColors::userDefColor7, wxNullFont); break;
            }
            return cache[color_id].get();
        }
    }

    /* Returns the alternating background pattern */
    return (item % 2) ? attr2_.get() : attr1_.get();
}

void billsDepositsListCtrl::OnSetUserColour(wxCommandEvent& event)
{
    if (m_selected_row == -1) return;
    int id = m_bdp->bills_[m_selected_row].BDID;

    int user_color_id = event.GetId();
    user_color_id -= MENU_ON_SET_UDC0;
    wxLogDebug("id: %i", user_color_id);

    Model_Billsdeposits::instance().Savepoint();

    Model_Billsdeposits::Data* item = Model_Billsdeposits::instance().get(id);
    if (item)
    {
        item->FOLLOWUPID = user_color_id;
        Model_Billsdeposits::instance().save(item);
    }
    Model_Billsdeposits::instance().ReleaseSavepoint();

    RefreshList();
}

void mmBillsDepositsPanel::RefreshList()
{
    listCtrlAccount_->RefreshList();
}

void mmBillsDepositsPanel::OnFilterTransactions(wxCommandEvent& WXUNUSED(event))
{

    if (transFilterDlg_->ShowModal() == wxID_OK && transFilterDlg_->mmIsSomethingChecked())
    {
        transFilterActive_ = true;
        m_bitmapTransFilter->SetBitmap(mmBitmap(png::TRANSFILTER_ACTIVE, mmBitmapButtonSize));
    }
    else 
    {
        transFilterActive_ = false;
        m_bitmapTransFilter->SetBitmap(mmBitmap(png::TRANSFILTER, mmBitmapButtonSize));
    }

    initVirtualListControl();
}

wxString  mmBillsDepositsPanel::BuildPage() const
{
    return listCtrlAccount_->BuildPage(_("Recurring Transactions")); 
}

void mmBillsDepositsPanel::do_delete_custom_values(int id)
{
    const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
    Model_CustomFieldData::DeleteAllData(RefType, id);
}
