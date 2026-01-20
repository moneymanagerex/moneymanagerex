/*******************************************************
 Copyright (C) 2025 Klaus Wich

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

#include "reconciledialog.h"
#include "constants.h"
#include "images_list.h"
#include "option.h"
#include "reports/mmDateRange.h"
#include "model/Model_Checking.h"
#include "model/Model_Payee.h"
#include "model/Model_Account.h"
#include "model/Model_Infotable.h"
#include "validators.h"
#include "transdialog.h"


wxIMPLEMENT_DYNAMIC_CLASS(mmReconcileDialog, wxDialog);

mmReconcileDialog::mmReconcileDialog()
{
}

mmReconcileDialog::~mmReconcileDialog()
{
    wxSize size = GetSize();
    Model_Infotable::instance().setSize("RECONCILE_DIALOG_SIZE", size);
    Model_Infotable::instance().setBool("RECONCILE_DIALOG_SHOW_STATE_COL", m_settings[SETTING_SHOW_STATE_COL]);
    Model_Infotable::instance().setBool("RECONCILE_DIALOG_SHOW_NUMBER_COL", m_settings[SETTING_SHOW_NUMBER_COL]);
    Model_Infotable::instance().setBool("RECONCILE_DIALOG_INCLUDE_VOID", m_settings[SETTING_INCLUDE_VOID]);
    Model_Infotable::instance().setBool("RECONCILE_DIALOG_INCLUDE_DUPLICATED", m_settings[SETTING_INCLUDE_DUPLICATED]);
}

mmReconcileDialog::mmReconcileDialog(wxWindow* parent, Model_Account::Data* account, mmCheckingPanel* cp)
{
    m_account = account;
    m_checkingPanel = cp;
    m_reconciledBalance = cp->GetReconciledBalance();

    m_ignore  = false;
    this->SetFont(parent->GetFont());

    Create(parent, -1, _t("Reconcile account") + " '" + m_account->ACCOUNTNAME + "'", wxDefaultPosition, wxDefaultSize, wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX, "");
    CreateControls();

    m_settings[SETTING_INCLUDE_VOID] = Model_Infotable::instance().getBool("RECONCILE_DIALOG_INCLUDE_VOID", false);
    m_settings[SETTING_INCLUDE_DUPLICATED] =  Model_Infotable::instance().getBool("RECONCILE_DIALOG_INCLUDE_DUPLICATED", true);

    FillControls(true);
    UpdateAll();

    SetIcon(mmex::getProgramIcon());
    applyColumnSettings();
    Fit();
    SetSize(Model_Infotable::instance().getSize("RECONCILE_DIALOG_SIZE"));
}

void mmReconcileDialog::CreateControls()
{
    // --- Top panel: ---
    wxPanel* topPanel = new wxPanel(this);
    topPanel->SetAutoLayout(true);
    wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);

    topSizer->Add(new wxStaticText(topPanel, wxID_ANY, _t("Statement ending:")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    m_amountCtrl = new mmTextCtrl(topPanel, wxID_ANY, "", wxDefaultPosition, wxSize(150,-1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    m_amountCtrl->Bind(wxEVT_TEXT, &mmReconcileDialog::OnAmountChanged, this);

    topSizer->Add(m_amountCtrl, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);

#ifndef __WXOSX__   // Issue https://github.com/moneymanagerex/moneymanagerex/issues/8000
    m_btnCalc = new wxBitmapButton(topPanel, wxID_ANY, mmBitmapBundle(png::CALCULATOR, mmBitmapButtonSize));
    m_btnCalc->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &mmReconcileDialog::OnCalculator, this);
    m_btnCalc->SetCanFocus(false);
    mmToolTip(m_btnCalc, _t("Open Calculator"));
    topSizer->Add(m_btnCalc, 0, wxRIGHT, 20);
    m_calculaterPopup = new mmCalculatorPopup(m_btnCalc, m_amountCtrl, true);
    m_calculaterPopup->SetCanFocus(false);
#endif

    topSizer->AddStretchSpacer();
    m_btnEdit = new wxButton(topPanel, wxID_ANY, _t("&Edit"));
    m_btnEdit->Bind(wxEVT_BUTTON, &mmReconcileDialog::OnEdit, this);
    m_btnEdit->SetCanFocus(false);
    topSizer->Add(m_btnEdit, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);

    wxButton* btn = new wxButton(topPanel, wxID_ANY, _t("&New"));
    btn->Bind(wxEVT_BUTTON, &mmReconcileDialog::OnNew, this);
    btn->SetCanFocus(false);
    topSizer->Add(btn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 20);

    btn = new wxButton(topPanel, wxID_ANY, _t("&Toggle all"));
    btn->Bind(wxEVT_BUTTON, &mmReconcileDialog::OnToggle, this);
    btn->SetCanFocus(false);
    topSizer->Add(btn, 0, wxRIGHT, 20);

    wxBitmapButton* bbtn = new wxBitmapButton(topPanel, ID_BUTTON, mmBitmapBundle(png::OPTIONS, mmBitmapButtonSize));
    bbtn->Bind(wxEVT_BUTTON, &mmReconcileDialog::OnSettings, this);
    bbtn->SetCanFocus(false);
    mmToolTip(bbtn, _t("Settings"));
    topSizer->Add(bbtn, 0, wxRIGHT, 20);

    topPanel->SetSizer(topSizer);

    // --- middle panel: Listctrls ---
    wxPanel* midPanel = new wxPanel(this);
    wxBoxSizer* midSizer = new wxBoxSizer(wxHORIZONTAL);

    auto addColumns = [](wxListCtrl* list) {
        list->InsertColumn(0, "",            wxLIST_FORMAT_CENTRE, 30);
        list->InsertColumn(1, _t("Date"),    wxLIST_FORMAT_CENTRE);
        list->InsertColumn(2, _t("Number"),  wxLIST_FORMAT_RIGHT);
        list->InsertColumn(3, _t("Payee"),   wxLIST_FORMAT_LEFT);
        list->InsertColumn(4, _t("Amount"),  wxLIST_FORMAT_RIGHT);
        list->InsertColumn(5, _t("State"),   wxLIST_FORMAT_CENTRE, 50);
    };

    wxPanel* leftlistPanel = new wxPanel(midPanel);
    wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);

    m_images.push_back(mmBitmapBundle(png::UNRECONCILED));
    m_images.push_back(mmBitmapBundle(png::RECONCILED));

    wxStaticText* leftLabel = new wxStaticText(leftlistPanel, wxID_ANY, _t("Withdrawals"));
    m_listLeft = new wxListCtrl(leftlistPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
    addColumns(m_listLeft);
    m_listLeft->SetMinSize(wxSize(250,100));

    m_listLeft->Bind(wxEVT_LEFT_DOWN, &mmReconcileDialog::OnLeftItemLeftClick, this);
    m_listLeft->Bind(wxEVT_RIGHT_DOWN, &mmReconcileDialog::OnLeftItemRightClick, this);
    m_listLeft->Bind(wxEVT_KEY_DOWN, &mmReconcileDialog::OnListKeyDown, this);
    m_listLeft->Bind(wxEVT_SET_FOCUS, &mmReconcileDialog::OnLeftFocus, this);
    m_listLeft->Bind(wxEVT_KILL_FOCUS, &mmReconcileDialog::OnLeftFocusKill, this);

    m_listLeft->SetSmallImages(m_images);
    m_listLeft->SetNormalImages(m_images);

    leftSizer->Add(leftLabel, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 5);
    leftSizer->Add(m_listLeft,  1, wxEXPAND | wxALL, 5);

    leftlistPanel->SetSizer(leftSizer);

    wxPanel* rightlistPanel = new wxPanel(midPanel);
    wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);

    wxStaticText* rightLabel = new wxStaticText(rightlistPanel, wxID_ANY, _t("Deposits"));
    m_listRight = new wxListCtrl(rightlistPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
    addColumns(m_listRight);
    m_listRight->SetMinSize(wxSize(250,100));

    m_listRight->Bind(wxEVT_LEFT_DOWN, &mmReconcileDialog::OnRightItemLeftClick, this);
    m_listRight->Bind(wxEVT_RIGHT_DOWN, &mmReconcileDialog::OnRightItemRightClick, this);
    m_listRight->Bind(wxEVT_KEY_DOWN, &mmReconcileDialog::OnListKeyDown, this);
    m_listRight->Bind(wxEVT_SET_FOCUS, &mmReconcileDialog::OnRightFocus, this);
    m_listRight->Bind(wxEVT_KILL_FOCUS, &mmReconcileDialog::OnRightFocusKill, this);

    m_listRight->SetSmallImages(m_images);
    m_listRight->SetNormalImages(m_images);

    rightSizer->Add(rightLabel, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 5);
    rightSizer->Add(m_listRight, 1, wxEXPAND | wxALL, 5);

    rightlistPanel->SetSizer(rightSizer);

    midSizer->Add(leftlistPanel, 1, wxEXPAND | wxALL, 5);
    midSizer->Add(rightlistPanel, 1, wxEXPAND | wxALL, 5);

    midPanel->SetSizer(midSizer);
    midPanel->Bind(wxEVT_SIZE, &mmReconcileDialog::OnSize, this);

    // --- Result: ----
    wxPanel* resPanelOut = new wxPanel(this);
    resPanelOut->SetCanFocus(false);
    wxBoxSizer* resPanelOutSizer = new wxBoxSizer(wxHORIZONTAL);

    wxPanel* resPanel = new wxPanel(resPanelOut);
    wxFlexGridSizer* resSizer = new wxFlexGridSizer(2, 5, 5);

    resSizer->Add(new wxStaticText(resPanel, wxID_ANY, _t("Previous balance:")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 20);
    m_previousCtrl = new wxStaticText(resPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
    m_previousCtrl->SetCanFocus(false);
    resSizer->Add(m_previousCtrl, 0, wxALIGN_RIGHT, 20);

    resSizer->Add(new wxStaticText(resPanel, wxID_ANY, _t("Cleared Balance:")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 20);
    m_clearedBalanceCtrl = new wxStaticText(resPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
    m_clearedBalanceCtrl->SetCanFocus(false);
    resSizer->Add(m_clearedBalanceCtrl, 0, wxALIGN_RIGHT, 20);

    resSizer->Add(new wxStaticText(resPanel, wxID_ANY, _t("Statement ending:")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 20);
    m_endingCtrl = new wxStaticText(resPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
    m_endingCtrl->SetCanFocus(false);
    resSizer->Add(m_endingCtrl, 0, wxALIGN_RIGHT, 20);

    m_differenceLabel = new wxStaticText(resPanel, wxID_ANY, _t("Difference: "));
    resSizer->Add(m_differenceLabel,  0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 20);
    m_differenceCtrl = new wxStaticText(resPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
    int labelWidth = 100;
    m_differenceCtrl->SetMinSize(wxSize(labelWidth, -1));
    m_differenceCtrl->SetCanFocus(false);
    resSizer->Add(m_differenceCtrl, 0, wxALIGN_RIGHT, 20);
    resPanel->SetSizer(resSizer);

    resPanelOutSizer->AddStretchSpacer(1);
    resPanel->SetCanFocus(false);
    resPanelOutSizer->Add(resPanel, 0, wxALL, 10);
    resPanelOut->SetSizer(resPanelOutSizer);

    // --- Button panel ---
    wxPanel* bottomPanel = new wxPanel(this);
    wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);

    m_btnCancel          = new wxButton(bottomPanel, wxID_CANCEL, _t("&Cancel "));

    m_btnReconcileLater  = new wxButton(bottomPanel, wxID_ANY, _t("Reconcile &later"));
    m_btnReconcileLater->Bind(wxEVT_BUTTON, &mmReconcileDialog::OnClose, this);

    m_btnReconcile       = new wxButton(bottomPanel, wxID_OK, _t("&Reconcile"));
    m_btnReconcile->Bind(wxEVT_BUTTON, &mmReconcileDialog::OnClose, this);

    bottomSizer->AddStretchSpacer();
    bottomSizer->Add(m_btnCancel, 0, wxRIGHT, 10);
    bottomSizer->Add(m_btnReconcileLater, 0, wxRIGHT, 10);
    bottomSizer->Add(m_btnReconcile, 0, 0, 10);

    bottomSizer->AddStretchSpacer();
    bottomPanel->SetSizer(bottomSizer);

    // -- settings menu ---
    Bind(wxEVT_MENU, &mmReconcileDialog::OnMenuItemChecked, this, ID_CHECK_SHOW_STATE_COL);
    Bind(wxEVT_MENU, &mmReconcileDialog::OnMenuItemChecked, this, ID_CHECK_SHOW_NUMBER_COL);
    Bind(wxEVT_MENU, &mmReconcileDialog::OnMenuItemChecked, this, ID_CHECK_INCLUDE_VOID);
    Bind(wxEVT_MENU, &mmReconcileDialog::OnMenuItemChecked, this, ID_CHECK_INCLUDE_DUPLICATED);

    // --- Main layout ---
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(topPanel,    0, wxEXPAND | wxALL, 10);
    mainSizer->Add(midPanel,    1, wxEXPAND | wxLEFT | wxRIGHT, 10);
    mainSizer->Add(resPanelOut, 0, wxEXPAND | wxALL, 10);
    mainSizer->Add(bottomPanel, 0, wxEXPAND | wxALL, 10);
    SetSizerAndFit(mainSizer);
}

void mmReconcileDialog::FillControls(bool init)
{
    if (init) {
        double endval;
        wxString endvalue = Model_Infotable::instance().getString(wxString::Format("RECONCILE_ACCOUNT_%lld_END_BALANCE", m_account->ACCOUNTID), "0.00");
        if (!endvalue.ToDouble(&endval)) {
            endval = 0;
        }
        m_amountCtrl->SetValue(endval);
    }

    // get not reconciled transactions
    wxSharedPtr<mmDateRange> date_range;
    date_range = new mmCurrentMonthToDate;
    Model_Checking::Data_Set all_trans = Model_Checking::instance().find(
        Model_Checking::ACCOUNTID(m_account->ACCOUNTID),
        Model_Checking::STATUS(Model_Checking::STATUS_ID_RECONCILED, NOT_EQUAL),
        Model_Checking::DELETEDTIME(wxEmptyString, EQUAL),
        Model_Checking::TRANSDATE(wxDateTime(23,59,59,999), LESS_OR_EQUAL)
    );
    Model_Checking::Data_Set all_trans2 = Model_Checking::instance().find(  // get transfers
        Model_Checking::TOACCOUNTID(m_account->ACCOUNTID),
        Model_Checking::STATUS(Model_Checking::STATUS_ID_RECONCILED, NOT_EQUAL),
        Model_Checking::DELETEDTIME(wxEmptyString, EQUAL),
        Model_Checking::TRANSDATE(wxDateTime(23,59,59,999), LESS_OR_EQUAL)
    );

    all_trans.insert(all_trans.end(), all_trans2.begin(), all_trans2.end());
    std::stable_sort(all_trans.begin(), all_trans.end(), SorterByTRANSDATE());

    long ritemIndex = -1;
    long litemIndex = -1;
    m_itemDataMap.clear();
    long mapidx = 0;

    wxListCtrl* list;
    m_listLeft->DeleteAllItems();
    m_listRight->DeleteAllItems();
    long item;
    m_hiddenDuplicatedBalance = 0.0;
    for (const auto& trx : all_trans) {
        if (!m_settings[SETTING_INCLUDE_VOID] && trx.STATUS == "V") {
            continue;
        }
        if (!m_settings[SETTING_INCLUDE_DUPLICATED] && trx.STATUS == "D") {
            m_hiddenDuplicatedBalance += trx.TRANSAMOUNT;
            continue;
        }
        if (trx.TRANSCODE == "Deposit" || (trx.TRANSCODE == "Transfer" && trx.TOACCOUNTID == m_account->ACCOUNTID)) {
            list = m_listRight;
            item = m_listRight->InsertItem(++ritemIndex, "");
        }
        else {
            list = m_listLeft;
            item = m_listLeft->InsertItem(++litemIndex, "");
        }
        setListItemData(&trx, list, item);
        m_itemDataMap.push_back(trx.TRANSID);
        list->SetItemData(item, mapidx++);
    }
}

void mmReconcileDialog::UpdateAll()
{
    double clearedbalance = m_reconciledBalance;
    for (long i = 0; i < m_listLeft->GetItemCount(); ++i) {
        if (isListItemChecked(m_listLeft, i)) {
            wxString itext = m_listLeft->GetItemText(i, 4);
            double value;
            if (itext.ToDouble(&value)) {
               clearedbalance -= value;
            }
        }
    }
    for (long i = 0; i < m_listRight->GetItemCount(); ++i) {
        if (isListItemChecked(m_listRight, i)) {
            wxString itext = m_listRight->GetItemText(i, 4);
            double value;
            if (itext.ToDouble(&value)) {
               clearedbalance += value;
            }
        }
    }

    double endbalance;
    if (!m_amountCtrl->GetDouble(endbalance)) {
        endbalance = 0.0;
    }

    m_previousCtrl->SetLabel(wxString::Format("%.2f", m_reconciledBalance));
    m_clearedBalanceCtrl->SetLabel(wxString::Format("%.2f", clearedbalance));
    m_endingCtrl->SetLabel(wxString::Format("%.2f", endbalance));
    m_endingCtrl->SetMinSize(m_endingCtrl->GetBestSize());
    m_endingCtrl->GetParent()->Layout();

    double diff = clearedbalance - endbalance - m_hiddenDuplicatedBalance;
    m_differenceCtrl->SetLabel(wxString::Format("%.2f", diff));

    wxFont font = m_differenceCtrl->GetFont();
    int ps = m_previousCtrl->GetFont().GetPointSize();
    if (diff > 0.005 || diff < -0.005) {
        font.SetWeight(wxFONTWEIGHT_BOLD);
        font.SetPointSize(ps +2);
    }
    else {
        font.SetWeight(wxFONTWEIGHT_NORMAL);
        font.SetPointSize(ps);
    }
    m_differenceLabel->SetFont(font);
    m_differenceCtrl->SetFont(font);

    m_btnEdit->Enable(m_listLeft->GetSelectedItemCount() > 0 || m_listRight->GetSelectedItemCount() > 0);

    Refresh();
    Update();
}

void mmReconcileDialog::OnCalculator(wxCommandEvent& WXUNUSED(event))
{
    m_calculaterPopup->Popup();
}

void mmReconcileDialog::OnAmountChanged(wxCommandEvent& WXUNUSED(event))
{
    UpdateAll();
}

void mmReconcileDialog::OnLeftItemLeftClick(wxMouseEvent& event)
{
    processLeftClick(m_listLeft, event.GetPosition());
    event.Skip();
}

void mmReconcileDialog::OnRightItemLeftClick(wxMouseEvent& event)
{
    processLeftClick(m_listRight, event.GetPosition());
    event.Skip();
}

void mmReconcileDialog::processLeftClick(wxListCtrl* list, wxPoint pt)
{
    int flags = 0;
    long idx = list->HitTest(pt, flags);
    if (idx != -1) {
        wxListItem item;
        item.SetId(idx);
        if (list->GetItem(item)) {
            list->SetItemImage(item, item.GetImage() == 0 ? 1 : 0);
        }
        UpdateAll();
    }
}

void mmReconcileDialog::OnLeftItemRightClick(wxMouseEvent& event)
{
    wxPoint pt = event.GetPosition();
    int flags = 0;
    processRightClick(m_listLeft, m_listLeft->HitTest(pt, flags));
    event.Skip();
}

void mmReconcileDialog::OnRightItemRightClick(wxMouseEvent& event)
{
    wxPoint pt = event.GetPosition();
    int flags = 0;
    processRightClick(m_listRight, m_listRight->HitTest(pt, flags));
    event.Skip();
}

void mmReconcileDialog::processRightClick(wxListCtrl* list, long item)
{
    if (item != -1) {
       editTransaction(list, item);
    }
    else {
        newTransaction();
    }
}

void mmReconcileDialog::OnListKeyDown(wxKeyEvent& event)
{
    wxListCtrl* list = nullptr;
    if (m_listLeft->GetSelectedItemCount() > 0) {
        list = m_listLeft;
    }
    else if (m_listRight->GetSelectedItemCount() > 0) {
        list = m_listRight;
    }
    if (list) {
        switch(event.GetKeyCode()) {
            case WXK_LEFT:
            case WXK_RIGHT:
                list = list == m_listLeft ? m_listRight : m_listLeft;
                list->SetFocus();
                if (list->GetItemCount() > 0) {
                    long idx = list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_FOCUSED);
                    list->SetItemState(idx > -1 ? idx : 0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
                }
                break;
            case WXK_SPACE:
                long idx = list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
                if (idx > -1) {
                    wxListItem item;
                    item.SetId(idx);
                    if (list->GetItem(item)) {
                        list->SetItemImage(item, item.GetImage() == 0 ? 1 : 0);
                        if (idx < list->GetItemCount() - 1) {
                            list->SetItemState(idx + 1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
                        }
                        UpdateAll();
                    }
                }
                break;
        }
    }
    event.Skip();
}

void mmReconcileDialog::OnLeftFocus(wxFocusEvent& event)
{
    handleListFocus(m_listLeft);
    event.Skip();
}

void mmReconcileDialog::OnRightFocus(wxFocusEvent& event)
{
    handleListFocus(m_listRight);
    event.Skip();
}

void mmReconcileDialog::handleListFocus(wxListCtrl* list)
{
    if (list->GetItemCount() > 0) {
        long idx = list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_FOCUSED);
        list->SetItemState(idx > -1 ? idx : 0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    }
}

void mmReconcileDialog::OnLeftFocusKill(wxFocusEvent& event)
{
    handleListFocusKill(m_listLeft);
    event.Skip();
}

void mmReconcileDialog::OnRightFocusKill(wxFocusEvent& event)
{
    handleListFocusKill(m_listRight);
    event.Skip();
}

void mmReconcileDialog::handleListFocusKill(wxListCtrl* list)
{
    resetListSelections(list);
}

void mmReconcileDialog::OnNew(wxCommandEvent& WXUNUSED(event))
{
    newTransaction();
}

void mmReconcileDialog::OnSettings(wxCommandEvent& WXUNUSED(event))
{
    wxMenu menu;
    menu.AppendCheckItem(ID_CHECK_SHOW_STATE_COL, _tu("Show status column"));
    menu.AppendCheckItem(ID_CHECK_SHOW_NUMBER_COL, _tu("Show number column"));
    menu.AppendSeparator();
    menu.AppendCheckItem(ID_CHECK_INCLUDE_VOID, _tu("Include void transactions"));
    menu.AppendCheckItem(ID_CHECK_INCLUDE_DUPLICATED, _tu("Include duplicate transactions"));

    menu.FindItem(ID_CHECK_SHOW_STATE_COL)->Check(m_settings[SETTING_SHOW_STATE_COL]);
    menu.FindItem(ID_CHECK_SHOW_NUMBER_COL)->Check(m_settings[SETTING_SHOW_NUMBER_COL]);
    menu.FindItem(ID_CHECK_INCLUDE_VOID)->Check(m_settings[SETTING_INCLUDE_VOID]);
    menu.FindItem(ID_CHECK_INCLUDE_DUPLICATED)->Check(m_settings[SETTING_INCLUDE_DUPLICATED]);
    PopupMenu(&menu);
    //event.Skip();
}

void mmReconcileDialog::OnMenuSelected(wxCommandEvent& WXUNUSED(event))
{
    wxLogDebug("Menu selected");
}

void mmReconcileDialog::OnMenuItemChecked(wxCommandEvent& event)
{
    m_settings[event.GetId() - wxID_HIGHEST - 1] = event.IsChecked();
    switch (event.GetId()) {
        case ID_CHECK_SHOW_NUMBER_COL:
            showHideColumn(event.IsChecked(), 2, 0);
            resizeColumns();
            break;
        case ID_CHECK_SHOW_STATE_COL:
            showHideColumn(event.IsChecked(), 5, 1);
            resizeColumns();
            break;
        case ID_CHECK_INCLUDE_VOID:
            FillControls();
            UpdateAll();
            break;
        case ID_CHECK_INCLUDE_DUPLICATED:
            FillControls();
            UpdateAll();
            break;
    }
}

void mmReconcileDialog::showHideColumn(bool show, int col, int cs) {
    if (!show) {
        m_colwidth[cs] = m_listLeft->GetColumnWidth(col);
    }
    m_listLeft->SetColumnWidth(col, show ? m_colwidth[cs] : 0);
    m_listRight->SetColumnWidth(col, show ? m_colwidth[cs] : 0);
}

void mmReconcileDialog::newTransaction()
{
    mmTransDialog dlg(this, m_account->ACCOUNTID, {0, false}, false, Model_Checking::TYPE_ID_WITHDRAWAL);
    int i = wxID_CANCEL;
    do {
        i = dlg.ShowModal();
        if (i != wxID_CANCEL) {
            m_checkingPanel->refreshList();
            int64 transid = dlg.GetTransactionID();
            const Model_Checking::Data* trx = Model_Checking::instance().get(transid);
            addTransaction2List(trx);
        }
    } while (i == wxID_NEW);
}

void mmReconcileDialog::addTransaction2List(const Model_Checking::Data* trx)
{
    wxListCtrl* list = (trx->TRANSCODE == "Deposit" || (trx->TRANSCODE == "Transfer" && trx->TOACCOUNTID == m_account->ACCOUNTID)) ? m_listRight : m_listLeft;
    long idx = getListIndexByDate(trx, list);
    if (idx == -1) {
        idx = list->GetItemCount();
    }
    long item = list->InsertItem(idx, "");
    setListItemData(trx, list, item);
    list->SetItemData(item, m_itemDataMap.size());
    m_itemDataMap.push_back(trx->TRANSID);
}

void mmReconcileDialog::OnEdit(wxCommandEvent& WXUNUSED(event))
{
    wxListCtrl* list = nullptr;
    if (m_listLeft->GetSelectedItemCount() > 0) {
        list = m_listLeft;
    }
    else if (m_listRight->GetSelectedItemCount() > 0) {
        list = m_listRight;
    }
    if (list) {
        long item = list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        editTransaction(list, item);
    }
}

void mmReconcileDialog::editTransaction(wxListCtrl* list, long item)
{
    int64 transid = m_itemDataMap[list->GetItemData(item)];
    mmTransDialog dlg(this, transid, {transid, false});
    if (dlg.ShowModal() == wxID_OK) {
        m_checkingPanel->refreshList();
        const Model_Checking::Data* trx = Model_Checking::instance().get(transid);
        setListItemData(trx, list, item);
        long idx = getListIndexByDate(trx, list);
        if (idx != item) {
            moveItemData(list, item, idx);
        }
    }
}

long mmReconcileDialog::getListIndexByDate(const Model_Checking::Data* trx, wxListCtrl* list)
{
    int64 id;
    long idx = -1;
    for (long i = 0; i < list->GetItemCount(); ++i) {
        id = m_itemDataMap[list->GetItemData(i)];
        Model_Checking::Data* trl = Model_Checking::instance().get(id);
        if (trx->TRANSDATE.Left(10) < trl->TRANSDATE.Left(10)) {
            idx = i;
            break;
        }
    }
    return idx;
}

void mmReconcileDialog::moveItemData(wxListCtrl* list, int row1, int row2)
{
    std::vector<wxString> coldata;
    for (int i = 0; i < list->GetColumnCount(); i++) {
        coldata.push_back(list->GetItemText(row1, i));
    }
    long rowData = list->GetItemData(row1);
    bool rowischecked = isListItemChecked(list, row1);
    list->SetItemState(row1, 0, wxLIST_STATE_SELECTED);

    list->DeleteItem(row1);

    if (row2 == -1) {
        row2 = list->GetItemCount() + 1;
    }
    long item = list->InsertItem(row2 > row1 ? (row2 > list->GetItemCount() ?  list->GetItemCount() : row2 - 1) : row2, "");

    for (int i = 0; i < list->GetColumnCount(); i++) {
        list->SetItem(item, i , coldata[i]);
    }
    list->SetItemData(item, rowData);
    list->SetItemImage(item, rowischecked ? 1 : 0);
    list->SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
}

void mmReconcileDialog::setListItemData(const Model_Checking::Data* trx, wxListCtrl* list, long item)
{
    wxString prefix = trx->TRANSCODE == "Transfer" ? (trx->TOACCOUNTID == m_account->ACCOUNTID ? "< " : "> ") : "";
    wxString payeeName = (trx->TRANSCODE == "Transfer") ? Model_Account::get_account_name(trx->TOACCOUNTID == m_account->ACCOUNTID ? trx->ACCOUNTID : trx->TOACCOUNTID): Model_Payee::get_payee_name(trx->PAYEEID);
    list->SetItem(item, 1, mmGetDateTimeForDisplay(trx->TRANSDATE));
    list->SetItem(item, 2, trx->TRANSACTIONNUMBER);
    list->SetItem(item, 3, prefix + payeeName);
    list->SetItem(item, 4, wxString::Format("%.2f", trx->TRANSAMOUNT));
    list->SetItem(item, 5, trx->STATUS);
    list->SetItemImage(item, trx->STATUS == "F" ? 1 : 0);
}

void mmReconcileDialog::OnToggle(wxCommandEvent& WXUNUSED(event))
{
    DoWindowsFreezeThaw(this);
    bool isChecked = true;
    for (long i = 0; i < m_listLeft->GetItemCount(); ++i) {
        if (!isListItemChecked(m_listLeft, i)) {
            isChecked = false;
            break;
        }
    }
    if (isChecked) {
        for (long i = 0; i < m_listRight->GetItemCount(); ++i) {
            if (!isListItemChecked(m_listRight, i)) {
                isChecked = false;
                break;
            }
        }
    }
    for (long i = 0; i < m_listLeft->GetItemCount(); ++i) {
        m_listLeft->SetItemImage(i, isChecked ? 0 : 1);
    }
    for (long i = 0; i < m_listRight->GetItemCount(); ++i) {
        m_listRight->SetItemImage(i, isChecked ? 0 : 1);
    }
    UpdateAll();
    resetListSelections(m_listLeft);
    resetListSelections(m_listRight);
    DoWindowsFreezeThaw(this);
}

void mmReconcileDialog::resetListSelections(wxListCtrl* list)
{
    if (list->GetSelectedItemCount() > 0) {
        long item = list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (item > -1) {
            list->SetItemState(item, 0, wxLIST_STATE_SELECTED);
        }
    }
}

bool mmReconcileDialog::isListItemChecked(wxListCtrl* list, long item)
{
    bool checked = false;
    wxListItem litem;
    litem.SetId(item);
    if (list->GetItem(litem)) {
        checked = litem.GetImage() == 1;
    }
    return checked;
}

void mmReconcileDialog::OnSize(wxSizeEvent& event)
{
    resizeColumns();
    event.Skip();
}

void mmReconcileDialog::resizeColumns()
{
    auto setColWidth = [] (wxListCtrl* list) {
        int w = 0;
        for (int i = 0; i < list->GetColumnCount(); i++) {
            w += (i != 3) ? list->GetColumnWidth(i) : 0;
        }
        wxSize size = list->GetSize();
        int nwidth = size.GetWidth() - w;
        if (nwidth > 0) {
            list->SetColumnWidth(3, nwidth);
        }
    };

    setColWidth(m_listLeft);
    setColWidth(m_listRight);
}

void mmReconcileDialog::applyColumnSettings()
{
    m_colwidth[0] = m_listLeft->GetColumnWidth(2);
    m_colwidth[1] = m_listLeft->GetColumnWidth(5);

    m_settings[SETTING_SHOW_STATE_COL] = Model_Infotable::instance().getBool("RECONCILE_DIALOG_SHOW_STATE_COL", true);
    if (!m_settings[SETTING_SHOW_STATE_COL]) {
        showHideColumn(false, 5, 1);
    }

    m_settings[SETTING_SHOW_NUMBER_COL] = Model_Infotable::instance().getBool("RECONCILE_DIALOG_SHOW_NUMBER_COL", true);
    if (!m_settings[SETTING_SHOW_NUMBER_COL]) {
        showHideColumn(false, 2, 0);
    }
    resizeColumns();
}

void mmReconcileDialog::OnClose(wxCommandEvent& event)
{
    auto saveItem = [](int64 id, bool state, bool final) {
        Model_Checking::Data* trx = Model_Checking::instance().get(id);
        if (state) {
            trx->STATUS = final ? "R" : "F";
        }
        else {
            if (trx->STATUS == "F") {
                trx->STATUS = "";
            }
        }
        Model_Checking::instance().save(trx);
    };

    if (event.GetId() != wxID_CANCEL) {
        Model_Infotable::instance().setString(wxString::Format("RECONCILE_ACCOUNT_%lld_END_BALANCE", m_account->ACCOUNTID), m_endingCtrl->GetLabelText());

        // Save state:
        for (long i = 0; i < m_listLeft->GetItemCount(); ++i) {
            saveItem(m_itemDataMap[m_listLeft->GetItemData(i)], isListItemChecked(m_listLeft, i), event.GetId() == wxID_OK);
        }
        for (long i = 0; i < m_listRight->GetItemCount(); ++i) {
            saveItem(m_itemDataMap[m_listRight->GetItemData(i)], isListItemChecked(m_listRight, i), event.GetId() == wxID_OK);
        }
    }

    EndModal(wxID_OK);
}
