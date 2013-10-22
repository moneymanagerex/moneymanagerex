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
#include "constants.h"
#include "util.h"
#include "mmCurrencyFormatter.h"
#include "model/Model_Setting.h"
#include "model/Model_Payee.h"
#include "model/Model_Account.h"
#include "model/Model_Category.h"
#include "model/Model_Billsdeposits.h"

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


    EVT_MENU(MENU_TREEPOPUP_NEW,              billsDepositsListCtrl::OnNewBDSeries)
    EVT_MENU(MENU_TREEPOPUP_EDIT,             billsDepositsListCtrl::OnEditBDSeries)
    EVT_MENU(MENU_TREEPOPUP_DELETE,           billsDepositsListCtrl::OnDeleteBDSeries)
    EVT_MENU(MENU_POPUP_BD_ENTER_OCCUR,       billsDepositsListCtrl::OnEnterBDTransaction)
    EVT_MENU(MENU_POPUP_BD_SKIP_OCCUR,        billsDepositsListCtrl::OnSkipBDTransaction)

    EVT_LIST_KEY_DOWN(wxID_ANY,   billsDepositsListCtrl::OnListKeyDown)
END_EVENT_TABLE()
/*******************************************************/

mmBillsDepositsPanel::mmBillsDepositsPanel(mmCoreDB* core,
    wxWindow *parent, wxWindowID winid,
    const wxPoint& pos, const wxSize& size, long style, const wxString& name)
: core_(core)
, m_imageList()
, listCtrlAccount_()
{
    this->tips_.Add(_("MMEX allows regular payments to be set up as transactions. These transactions can also be regular deposits, or transfers that will occur at some future time. These transactions act a reminder that an event is about to occur, and appears on the Home Page 14 days before the transaction is due. "));
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
    wxListItem itemCol;
    itemCol.SetImage(4);
    itemCol.SetText(_("Next Due Date"));
    itemCol.SetAlign(wxLIST_FORMAT_RIGHT);
    listCtrlAccount_->InsertColumn(0, _("Payee"));
    listCtrlAccount_->InsertColumn(1, _("Account"));
    listCtrlAccount_->InsertColumn(2, _("Type"));
    listCtrlAccount_->InsertColumn(3, _("Amount"));
    listCtrlAccount_->InsertColumn(4, itemCol);
    listCtrlAccount_->InsertColumn(5, _("Frequency"));
    listCtrlAccount_->InsertColumn(6, _("Remaining Days"));
    listCtrlAccount_->InsertColumn(7, _("Notes"));

    /* See if we can get data from inidb */
    int col0 = Model_Setting::instance().GetIntSetting("BD_COL0_WIDTH", 150);
    int col1 = Model_Setting::instance().GetIntSetting("BD_COL1_WIDTH", -2);
    int col2 = Model_Setting::instance().GetIntSetting("BD_COL2_WIDTH", -2);
    int col3 = Model_Setting::instance().GetIntSetting("BD_COL3_WIDTH", -2);
    int col4 = Model_Setting::instance().GetIntSetting("BD_COL4_WIDTH", -2);
    int col5 = Model_Setting::instance().GetIntSetting("BD_COL5_WIDTH", -2);
    int col6 = Model_Setting::instance().GetIntSetting("BD_COL6_WIDTH", -2);
    int col7 = Model_Setting::instance().GetIntSetting("BD_COL7_WIDTH", -2);

    listCtrlAccount_->SetColumnWidth(0, col0);
    listCtrlAccount_->SetColumnWidth(1, col1);
    listCtrlAccount_->SetColumnWidth(2, col2);
    listCtrlAccount_->SetColumnWidth(3, col3);
    listCtrlAccount_->SetColumnWidth(4, col4);
    listCtrlAccount_->SetColumnWidth(5, col5);
    listCtrlAccount_->SetColumnWidth(6, col6);
    listCtrlAccount_->SetColumnWidth(7, col7);

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
    trans_.clear();
    listCtrlAccount_->DeleteAllItems();

    int cnt = 0, selected_item = -1;
    for (const auto& entry: Model_Billsdeposits::instance().all(Model_Billsdeposits::COL_NEXTOCCURRENCEDATE))
    {
        mmBDTransactionHolder th;

        Model_Account::Data* account = Model_Account::instance().get(entry.ACCOUNTID);
        const Model_Category::Data* category = Model_Category::instance().get(entry.CATEGID);
        const Model_Subcategory::Data* sub_category = (entry.SUBCATEGID != -1 ? Model_Subcategory::instance().get(entry.SUBCATEGID) : 0);
        if (!account || !category)
        {
            continue;
        }

        th.id_           = entry.BDID;
        th.nextOccurDate_  = Model_Billsdeposits::NEXTOCCURRENCEDATE(entry);
        th.nextOccurStr_   = mmGetDateForDisplay(th.nextOccurDate_);
        int repeats        = entry.REPEATS;
        th.payeeID_        = entry.PAYEEID;
        th.sStatus_        = entry.STATUS;
        th.transType_      = entry.TRANSCODE;
        th.accountID_      = entry.ACCOUNTID;
        th.toAccountID_    = entry.TOACCOUNTID;
        th.accountName_    = account->ACCOUNTNAME;
        th.amt_            = entry.TRANSAMOUNT;
        th.toAmt_          = entry.TOTRANSAMOUNT;
        th.sNumber_        = entry.TRANSACTIONNUMBER;
        th.notes_          = entry.NOTES;
        th.categID_        = entry.CATEGID;
        th.categoryStr_    = category->CATEGNAME;
        th.subcategID_     = entry.SUBCATEGID;
        th.subcategoryStr_ = sub_category ? sub_category->SUBCATEGNAME : "";

        int numRepeats     = entry.NUMOCCURRENCES;

        th.bd_repeat_user_ = false;
        th.bd_repeat_auto_ = false;

        // DeMultiplex the Auto Executable fields.
        if (repeats >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute User Acknowlegement required
        {
            repeats -= BD_REPEATS_MULTIPLEX_BASE;
            th.bd_repeat_user_ = true;
        }

        if (repeats >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute Silent mode
        {
            repeats -= BD_REPEATS_MULTIPLEX_BASE;
            th.bd_repeat_auto_ = true;
        }

        th.repeatsStr_ = BILLSDEPOSITS_REPEATS[repeats];
        if ( repeats >10 && repeats<15)
            th.repeatsStr_ = wxString::Format(th.repeatsStr_, (wxString()<<numRepeats));

        wxDateTime today = wxDateTime::Now();
        wxTimeSpan ts = th.nextOccurDate_.Subtract(today);
        th.daysRemaining_ = ts.GetDays();
        int minutesRemaining = ts.GetMinutes();

        if (minutesRemaining > 0)
            th.daysRemaining_ += 1;

        th.daysRemainingStr_ = wxString::Format("%d", th.daysRemaining_) + _(" days remaining");

        if (th.daysRemaining_ == 0)
        {
            if (((repeats > 10) && (repeats < 15)) && (numRepeats < 0) )
                th.daysRemainingStr_ = _("Inactive");
        }

        if (th.daysRemaining_ < 0)
        {
            th.daysRemainingStr_ = wxString::Format("%d", abs(th.daysRemaining_)) + _(" days overdue!");
            if ( ((repeats > 10) && (repeats < 15)) && (numRepeats < 0) )
                th.daysRemainingStr_ = _("Inactive");
        }

        th.transAmtString_ = CurrencyFormatter::float2String(th.amt_);
        th.transToAmtString_ = CurrencyFormatter::float2String(th.toAmt_);

        if (th.transType_ == TRANS_TYPE_TRANSFER_STR)
        {
            Model_Account::Data *account = Model_Account::instance().get(th.toAccountID_);
            if (account)
                th.payeeStr_ = account->ACCOUNTNAME;
        }
        else
        {
            Model_Payee::Data* payee = Model_Payee::instance().get(th.payeeID_);
            if (payee)
                th.payeeStr_ = payee->PAYEENAME;
        }

        bool toAdd = true;

        if (transFilterActive_)
        {
            toAdd  = transFilterDlg_->somethingSelected();
            if (transFilterDlg_->getAccountCheckBox())
                toAdd = toAdd && (transFilterDlg_->getAccountID() == th.accountID_);
            if (transFilterDlg_->getDateRangeCheckBox())
                toAdd = toAdd && (transFilterDlg_->getFromDateCtrl() <= th.nextOccurDate_
                    && transFilterDlg_->getToDateControl() >= th.nextOccurDate_);
            if (transFilterDlg_->getPayeeCheckBox())
                toAdd = toAdd && (transFilterDlg_->userPayeeStr() == th.payeeStr_);
            if (transFilterDlg_->getCategoryCheckBox())
                toAdd = toAdd && (transFilterDlg_->getCategoryID() == th.categID_
                    && (transFilterDlg_->getSubCategoryID() == th.subcategID_ || transFilterDlg_->getSubCategoryID()<0));
            if (transFilterDlg_->getStatusCheckBox())
                toAdd = toAdd && transFilterDlg_->compareStatus(th.sStatus_);
            if (transFilterDlg_->getTypeCheckBox())
                toAdd = toAdd && (transFilterDlg_->getType().Contains(th.transType_));
            if (transFilterDlg_->getAmountRangeCheckBoxMin())
                toAdd = toAdd && (transFilterDlg_->getAmountMin() <= th.amt_);
            if (transFilterDlg_->getAmountRangeCheckBoxMax())
                toAdd = toAdd && (transFilterDlg_->getAmountMax() >= th.amt_);
            if (transFilterDlg_->getNumberCheckBox())
                toAdd = toAdd && (transFilterDlg_->getNumber().Trim().Lower() == th.sNumber_.Lower());
            if (transFilterDlg_->getNotesCheckBox())
                toAdd = toAdd && (th.notes_.Lower().Matches(transFilterDlg_->getNotes().Trim().Lower()));

        }

        if (toAdd) trans_.push_back(th);
        if (th.id_ == id) selected_item = cnt;
    }

    listCtrlAccount_->SetItemCount(static_cast<long>(trans_.size()));
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
    if (column == 0) return trans_[item].payeeStr_;
    else if (column == 1) return trans_[item].accountName_;
    else if (column == 2) return wxGetTranslation(trans_[item].transType_);
    else if (column == 3) return trans_[item].transAmtString_;
    else if (column == 4) return trans_[item].nextOccurStr_;
    else if (column == 5) return wxGetTranslation(trans_[item].repeatsStr_);
    else if (column == 6) return trans_[item].daysRemainingStr_;
    else if (column == 7) return trans_[item].notes_;

    return "";
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
    /* Returns the icon to be shown for each entry */
    if (cp_->trans_[item].daysRemainingStr_ == _("Inactive")) return -1;
    if (cp_->trans_[item].daysRemaining_ < 0) return 0;
    if (cp_->trans_[item].bd_repeat_auto_) return 1;
    if (cp_->trans_[item].bd_repeat_user_) return 2;

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
    mmBDDialog dlg(cp_->core_, 0, false, false, this );
    if ( dlg.ShowModal() == wxID_OK )
        refreshVisualList(cp_->initVirtualListControl(dlg.GetTransID()));
}

void billsDepositsListCtrl::OnEditBDSeries(wxCommandEvent& /*event*/)
{
    if (m_selected_row == -1) return;
    if (!cp_->core_->db_.get()) return;

    mmBDDialog dlg(cp_->core_, cp_->trans_[m_selected_row].id_, true, false, this );
    if ( dlg.ShowModal() == wxID_OK )
        refreshVisualList(cp_->initVirtualListControl(dlg.GetTransID()));
}

void billsDepositsListCtrl::OnDeleteBDSeries(wxCommandEvent& /*event*/)
{
    if (m_selected_row < 0) return;
    if (!cp_->core_->db_.get()) return;
    if (cp_->trans_.size() == 0) return;

    wxMessageDialog msgDlg(this, _("Do you really want to delete the series?"),
                                        _("Confirm Series Deletion"),
                                        wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION);
    if (msgDlg.ShowModal() == wxID_YES)
    {
        mmDBWrapper::deleteBDSeries(cp_->core_->db_.get(), cp_->trans_[m_selected_row].id_);
        cp_->initVirtualListControl();
        refreshVisualList(m_selected_row);
    }
}

void billsDepositsListCtrl::OnEnterBDTransaction(wxCommandEvent& /*event*/)
{
    if (m_selected_row == -1) return;
    if (!cp_->core_->db_.get()) return;

    int id = cp_->trans_[m_selected_row].id_;
    mmBDDialog dlg(cp_->core_, id, false, true, this );
    if ( dlg.ShowModal() == wxID_OK )
        refreshVisualList(cp_->initVirtualListControl(id));
}

void billsDepositsListCtrl::OnSkipBDTransaction(wxCommandEvent& /*event*/)
{
    if (m_selected_row == -1 || !cp_->core_->db_.get()) return;

    int id = cp_->trans_[m_selected_row].id_;
    mmDBWrapper::completeBDInSeries(cp_->core_->db_.get(), id);
    refreshVisualList(cp_->initVirtualListControl(id));
}

void billsDepositsListCtrl::OnListItemActivated(wxListEvent& /*event*/)
{
    if (m_selected_row == -1) return;
    if (!cp_->core_->db_.get()) return;

    mmBDDialog dlg(cp_->core_, cp_->trans_[m_selected_row].id_, true, false, this);
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
        wxString addInfo;
        addInfo << trans_[selIndex].categoryStr_ << (trans_[selIndex].subcategoryStr_ == "" ? "" : ":" + trans_[selIndex].subcategoryStr_);
        stm->SetLabel(addInfo);
        st ->SetLabel (trans_[selIndex].notes_ );
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
    // TODO
}

wxString mmBillsDepositsPanel::tips()
{
    return this->tips_[rand() % this->tips_.GetCount()];
}

void billsDepositsListCtrl::refreshVisualList(int selected_index)
{

    //TODO:
    if (selected_index >= (long)cp_->trans_.size() || selected_index < 0)
        selected_index = /*g_asc*/ true ? (long)cp_->trans_.size() - 1 : 0;
    if (!cp_->trans_.empty()) {
        RefreshItems(0, cp_->trans_.size() - 1);
    }
    else
        selected_index = -1;

    if (selected_index >= 0 && !cp_->trans_.empty())
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
    if (cp_->trans_.size() == 0) return;
    int id = -1;
    if (m_selected_row != -1)
    {
        id = cp_->trans_[m_selected_row].id_;
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
