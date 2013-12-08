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

#include "billsdepositspanel.h"
#include "billsdepositsdialog.h"
#include "util.h"
#include "model/Model_Setting.h"
#include "model/Model_Payee.h"
#include "model/Model_Account.h"
#include "model/Model_Category.h"

/*******************************************************/
BEGIN_EVENT_TABLE(mmBillsDepositsPanel, wxPanel)
    EVT_BUTTON(wxID_NEW,         mmBillsDepositsPanel::OnNewBDSeries)
    EVT_BUTTON(wxID_EDIT,        mmBillsDepositsPanel::OnEditBDSeries)
    EVT_BUTTON(wxID_DELETE,      mmBillsDepositsPanel::OnDeleteBDSeries)
    EVT_BUTTON(wxID_PASTE, mmBillsDepositsPanel::OnEnterBDTransaction)
    EVT_BUTTON(wxID_IGNORE,  mmBillsDepositsPanel::OnSkipBDTransaction)
END_EVENT_TABLE()
/*******************************************************/
BEGIN_EVENT_TABLE(billsDepositsListCtrl, mmListCtrl)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY,   billsDepositsListCtrl::OnListItemActivated)
    EVT_LIST_ITEM_RIGHT_CLICK(wxID_ANY, billsDepositsListCtrl::OnItemRightClick)
    EVT_LIST_ITEM_SELECTED(wxID_ANY,    billsDepositsListCtrl::OnListItemSelected)
    EVT_LIST_ITEM_DESELECTED(wxID_ANY,    billsDepositsListCtrl::OnListItemDeselected)
    EVT_LIST_COL_END_DRAG(wxID_ANY, billsDepositsListCtrl::OnItemResize)
    EVT_LIST_COL_CLICK(wxID_ANY, billsDepositsListCtrl::OnColClick)

    EVT_MENU(MENU_TREEPOPUP_NEW,              billsDepositsListCtrl::OnNewBDSeries)
    EVT_MENU(MENU_TREEPOPUP_EDIT,             billsDepositsListCtrl::OnEditBDSeries)
    EVT_MENU(MENU_TREEPOPUP_DELETE,           billsDepositsListCtrl::OnDeleteBDSeries)
    EVT_MENU(MENU_POPUP_BD_ENTER_OCCUR,       billsDepositsListCtrl::OnEnterBDTransaction)
    EVT_MENU(MENU_POPUP_BD_SKIP_OCCUR,        billsDepositsListCtrl::OnSkipBDTransaction)

    EVT_LIST_KEY_DOWN(wxID_ANY,   billsDepositsListCtrl::OnListKeyDown)
END_EVENT_TABLE()
/*******************************************************/

billsDepositsListCtrl::billsDepositsListCtrl(mmBillsDepositsPanel* cp, wxWindow *parent, wxWindowID winid)
: mmListCtrl(parent, winid)
, cp_(cp)
{
    m_selected_col = 4;
    m_asc = true;
}

billsDepositsListCtrl::~billsDepositsListCtrl()
{
}

void billsDepositsListCtrl::OnColClick(wxListEvent& event)
{
    if (0 > event.GetColumn() || event.GetColumn() >= cp_->getColumnsNumber()) return;

    if (m_selected_col == event.GetColumn()) m_asc = !m_asc;

    wxListItem item;
    item.SetMask(wxLIST_MASK_IMAGE);
    item.SetImage(-1);
    SetColumn(m_selected_col, item);

    m_selected_col = event.GetColumn();

    int id = -1;
    if (m_selected_row >= 0) id = cp_->bills_[m_selected_row].BDID;
    refreshVisualList(cp_->initVirtualListControl(id));
}

mmBillsDepositsPanel::mmBillsDepositsPanel(wxWindow *parent, wxWindowID winid,
    const wxPoint& pos, const wxSize& size, long style, const wxString& name)
: m_imageList()
, listCtrlAccount_()
, transFilterDlg_(0)
{
    ColName_[COL_PAYEE] = _("Payee");
    ColName_[COL_ACCOUNT] = _("Account");
    ColName_[COL_TYPE] = _("Type");
    ColName_[COL_AMOUNT] = _("Amount");
    ColName_[COL_DUE_DATE] = _("Next Due Date");
    ColName_[COL_FREQUENCY] = _("Frequency");
    ColName_[COL_DAYS] = _("Remaining Days");
    ColName_[COL_NOTES] = _("Notes");

    this->tips_.Add(_("MMEX allows regular payments to be set up as transactions. These transactions can also be regular deposits, or transfers that will occur at some future time. These transactions act as a reminder that an event is about to occur, and appears on the Home Page 14 days before the transaction is due. "));
    this->tips_.Add(_("Tip: These transactions can be set up to activate – allowing the user to adjust any values on the due date."));

    Create(parent, winid, pos, size, style, name);
}

bool mmBillsDepositsPanel::Create( wxWindow *parent,
            wxWindowID winid, const wxPoint& pos,
            const wxSize& size,long style, const wxString& name  )
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create(parent, winid, pos, size, style, name);

    this->windowsFreezeThaw();

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    /* Set up the transaction filter.  The transFilter dialog will be destroyed
       when the checking panel is destroyed. */
    transFilterActive_ = false;
    transFilterDlg_    = new mmFilterTransactionsDialog(this);

    initVirtualListControl();
    
    this->windowsFreezeThaw();

    return TRUE;
}

mmBillsDepositsPanel::~mmBillsDepositsPanel()
{
   if (m_imageList)
        delete m_imageList;
   if (transFilterDlg_)
       delete transFilterDlg_;
}

void mmBillsDepositsPanel::CreateControls()
{

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer9);

    /* ---------------------- */
    wxPanel* headerPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition,
        wxDefaultSize, wxNO_BORDER|wxTAB_TRAVERSAL );
    itemBoxSizer9->Add(headerPanel, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxBoxSizer* itemBoxSizerVHeader = new wxBoxSizer(wxVERTICAL);
    headerPanel->SetSizer(itemBoxSizerVHeader);

    int font_size = this->GetFont().GetPointSize() + 2;
    wxStaticText* itemStaticText9 = new wxStaticText( headerPanel, wxID_ANY,
        _("Repeating Transactions"));
    itemStaticText9->SetFont(wxFont(font_size, wxSWISS, wxNORMAL, wxBOLD, FALSE, ""));
    itemBoxSizerVHeader->Add(itemStaticText9, 0, wxALL, 1);

    wxBoxSizer* itemBoxSizerHHeader2 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizerVHeader->Add(itemBoxSizerHHeader2);

    wxBitmap itemStaticBitmap(rightarrow_xpm);
    bitmapTransFilter_ = new wxStaticBitmap( headerPanel, wxID_ANY, itemStaticBitmap);
    itemBoxSizerHHeader2->Add(bitmapTransFilter_, 0, wxALL, 1);
    bitmapTransFilter_->Connect(wxID_ANY, wxEVT_LEFT_DOWN, wxMouseEventHandler(mmBillsDepositsPanel::OnFilterTransactions), NULL, this);
    bitmapTransFilter_->Connect(wxID_ANY, wxEVT_RIGHT_DOWN, wxMouseEventHandler(mmBillsDepositsPanel::OnFilterTransactions), NULL, this);

    itemBoxSizerHHeader2->AddSpacer(5);
    wxStaticText* statTextTransFilter_ = new wxStaticText( headerPanel, wxID_ANY,
        _("Transaction Filter"));
    itemBoxSizerHHeader2->Add(statTextTransFilter_, 0, wxALIGN_CENTER_VERTICAL, 0);

    /* ---------------------- */
    wxSplitterWindow* itemSplitterWindowBillsDeposit = new wxSplitterWindow( this,
        ID_SPLITTERWINDOW, wxDefaultPosition, wxSize(200, 200),
        wxSP_3DBORDER|wxSP_3DSASH|wxNO_BORDER );

    wxSize imageSize(16, 16);
    m_imageList = new wxImageList( imageSize.GetWidth(), imageSize.GetHeight() );
    m_imageList->Add(wxBitmap(wxImage(error_xpm).Scale(16, 16)));
    m_imageList->Add(wxBitmap(wxImage(rt_exec_auto_xpm).Scale(16, 16)));
    m_imageList->Add(wxBitmap(wxImage(rt_exec_user_xpm).Scale(16, 16)));
    m_imageList->Add(wxBitmap(wxImage(uparrow_xpm).Scale(16, 16)));
    m_imageList->Add(wxBitmap(wxImage(downarrow_xpm).Scale(16, 16)));

    listCtrlAccount_ = new billsDepositsListCtrl( this, itemSplitterWindowBillsDeposit);

    listCtrlAccount_->SetImageList(m_imageList, wxIMAGE_LIST_SMALL);
    for (const auto&column : ColName_)
    {
        wxListItem itemCol;
        if (column.first == 4) itemCol.SetImage(4);
        itemCol.SetText(column.second);
        listCtrlAccount_->InsertColumn(column.first, column.second, (column.first == 4 ? wxLIST_FORMAT_RIGHT : wxLIST_FORMAT_LEFT));

        int col_x = Model_Setting::instance().GetIntSetting(wxString::Format("BD_COL%d_WIDTH", column.first), (column.first > 0 ? - 2 : 150));
        listCtrlAccount_->SetColumnWidth(column.first, col_x);
    }

    wxPanel* itemPanel12 = new wxPanel( itemSplitterWindowBillsDeposit, ID_PANEL1,
        wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxTAB_TRAVERSAL );

    itemSplitterWindowBillsDeposit->SplitHorizontally(listCtrlAccount_, itemPanel12);
    itemSplitterWindowBillsDeposit->SetMinimumPaneSize(100);
    itemSplitterWindowBillsDeposit->SetSashGravity(1.0);
    itemBoxSizer9->Add(itemSplitterWindowBillsDeposit, 1, wxGROW|wxALL, 1);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemPanel12->SetSizer(itemBoxSizer4);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer4->Add(itemBoxSizer5, 0, wxALIGN_LEFT|wxALL, 5);

    wxButton* itemButton6 = new wxButton( itemPanel12, wxID_NEW);
    itemButton6->SetToolTip(_("New Bills & Deposit Series"));
    itemBoxSizer5->Add(itemButton6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 4);

    wxButton* itemButton81 = new wxButton( itemPanel12, wxID_EDIT);
    itemButton81->SetToolTip(_("Edit Bills & Deposit Series"));
    itemBoxSizer5->Add(itemButton81, 0, wxALIGN_CENTER_VERTICAL|wxALL, 4);
    itemButton81->Enable(false);

    wxButton* itemButton7 = new wxButton( itemPanel12, wxID_DELETE);
    itemButton7->SetToolTip(_("Delete Bills & Deposit Series"));
    itemBoxSizer5->Add(itemButton7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 4);
    itemButton7->Enable(false);

    wxButton* itemButton8 = new wxButton( itemPanel12, wxID_PASTE, _("En&ter"),
        wxDefaultPosition, wxDefaultSize, 0 );
    itemButton8->SetToolTip(_("Enter Next Bills & Deposit Occurrence"));
    itemBoxSizer5->Add(itemButton8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 4);
    itemButton8->Enable(false);

    wxButton* buttonSkipTrans = new wxButton( itemPanel12, wxID_IGNORE, _("&Skip"));
    buttonSkipTrans->SetToolTip(_("Skip Next Bills & Deposit Occurrence"));
    itemBoxSizer5->Add(buttonSkipTrans, 0, wxALIGN_CENTER_VERTICAL|wxALL, 4);
    buttonSkipTrans->Enable(false);

    //Infobar-mini
    wxStaticText* itemStaticText444 = new wxStaticText(itemPanel12, ID_PANEL_BD_STATIC_MINI, "",
                                                       wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer5->Add(itemStaticText444, 1, wxGROW|wxTOP, 12);

    //Infobar
    wxStaticText* text = new wxStaticText( itemPanel12, ID_PANEL_BD_STATIC_DETAILS, "",
    wxPoint(-1,-1), wxSize(200, -1), wxNO_BORDER|wxTE_MULTILINE|wxTE_WORDWRAP|wxST_NO_AUTORESIZE);
    itemBoxSizer4->Add(text, 1, wxGROW|wxLEFT|wxRIGHT, 14);

    mmBillsDepositsPanel::updateBottomPanelData(-1);
}

int mmBillsDepositsPanel::initVirtualListControl(int id)
{
    listCtrlAccount_->DeleteAllItems();

    wxListItem item;
    item.SetMask(wxLIST_MASK_IMAGE);
    item.SetImage(listCtrlAccount_->m_asc ? 4 : 3);
    listCtrlAccount_->SetColumn(listCtrlAccount_->m_selected_col, item);

    bills_.clear();
    if (transFilterActive_)
    {
        for (const Model_Billsdeposits::Data& data : Model_Billsdeposits::instance().all(Model_Billsdeposits::COL_NEXTOCCURRENCEDATE))
        {
            if (transFilterDlg_->getAccountCheckBox() && transFilterDlg_->getAccountID() != data.ACCOUNTID)
                continue; // Skip

            if (transFilterDlg_->getDateRangeCheckBox() && !Model_Billsdeposits::NEXTOCCURRENCEDATE(data).IsBetween(transFilterDlg_->getFromDateCtrl(), transFilterDlg_->getToDateControl()))
                continue; // Skip

            if (!transFilterDlg_->checkPayee(data.PAYEEID))
                continue; // Skip

            if (transFilterDlg_->getCategoryCheckBox())
            {
                if (data.CATEGID != -1)
                {
                    if (transFilterDlg_->getCategoryID() != data.CATEGID)
                        continue; // Skip
                    if (transFilterDlg_->getSubCategoryID() != data.SUBCATEGID && !transFilterDlg_->getSimilarCategoryStatus())
                        continue; // Skip
                }
                else
                {
                    bool bMatching = false;
                    for (const Model_Budgetsplittransaction::Data split : Model_Billsdeposits::splittransaction(data))
                    {
                        if (split.CATEGID != data.CATEGID)
                            continue;
                        if (split.SUBCATEGID != data.SUBCATEGID && !transFilterDlg_->getSimilarCategoryStatus())
                            continue;

                        bMatching = true;
                        break;
                    }
                    if (!bMatching)
                        continue;
                }
            }
            if (transFilterDlg_->getStatusCheckBox() && !transFilterDlg_->compareStatus(data.STATUS))
                continue; // Skip

            // Repeating transactions are always a transfer to
            if (transFilterDlg_->getTypeCheckBox() && !transFilterDlg_->allowType(data.TRANSCODE, data.ACCOUNTID != data.TOACCOUNTID))
                continue; // Skip

            if (transFilterDlg_->getAmountRangeCheckBoxMin() && transFilterDlg_->getAmountMin() > data.TRANSAMOUNT)
                continue; // Skip

            if (transFilterDlg_->getAmountRangeCheckBoxMax() && transFilterDlg_->getAmountMax() < data.TRANSAMOUNT)
                continue; // Skip

            if (transFilterDlg_->getNumberCheckBox() && !data.TRANSACTIONNUMBER.Lower().Matches(transFilterDlg_->getNumber().Trim().Lower()))
                continue; // Skip

            if (transFilterDlg_->getNotesCheckBox() && !data.NOTES.Lower().Matches(transFilterDlg_->getNotes().Trim().Lower()))
                continue; // Skip

            Model_Billsdeposits::Full_Data r(data);

            const Model_Payee::Data* payee = Model_Payee::instance().get(r.PAYEEID);
            if (payee) r.PAYEENAME = payee->PAYEENAME;
            const Model_Account::Data* account = Model_Account::instance().get(r.ACCOUNTID);
            if (account) 
            {
                r.ACCOUNTNAME = account->ACCOUNTNAME;
                if (Model_Billsdeposits::type(r) == Model_Billsdeposits::TRANSFER)
                    r.PAYEENAME = account->ACCOUNTNAME;
            }

            bills_.push_back(r);
        }
    }
    else
        for (const auto& data: Model_Billsdeposits::instance().all(Model_Billsdeposits::COL_NEXTOCCURRENCEDATE))
        {
            Model_Billsdeposits::Full_Data r(data);

            const Model_Payee::Data* payee = Model_Payee::instance().get(r.PAYEEID);
            if (payee) r.PAYEENAME = payee->PAYEENAME;
            const Model_Account::Data* account = Model_Account::instance().get(r.ACCOUNTID);
            if (account)
            {
                r.ACCOUNTNAME = account->ACCOUNTNAME;
                if (Model_Billsdeposits::type(r) == Model_Billsdeposits::TRANSFER)
                    r.PAYEENAME = account->ACCOUNTNAME;
            }

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

/*******************************************************/
void billsDepositsListCtrl::OnItemResize(wxListEvent& event)
{
    int i = event.GetColumn();
    wxString parameter_name = wxString::Format("BD_COL%d_WIDTH", i);
    int current_width = GetColumnWidth(i);
    Model_Setting::instance().Set(parameter_name, current_width);
}

void billsDepositsListCtrl::OnItemRightClick(wxListEvent& event)
{
    m_selected_row = event.GetIndex();

    wxMenu menu;
    menu.Append(MENU_POPUP_BD_ENTER_OCCUR, _("Enter next Occurrence..."));
    menu.AppendSeparator();
    menu.Append(MENU_POPUP_BD_SKIP_OCCUR, _("Skip next Occurrence"));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_NEW, _("&New Bills && Deposit Series..."));
    menu.Append(MENU_TREEPOPUP_EDIT, _("&Edit Bills && Deposit Series..."));
    menu.Append(MENU_TREEPOPUP_DELETE, _("&Delete Bills && Deposit Series..."));
    PopupMenu(&menu, event.GetPoint());
}

wxString mmBillsDepositsPanel::getItem(long item, long column)
{
    const Model_Billsdeposits::Full_Data& bill = this->bills_.at(item);
    wxString text = "";
    if (column == COL_PAYEE)
    {
        text = bill.PAYEENAME;
    }
    else if (column == COL_ACCOUNT)
    {
        text = bill.ACCOUNTNAME;
    }
    else if (column == COL_TYPE)
    {
        text = wxGetTranslation(bill.TRANSCODE);
    }
    else if (column == COL_AMOUNT)
    {
        text = Model_Account::toCurrency(bill.TRANSAMOUNT, Model_Account::instance().get(bill.ACCOUNTID));
    }
    else if (column == COL_DUE_DATE)
    {
        text = mmGetDateForDisplay(Model_Billsdeposits::NEXTOCCURRENCEDATE(bill));
    }
    else if (column == COL_FREQUENCY)
    {
        text = GetFrequency(&bill);
    }
    else if (column == COL_DAYS)
    {
        text = GetRemainingDays(&bill);
    }
    else if (column == COL_NOTES)
    {
        text = bill.NOTES;
    }

    return text;
}

wxString mmBillsDepositsPanel::GetFrequency(const Model_Billsdeposits::Data* item)
{
    int repeats = item->REPEATS;
    // DeMultiplex the Auto Executable fields.
    if (repeats >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute User Acknowlegement required
        repeats -= BD_REPEATS_MULTIPLEX_BASE;
    if (repeats >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute Silent mode
        repeats -= BD_REPEATS_MULTIPLEX_BASE;

    wxString text = BILLSDEPOSITS_REPEATS[repeats];
    if (repeats > 10 && repeats < 15)
        text = wxString::Format(text, (wxString() << item->NUMOCCURRENCES));
    return text;
}

wxString mmBillsDepositsPanel::GetRemainingDays(const Model_Billsdeposits::Data* item)
{
    int repeats = item->REPEATS;
    // DeMultiplex the Auto Executable fields.
    if (repeats >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute User Acknowlegement required
        repeats -= BD_REPEATS_MULTIPLEX_BASE;
    if (repeats >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute Silent mode
        repeats -= BD_REPEATS_MULTIPLEX_BASE;

    wxDateTime today = wxDateTime::Now();
    wxTimeSpan ts = Model_Billsdeposits::NEXTOCCURRENCEDATE(item).Subtract(today);
    int daysRemaining = ts.GetDays();
    int minutesRemaining = ts.GetMinutes();

    if (minutesRemaining > 0)
        daysRemaining += 1;

    wxString text = wxString::Format(_("%d days remaining"), daysRemaining);

    if (daysRemaining == 0)
    {
        if (((repeats > 10) && (repeats < 15)) && (item->NUMOCCURRENCES < 0))
            text = _("Inactive");
    }

    if (daysRemaining < 0)
    {
        text = wxString::Format(_("%d days overdue!"), abs(daysRemaining));
        if (((repeats > 10) && (repeats < 15)) && (item->NUMOCCURRENCES < 0))
            text = _("Inactive");
    }
    return text;
}

wxString billsDepositsListCtrl::OnGetItemText(long item, long column) const
{
    return cp_->getItem(item, column);
}

void billsDepositsListCtrl::OnListItemSelected(wxListEvent& event)
{
    m_selected_row = event.GetIndex();
    cp_->updateBottomPanelData(m_selected_row);
}

void billsDepositsListCtrl::OnListItemDeselected(wxListEvent& /*event*/)
{
    m_selected_row = -1;
    cp_->updateBottomPanelData(m_selected_row);
}

int billsDepositsListCtrl::OnGetItemImage(long item) const
{
    bool bd_repeat_user = false;
    bool bd_repeat_auto = false;
    int repeats = cp_->bills_[item].REPEATS;
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

    wxDateTime today = wxDateTime::Now();
    wxTimeSpan ts = Model_Billsdeposits::NEXTOCCURRENCEDATE(cp_->bills_[item]).Subtract(today);
    int daysRemaining = ts.GetDays();
    int minutesRemaining = ts.GetMinutes();

    if (minutesRemaining > 0)
        daysRemaining += 1;

    wxString daysRemainingStr = wxString::Format(_("%d days remaining"), daysRemaining);

    if (daysRemaining == 0)
    {
        if (((repeats > 10) && (repeats < 15)) && (cp_->bills_[item].NUMOCCURRENCES < 0))
            daysRemainingStr = _("Inactive");
    }

    if (daysRemaining < 0)
    {
        daysRemainingStr = wxString::Format(_("%d days overdue!"), abs(daysRemaining));
        if (((repeats > 10) && (repeats < 15)) && (cp_->bills_[item].NUMOCCURRENCES < 0))
            daysRemainingStr = _("Inactive");
    }

    /* Returns the icon to be shown for each entry */
    if (daysRemainingStr == _("Inactive")) return -1;
    if (daysRemaining < 0) return 0;
    if (bd_repeat_auto) return 1;
    if (bd_repeat_user) return 2;

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
    mmBDDialog dlg(0, false, false, this );
    if ( dlg.ShowModal() == wxID_OK )
        refreshVisualList(cp_->initVirtualListControl(dlg.GetTransID()));
}

void billsDepositsListCtrl::OnEditBDSeries(wxCommandEvent& /*event*/)
{
    if (m_selected_row == -1) return;

    mmBDDialog dlg(cp_->bills_[m_selected_row].BDID, true, false, this );
    if ( dlg.ShowModal() == wxID_OK )
        refreshVisualList(cp_->initVirtualListControl(dlg.GetTransID()));
}

void billsDepositsListCtrl::OnDeleteBDSeries(wxCommandEvent& /*event*/)
{
    if (m_selected_row < 0) return;
    if (cp_->bills_.size() == 0) return;

    wxMessageDialog msgDlg(this, _("Do you really want to delete the series?")
        , _("Confirm Series Deletion")
        , wxYES_NO | wxNO_DEFAULT | wxICON_ERROR);
    if (msgDlg.ShowModal() == wxID_YES)
    {
        Model_Billsdeposits::instance().remove(cp_->bills_[m_selected_row].BDID);
        cp_->initVirtualListControl();
        refreshVisualList(m_selected_row);
    }
}

void billsDepositsListCtrl::OnEnterBDTransaction(wxCommandEvent& /*event*/)
{
    if (m_selected_row == -1) return;

    int id = cp_->bills_[m_selected_row].BDID;
    mmBDDialog dlg(id, false, true, this );
    if ( dlg.ShowModal() == wxID_OK )
        refreshVisualList(cp_->initVirtualListControl(id));
}

void billsDepositsListCtrl::OnSkipBDTransaction(wxCommandEvent& /*event*/)
{
    if (m_selected_row == -1) return;

    int id = cp_->bills_[m_selected_row].BDID;
    Model_Billsdeposits::instance().completeBDInSeries(id);
    refreshVisualList(cp_->initVirtualListControl(id));
}

void billsDepositsListCtrl::OnListItemActivated(wxListEvent& /*event*/)
{
    if (m_selected_row == -1) return;

    mmBDDialog dlg(cp_->bills_[m_selected_row].BDID, true, false, this);
    if ( dlg.ShowModal() == wxID_OK )
        refreshVisualList(cp_->initVirtualListControl(dlg.GetTransID()));
}

void mmBillsDepositsPanel::updateBottomPanelData(int selIndex)
{
    enableEditDeleteButtons(selIndex >= 0);
    wxStaticText* st = (wxStaticText*)FindWindow(ID_PANEL_BD_STATIC_DETAILS);
    wxStaticText* stm = (wxStaticText*)FindWindow(ID_PANEL_BD_STATIC_MINI);

    if (selIndex !=-1)
    {
        wxString categoryStr = "";
        if (Model_Billsdeposits::type(bills_[selIndex]) == Model_Billsdeposits::TRANSFER)
        {
            const Model_Category::Data* category = Model_Category::instance().get(bills_[selIndex].CATEGID);
            if (category)
                categoryStr = category->CATEGNAME;
        }
        wxString subcategoryStr = "";
        if (bills_[selIndex].SUBCATEGID >= 0)
        {
            const Model_Subcategory::Data* subcategory = Model_Subcategory::instance().get(bills_[selIndex].SUBCATEGID);
            if (subcategory)
                subcategoryStr = subcategory->SUBCATEGNAME;
        }
        wxString addInfo;
        addInfo << categoryStr << (subcategoryStr == "" ? "" : ":" + subcategoryStr);
        stm->SetLabel(addInfo);
        st ->SetLabel (bills_[selIndex].NOTES );
    }
    else
    {
        st-> SetLabel(this->tips());
        stm-> SetLabel("");
    }
}

void mmBillsDepositsPanel::enableEditDeleteButtons(bool en)
{
    wxButton* bE = (wxButton*)FindWindow(wxID_EDIT);
    wxButton* bD = (wxButton*)FindWindow(wxID_DELETE);
    wxButton* bN = (wxButton*)FindWindow(wxID_PASTE);
    wxButton* bS = (wxButton*)FindWindow(wxID_IGNORE);
    bE->Enable(en);
    bD->Enable(en);
    bN->Enable(en);
    bS->Enable(en);
}

void mmBillsDepositsPanel::sortTable()
{
    std::sort(bills_.begin(), bills_.end());
    switch (listCtrlAccount_->m_selected_col)
    {
    case COL_PAYEE:
        std::stable_sort(bills_.begin(), bills_.end(), SorterByPAYEENAME());
        break;
    case COL_ACCOUNT:
        std::stable_sort(bills_.begin(), bills_.end(), SorterByACCOUNTNAME());
        break;
    case COL_TYPE:
        std::stable_sort(bills_.begin(), bills_.end(), SorterByTRANSCODE());
        break;
    case COL_AMOUNT:
        std::stable_sort(bills_.begin(), bills_.end(), SorterByTRANSAMOUNT());
        break;
    case COL_DUE_DATE:
        std::stable_sort(bills_.begin(), bills_.end(), SorterByNEXTOCCURRENCEDATE());
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

    if (selected_index >= (long)cp_->bills_.size() || selected_index < 0)
        selected_index = - 1;
    if (!cp_->bills_.empty()) {
        RefreshItems(0, cp_->bills_.size() - 1);
    }
    else
        selected_index = -1;

    if (selected_index >= 0 && !cp_->bills_.empty())
    {
        SetItemState(selected_index, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        SetItemState(selected_index, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
        EnsureVisible(selected_index);
    }
    m_selected_row = selected_index;
    cp_->updateBottomPanelData(selected_index);
}

void billsDepositsListCtrl::RefreshList()
{
    if (cp_->bills_.size() == 0) return;
    int id = -1;
    if (m_selected_row != -1)
    {
        id = cp_->bills_[m_selected_row].BDID;
    }
    refreshVisualList(cp_->initVirtualListControl(id));
}

void mmBillsDepositsPanel::RefreshList()
{
    listCtrlAccount_->RefreshList();
}

void mmBillsDepositsPanel::OnFilterTransactions(wxMouseEvent& event)
{

    int e = event.GetEventType();

    wxBitmap bitmapFilterIcon(rightarrow_xpm);

    if (e == wxEVT_LEFT_DOWN)
    {

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

    initVirtualListControl();
}
