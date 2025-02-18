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
    _n("Once"),
    _n("Weekly"),
    _n("Fortnightly"),
    _n("Monthly"),
    _n("Every 2 Months"),
    _n("Quarterly"),
    _n("Half-Yearly"),
    _n("Yearly"),
    _n("Four Months"),
    _n("Four Weeks"),
    _n("Daily"),
    _n("In %s Days"),
    _n("In %s Months"),
    _n("Every %s Days"),
    _n("Every %s Months"),
    _n("Monthly (last day)"),
    _n("Monthly (last business day)")
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

const std::vector<ListColumnInfo> billsDepositsListCtrl::col_info_all()
{
    return {
        { " ",              25,                        wxLIST_FORMAT_LEFT,  false },
        { _("ID"),          wxLIST_AUTOSIZE,           wxLIST_FORMAT_RIGHT, true },
        { _("Date Paid"),   wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT,  true },
        { _("Date Due"),    wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT,  true },
        { _("Account"),     wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT,  true },
        { _("Payee"),       wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT,  true },
        { _("Status"),      wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT,  true },
        { _("Category"),    wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT,  true },
        { _("Tags"),        200,                       wxLIST_FORMAT_LEFT,  true },
        { _("Type"),        wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT,  true },
        { _("Amount"),      wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT, true },
        { _("Frequency"),   wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT,  true },
        { _("Repetitions"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT, true },
        { _("Autorepeat"),  wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT,  true },
        { _("Payment"),     wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT,  true },
        { _("Number"),      wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT,  true },
        { _("Notes"),       150,                       wxLIST_FORMAT_LEFT,  true },
    };
}

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
    m_selected_col = Model_Setting::instance().getInt("BD_SORT_COL", col_sort());
    m_asc = Model_Setting::instance().getBool("BD_ASC", true);

    m_columns = col_info_all();
    for (int i = 0; i < LIST_COL_size; ++i)
        m_column_order.push_back(i);
    m_col_width_fmt = "BD_COL%d_WIDTH";
    m_col_type_str = "BD";
    m_default_sort_column = col_sort();

    createColumns();

}

billsDepositsListCtrl::~billsDepositsListCtrl()
{
    wxLogDebug("Exit billsDepositsListCtrl");
}

void billsDepositsListCtrl::OnColClick(wxListEvent& event)
{
    int ColumnNr;
    if (event.GetId() != MENU_HEADER_SORT && event.GetId() != MENU_HEADER_RESET)
        ColumnNr = event.GetColumn();
    else
        ColumnNr = m_ColumnHeaderNbr;
    if (0 > ColumnNr || ColumnNr >= m_bdp->getColumnsNumber() || ColumnNr == 0) return;

    if (m_selected_col == ColumnNr &&
        event.GetId() != MENU_HEADER_SORT && event.GetId() != MENU_HEADER_RESET
    ) m_asc = !m_asc;

    wxListItem item;
    item.SetMask(wxLIST_MASK_IMAGE);
    item.SetImage(-1);
    SetColumn(m_selected_col, item);

    m_selected_col = ColumnNr;

    Model_Setting::instance().setBool("BD_ASC", m_asc);
    Model_Setting::instance().setInt("BD_SORT_COL", m_selected_col);

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
        " and appears on the Dashboard 14 days before the transaction is due."));
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

    return true;
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
        , _("Scheduled Transactions"));
    itemStaticText9->SetFont(this->GetFont().Larger().Bold());
    itemBoxSizerVHeader->Add(itemStaticText9, g_flagsBorder1V);

    /* Disable feature to judge reaction :-)
       https://github.com/moneymanagerex/moneymanagerex/issues/5281

    wxBoxSizer* itemBoxSizerHHeader2 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizerVHeader->Add(itemBoxSizerHHeader2);


    m_bitmapTransFilter = new wxButton(headerPanel, wxID_FILE2);
    m_bitmapTransFilter->SetBitmap(mmBitmapBundle(png::TRANSFILTER, mmBitmapButtonSize));
    m_bitmapTransFilter->SetLabel(_("Transaction Filter"));
    itemBoxSizerHHeader2->Add(m_bitmapTransFilter, g_flagsBorder1H);
    */

    /* ---------------------- */
    wxSplitterWindow* itemSplitterWindowBillsDeposit = new wxSplitterWindow(this
        , wxID_ANY, wxDefaultPosition, wxSize(200, 200)
        , wxSP_3DBORDER | wxSP_3DSASH | wxNO_BORDER);

    wxVector<wxBitmapBundle> images;
    images.push_back(mmBitmapBundle(png::FOLLOW_UP));
    images.push_back(mmBitmapBundle(png::RUN_AUTO));
    images.push_back(mmBitmapBundle(png::RUN));
    images.push_back(mmBitmapBundle(png::UPARROW));
    images.push_back(mmBitmapBundle(png::DOWNARROW));

    listCtrlAccount_ = new billsDepositsListCtrl(this, itemSplitterWindowBillsDeposit);

    listCtrlAccount_->SetSmallImages(images);

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
    mmToolTip(itemButtonNew, _("New Scheduled Transaction"));
    itemBoxSizer5->Add(itemButtonNew, 0, wxRIGHT, 2);

    wxButton* itemButton81 = new wxButton(bdPanel, wxID_EDIT, _("&Edit "));
    mmToolTip(itemButton81, _("Edit Scheduled Transaction"));
    itemBoxSizer5->Add(itemButton81, 0, wxRIGHT, 2);
    itemButton81->Enable(false);

    wxButton* itemButton82 = new wxButton(bdPanel, wxID_DUPLICATE, _("D&uplicate "));
    mmToolTip(itemButton82, _("Duplicate Scheduled Transaction"));
    itemBoxSizer5->Add(itemButton82, 0, wxRIGHT, 2);
    itemButton82->Enable(false);

    wxButton* itemButton7 = new wxButton(bdPanel, wxID_DELETE, _("&Delete "));
    mmToolTip(itemButton7, _("Delete Scheduled Transaction"));
    itemBoxSizer5->Add(itemButton7, 0, wxRIGHT, 2);
    itemButton7->Enable(false);

    wxButton* itemButton8 = new wxButton(bdPanel, wxID_PASTE, _("Ente&r"));
    mmToolTip(itemButton8, _("Enter Next Scheduled Transaction Occurrence"));
    itemBoxSizer5->Add(itemButton8, 0, wxRIGHT, 2);
    itemButton8->Enable(false);

    wxButton* buttonSkipTrans = new wxButton(bdPanel, wxID_IGNORE, _("&Skip"));
    mmToolTip(buttonSkipTrans, _("Skip Next Scheduled Transaction Occurrence"));
    itemBoxSizer5->Add(buttonSkipTrans, 0, wxRIGHT, 2);
    buttonSkipTrans->Enable(false);

    wxBitmapButton* btnAttachment_ = new wxBitmapButton(bdPanel, wxID_FILE
        , mmBitmapBundle(png::CLIP, mmBitmapButtonSize), wxDefaultPosition
        , wxSize(30, itemButton8->GetSize().GetY()));
    mmToolTip(btnAttachment_, _("Open attachments"));
    itemBoxSizer5->Add(btnAttachment_, 0, wxRIGHT, 2);
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

int mmBillsDepositsPanel::initVirtualListControl(int64 id)
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
    menu.Append(MENU_POPUP_BD_ENTER_OCCUR, _tu("Enter next Occurrence…"));
    menu.AppendSeparator();
    menu.Append(MENU_POPUP_BD_SKIP_OCCUR, _("Skip next Occurrence"));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_NEW, _tu("&New Scheduled Transaction…"));
    menu.Append(MENU_TREEPOPUP_EDIT, _tu("&Edit Scheduled Transaction…"));
    menu.Append(MENU_TREEPOPUP_DUPLICATE, _tu("D&uplicate Scheduled Transaction…"));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_DELETE, _tu("&Delete Scheduled Transaction…"));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, _tu("&Organize Attachments…"));

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
    case billsDepositsListCtrl::LIST_COL_ID:
        return wxString::Format("%lld", bill.BDID).Trim();
    case billsDepositsListCtrl::LIST_COL_PAYMENT_DATE:
        return mmGetDateTimeForDisplay(bill.TRANSDATE);
    case billsDepositsListCtrl::LIST_COL_DUE_DATE:
        return mmGetDateTimeForDisplay(bill.NEXTOCCURRENCEDATE);
    case billsDepositsListCtrl::LIST_COL_ACCOUNT:
        return bill.ACCOUNTNAME;
    case billsDepositsListCtrl::LIST_COL_PAYEE:
        return bill.real_payee_name();
    case billsDepositsListCtrl::LIST_COL_STATUS:
        return bill.STATUS;
    case billsDepositsListCtrl::LIST_COL_CATEGORY:
        return bill.CATEGNAME;
    case billsDepositsListCtrl::LIST_COL_TAGS:
        return bill.TAGNAMES;
    case billsDepositsListCtrl::LIST_COL_TYPE:
        return wxGetTranslation(bill.TRANSCODE);
    case billsDepositsListCtrl::LIST_COL_AMOUNT:
        return Model_Account::toCurrency(bill.TRANSAMOUNT, Model_Account::instance().get(bill.ACCOUNTID));
    case billsDepositsListCtrl::LIST_COL_FREQUENCY:
        return GetFrequency(&bill);
    case billsDepositsListCtrl::LIST_COL_REPEATS:
    {
        int numRepeats = GetNumRepeats(&bill);
        if (numRepeats > 0)
            return wxString::Format("%i", numRepeats).Trim();
        else if (numRepeats == Model_Billsdeposits::REPEAT_NUM_INFINITY)
            return L"\x221E";  // INFITITY
        else
            return L"\x2015";  // HORIZONTAL BAR
    }
    case billsDepositsListCtrl::LIST_COL_AUTO:
    {
        int autoExecute = bill.REPEATS.GetValue() / BD_REPEATS_MULTIPLEX_BASE;
        wxString repeatSTR =
            (autoExecute == Model_Billsdeposits::REPEAT_AUTO_SILENT) ? _("Automated") :
            (autoExecute == Model_Billsdeposits::REPEAT_AUTO_MANUAL) ? _("Suggested") :
            _("Manual");
        return repeatSTR;
    }
    case billsDepositsListCtrl::LIST_COL_DAYS:
        return GetRemainingDays(&bill);
    case billsDepositsListCtrl::LIST_COL_NUMBER:
        return bill.TRANSACTIONNUMBER;
    case billsDepositsListCtrl::LIST_COL_NOTES:
    {
        wxString value = bill.NOTES;
        value.Replace("\n", " ");
        if (Model_Attachment::NrAttachments(Model_Attachment::REFTYPE_STR_BILLSDEPOSIT, bill.BDID))
            value.Prepend(mmAttachmentManage::GetAttachmentNoteSign());
        return value;
    }
    default:
        return wxEmptyString;
    }
}

const wxString mmBillsDepositsPanel::GetFrequency(const Model_Billsdeposits::Data* item) const
{
    int repeats = item->REPEATS.GetValue() % BD_REPEATS_MULTIPLEX_BASE; // DeMultiplex the Auto Executable fields.

    wxString text = wxGetTranslation(BILLSDEPOSITS_REPEATS[repeats]);
    if (repeats >= Model_Billsdeposits::REPEAT_IN_X_DAYS && repeats <= Model_Billsdeposits::REPEAT_EVERY_X_MONTHS)
        text = wxString::Format(text, wxString::Format("%lld", item->NUMOCCURRENCES));
    return text;
}

int mmBillsDepositsPanel::GetNumRepeats(const Model_Billsdeposits::Data* item) const
{
    int repeats = item->REPEATS.GetValue() % BD_REPEATS_MULTIPLEX_BASE; // DeMultiplex the Auto Executable fields.
    int numRepeats = item->NUMOCCURRENCES.GetValue();

    if (repeats == Model_Billsdeposits::REPEAT_ONCE)
        numRepeats = 1;
    else if (repeats >= Model_Billsdeposits::REPEAT_IN_X_DAYS && repeats <= Model_Billsdeposits::REPEAT_IN_X_MONTHS)
        numRepeats = numRepeats > 0 ? 2 : Model_Billsdeposits::REPEAT_NUM_UNKNOWN;
    else if (repeats >= Model_Billsdeposits::REPEAT_EVERY_X_DAYS && repeats <= Model_Billsdeposits::REPEAT_EVERY_X_MONTHS)
        numRepeats = numRepeats > 0 ? Model_Billsdeposits::REPEAT_NUM_INFINITY : Model_Billsdeposits::REPEAT_NUM_UNKNOWN;
    else if (numRepeats < -1)
    {
        wxFAIL;
        numRepeats = Model_Billsdeposits::REPEAT_NUM_UNKNOWN;
    }

    return numRepeats;
}

const wxString mmBillsDepositsPanel::GetRemainingDays(const Model_Billsdeposits::Data* item) const
{
    int repeats = item->REPEATS.GetValue() % BD_REPEATS_MULTIPLEX_BASE; // DeMultiplex the Auto Executable fields.
    if (repeats >= Model_Billsdeposits::REPEAT_IN_X_DAYS && repeats <= Model_Billsdeposits::REPEAT_EVERY_X_MONTHS && item->NUMOCCURRENCES < 0)
    {
        return _("Inactive");
    }
    
    int daysRemaining = Model_Billsdeposits::TRANSDATE(item)
        .Subtract(this->getToday()).GetSeconds().GetValue() / 86400;
    int daysOverdue = Model_Billsdeposits::NEXTOCCURRENCEDATE(item)
        .Subtract(this->getToday()).GetSeconds().GetValue() / 86400;

    wxString text =
        (daysOverdue < 0) ? wxString::Format(wxPLURAL("%d day overdue!", "%d days overdue!", -daysOverdue), -daysOverdue) :
        (daysRemaining < 0) ? wxString::Format(wxPLURAL("%d day delay!", "%d days delay!", -daysRemaining), -daysRemaining) :
        wxString::Format(wxPLURAL("%d day remaining", "%d days remaining", daysRemaining), daysRemaining);

    return text;
}

wxString billsDepositsListCtrl::OnGetItemText(long item, long column) const
{
    return m_bdp->getItem(item, m_column_order[column]);
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
    // demultiplex REPEATS
    int autoExecute = m_bdp->bills_[item].REPEATS.GetValue() / BD_REPEATS_MULTIPLEX_BASE;
    int repeats = m_bdp->bills_[item].REPEATS.GetValue() % BD_REPEATS_MULTIPLEX_BASE;
    if (repeats >= Model_Billsdeposits::REPEAT_IN_X_DAYS && repeats <= Model_Billsdeposits::REPEAT_EVERY_X_MONTHS && m_bdp->bills_[item].NUMOCCURRENCES < 0)
    {
        // inactive
        return -1;
    }

    int daysRemaining = Model_Billsdeposits::NEXTOCCURRENCEDATE(m_bdp->bills_[item])
        .Subtract(m_bdp->getToday()).GetSeconds().GetValue() / 86400;

    /* Returns the icon to be shown for each entry */
    if (daysRemaining < 0)
        return mmBillsDepositsPanel::ICON_FOLLOWUP;
    if (autoExecute == Model_Billsdeposits::REPEAT_AUTO_SILENT)
        return mmBillsDepositsPanel::ICON_RUN_AUTO;
    if (autoExecute == Model_Billsdeposits::REPEAT_AUTO_MANUAL)
        return mmBillsDepositsPanel::ICON_RUN;
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

    wxMessageDialog msgDlg(this, _("Do you want to delete the scheduled transaction?")
        , _("Confirm Deletion")
        , wxYES_NO | wxNO_DEFAULT | wxICON_ERROR);
    if (msgDlg.ShowModal() == wxID_YES)
    {
        int64 BdId = m_bdp->bills_[m_selected_row].BDID;
        Model_Billsdeposits::instance().remove(BdId);
        mmAttachmentManage::DeleteAllAttachments(Model_Attachment::REFTYPE_STR_BILLSDEPOSIT, BdId);
        m_bdp->do_delete_custom_values(-BdId);
        m_bdp->initVirtualListControl();
        refreshVisualList(m_selected_row);
    }
}

void billsDepositsListCtrl::OnEnterBDTransaction(wxCommandEvent& /*event*/)
{
    if (m_selected_row == -1) return;

    int64 id = m_bdp->bills_[m_selected_row].BDID;
    mmBDDialog dlg(this, id, false, true);
    if ( dlg.ShowModal() == wxID_OK )
    {
        if (++m_selected_row < long(m_bdp->bills_.size()))
            id = m_bdp->bills_[m_selected_row].BDID;
        refreshVisualList(m_bdp->initVirtualListControl(id));
    }
}

void billsDepositsListCtrl::OnSkipBDTransaction(wxCommandEvent& /*event*/)
{
    if (m_selected_row == -1) return;

    int64 id = m_bdp->bills_[m_selected_row].BDID;
    Model_Billsdeposits::instance().completeBDInSeries(id);
    if (++m_selected_row < long(m_bdp->bills_.size()))
        id = m_bdp->bills_[m_selected_row].BDID;
    refreshVisualList(m_bdp->initVirtualListControl(id));
}

void billsDepositsListCtrl::OnOrganizeAttachments(wxCommandEvent& /*event*/)
{
    if (m_selected_row == -1) return;

    int64 RefId = m_bdp->bills_[m_selected_row].BDID;
    const wxString& RefType = Model_Attachment::REFTYPE_STR_BILLSDEPOSIT;

    mmAttachmentDialog dlg(this, RefType, RefId);
    dlg.ShowModal();

    refreshVisualList(m_bdp->initVirtualListControl(RefId));
}

void billsDepositsListCtrl::OnOpenAttachment(wxCommandEvent& WXUNUSED(event))
{
    if (m_selected_row == -1) return;
    int64 RefId = m_bdp->bills_[m_selected_row].BDID;
    const wxString& RefType = Model_Attachment::REFTYPE_STR_BILLSDEPOSIT;

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
        m_infoTextMini->SetLabelText(Model_Category::full_name(bills_[selIndex].CATEGID));
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
    case billsDepositsListCtrl::LIST_COL_ID:
        std::stable_sort(bills_.begin(), bills_.end(), SorterByBDID());
        break;
    case billsDepositsListCtrl::LIST_COL_PAYMENT_DATE:
        std::stable_sort(bills_.begin(), bills_.end(), SorterByTRANSDATE());
        break;
    case billsDepositsListCtrl::LIST_COL_DUE_DATE:
        std::stable_sort(bills_.begin(), bills_.end(), SorterByNEXTOCCURRENCEDATE());
        break;
    case billsDepositsListCtrl::LIST_COL_ACCOUNT:
        std::stable_sort(bills_.begin(), bills_.end(), SorterByACCOUNTNAME());
        break;
    case billsDepositsListCtrl::LIST_COL_PAYEE:
        std::stable_sort(bills_.begin(), bills_.end(), SorterByPAYEENAME());
        break;
    case billsDepositsListCtrl::LIST_COL_STATUS:
        std::stable_sort(bills_.begin(), bills_.end(), SorterBySTATUS());
        break;
    case billsDepositsListCtrl::LIST_COL_CATEGORY:
        std::stable_sort(bills_.begin(), bills_.end(), SorterByCATEGNAME());
        break;
    case billsDepositsListCtrl::LIST_COL_TYPE:
        std::stable_sort(bills_.begin(), bills_.end(), SorterByTRANSCODE());
        break;
    case billsDepositsListCtrl::LIST_COL_AMOUNT:
        std::stable_sort(bills_.begin(), bills_.end(), SorterByTRANSAMOUNT());
        break;
    case billsDepositsListCtrl::LIST_COL_FREQUENCY:
        std::stable_sort(bills_.begin(), bills_.end()
            , [&](const Model_Billsdeposits::Full_Data& x, const Model_Billsdeposits::Full_Data& y)
        {
            wxString x_text = this->GetFrequency(&x);
            wxString y_text = this->GetFrequency(&y);
            return x_text < y_text;
        });
        break;
    case billsDepositsListCtrl::LIST_COL_REPEATS:
        std::stable_sort(bills_.begin(), bills_.end()
            , [&](const Model_Billsdeposits::Full_Data& x, const Model_Billsdeposits::Full_Data& y)
        {
            int xn = this->GetNumRepeats(&x);
            int yn = this->GetNumRepeats(&y);
            // the order is: 1, 2, …, -1 (REPEAT_NUM_INFINITY), 0 (REPEAT_NUM_UNKNOWN)
            if (xn > 0)
                return yn > xn || yn == Model_Billsdeposits::REPEAT_NUM_INFINITY || yn == Model_Billsdeposits::REPEAT_NUM_UNKNOWN;
            else
                return xn == Model_Billsdeposits::REPEAT_NUM_INFINITY && yn == Model_Billsdeposits::REPEAT_NUM_UNKNOWN;
        });
        break;
    case billsDepositsListCtrl::LIST_COL_DAYS:
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
    case billsDepositsListCtrl::LIST_COL_NOTES:
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
    int64 id = -1;
    if (m_selected_row != -1)
    {
        id = m_bdp->bills_[m_selected_row].BDID;
    }
    refreshVisualList(m_bdp->initVirtualListControl(id));
}

wxListItemAttr* billsDepositsListCtrl::OnGetItemAttr(long item) const
{
    if (item < 0 || item >= static_cast<int>(m_bdp->bills_.size())) return 0;

    int color_id = m_bdp->bills_[item].COLOR.GetValue();

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
    int64 id = m_bdp->bills_[m_selected_row].BDID;

    int user_color_id = event.GetId();
    user_color_id -= MENU_ON_SET_UDC0;
    wxLogDebug("id: %i", user_color_id);

    Model_Billsdeposits::instance().Savepoint();

    Model_Billsdeposits::Data* item = Model_Billsdeposits::instance().get(id);
    if (item)
    {
        item->COLOR = user_color_id;
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
        m_bitmapTransFilter->SetBitmap(mmBitmapBundle(png::TRANSFILTER_ACTIVE, mmBitmapButtonSize));
    }
    else
    {
        transFilterActive_ = false;
        m_bitmapTransFilter->SetBitmap(mmBitmapBundle(png::TRANSFILTER, mmBitmapButtonSize));
    }

    initVirtualListControl();
}

wxString  mmBillsDepositsPanel::BuildPage() const
{
    return listCtrlAccount_->BuildPage(_("Scheduled Transactions"));
}

void mmBillsDepositsPanel::do_delete_custom_values(int64 id)
{
    const wxString& RefType = Model_Attachment::REFTYPE_STR_TRANSACTION;
    Model_CustomFieldData::DeleteAllData(RefType, id);
}
