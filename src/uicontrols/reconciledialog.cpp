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

#include "base/constants.h"
#include "base/images_list.h"
#include "util/mmDate.h"
#include "util/mmDateRange.h"
#include "util/mmCalcValidator.h"

#include "model/AccountModel.h"
#include "model/InfoModel.h"
#include "model/PayeeModel.h"
#include "model/PrefModel.h"
#include "model/TrxModel.h"

#include "dialog/TrxDialog.h"
#include "reconciledialog.h"

wxIMPLEMENT_DYNAMIC_CLASS(mmReconcileDialog, wxDialog);

mmReconcileDialog::mmReconcileDialog()
{
}

mmReconcileDialog::~mmReconcileDialog()
{
    wxSize size = GetSize();
    InfoModel::instance().setSize("RECONCILE_DIALOG_SIZE", size);
    InfoModel::instance().setBool("RECONCILE_DIALOG_SHOW_STATE_COL",
        m_settings[SETTING_SHOW_STATE_COL]
    );
    InfoModel::instance().setBool("RECONCILE_DIALOG_SHOW_NUMBER_COL",
        m_settings[SETTING_SHOW_NUMBER_COL]
    );
    InfoModel::instance().setBool("RECONCILE_DIALOG_INCLUDE_VOID",
        m_settings[SETTING_INCLUDE_VOID]
    );
    InfoModel::instance().setBool("RECONCILE_DIALOG_INCLUDE_DUPLICATED",
        m_settings[SETTING_INCLUDE_DUPLICATED]
    );
}

mmReconcileDialog::mmReconcileDialog(
    wxWindow* parent,
    const AccountData* account_n,
    JournalPanel* cp
) {
    m_account = account_n;
    m_checkingPanel = cp;
    m_reconciledBalance = cp->getTodayReconciledBalance();
    m_currency = CurrencyModel::instance().get_id_data_n(account_n->m_currency_id);
    m_ignore  = false;
    this->SetFont(parent->GetFont());

    Create(parent, -1,
        _t("Reconcile account") + " '" + m_account->m_name + "'",
        wxDefaultPosition, wxDefaultSize,
        wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX,
        ""
    );
    CreateControls();

    m_settings[SETTING_INCLUDE_VOID] = InfoModel::instance().getBool(
        "RECONCILE_DIALOG_INCLUDE_VOID", false
    );
    m_settings[SETTING_INCLUDE_DUPLICATED] =  InfoModel::instance().getBool(
        "RECONCILE_DIALOG_INCLUDE_DUPLICATED", true
    );

    FillControls(true);
    UpdateAll();

    const wxAcceleratorEntry entries[] = {
        wxAcceleratorEntry(wxACCEL_NORMAL, WXK_F9, wxID_NEW),
        wxAcceleratorEntry(wxACCEL_CTRL, static_cast<int>('S'), wxID_SAVE),
    };
    wxAcceleratorTable tab(sizeof(entries) / sizeof(*entries), entries);
    SetAcceleratorTable(tab);
    Bind(wxEVT_MENU, &mmReconcileDialog::OnNew, this, wxID_NEW);
    Bind(wxEVT_MENU, &mmReconcileDialog::OnClose, this, wxID_SAVE);

    SetIcon(mmex::getProgramIcon());
    applyColumnSettings();
    Fit();
    SetSize(InfoModel::instance().getSize("RECONCILE_DIALOG_SIZE"));
}

void mmReconcileDialog::CreateControls()
{
    // --- Top panel: ---
    wxPanel* topPanel = new wxPanel(this);
    topPanel->SetAutoLayout(true);
    wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);

    topSizer->Add(new wxStaticText(topPanel, wxID_ANY, _t("Statement ending balance:")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    m_amountCtrl = new mmTextCtrl(topPanel, wxID_ANY, "", wxDefaultPosition, wxSize(150,-1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    m_amountCtrl->Bind(wxEVT_TEXT, &mmReconcileDialog::OnAmountChanged, this);

    topSizer->Add(m_amountCtrl, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);

    m_btnCalc = new wxBitmapButton(topPanel, wxID_ANY, mmBitmapBundle(png::CALCULATOR, mmBitmapButtonSize));
    m_btnCalc->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &mmReconcileDialog::OnCalculator, this);
    m_btnCalc->SetCanFocus(false);
    mmToolTip(m_btnCalc, _t("Open Calculator"));
    topSizer->Add(m_btnCalc, 0, wxRIGHT, 20);
    m_calculaterPopup = new mmCalculatorPopup(m_btnCalc, m_amountCtrl, true);
    m_calculaterPopup->SetCanFocus(false);

    topSizer->AddStretchSpacer();
    m_btnEdit = new wxButton(topPanel, wxID_ANY, _t("&Edit"));
    m_btnEdit->Bind(wxEVT_BUTTON, &mmReconcileDialog::OnEdit, this);
    m_btnEdit->SetCanFocus(false);
    m_btnEdit->Enable(false);

    topSizer->Add(m_btnEdit, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);

    wxButton* btn = new wxButton(topPanel, wxID_ANY, _t("&New"));
    btn->Bind(wxEVT_BUTTON, &mmReconcileDialog::OnNew, this);
    btn->SetCanFocus(false);
    topSizer->Add(btn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 20);

    btn = new wxButton(topPanel, wxID_ANY, _t("&All (un)cleared"));
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
        list->InsertColumn(5, _t("Status"),  wxLIST_FORMAT_CENTRE, 50);
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
    m_listLeft->Bind(wxEVT_LIST_ITEM_SELECTED, &mmReconcileDialog::OnListItemSelection, this);
    m_listLeft->Bind(wxEVT_LIST_ITEM_DESELECTED, &mmReconcileDialog::OnListItemSelection, this);

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
    m_listRight->Bind(wxEVT_LIST_ITEM_SELECTED, &mmReconcileDialog::OnListItemSelection, this);
    m_listRight->Bind(wxEVT_LIST_ITEM_DESELECTED, &mmReconcileDialog::OnListItemSelection, this);

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

    resSizer->Add(new wxStaticText(resPanel, wxID_ANY, _t("Statement opening balance:")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 20);
    m_previousCtrl = new wxStaticText(resPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
    m_previousCtrl->SetCanFocus(false);
    resSizer->Add(m_previousCtrl, 0, wxALIGN_RIGHT, 20);

    resSizer->Add(new wxStaticText(resPanel, wxID_ANY, _t("Cleared balance:")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 20);
    m_clearedBalanceCtrl = new wxStaticText(resPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
    m_clearedBalanceCtrl->SetCanFocus(false);
    resSizer->Add(m_clearedBalanceCtrl, 0, wxALIGN_RIGHT, 20);

    resSizer->Add(new wxStaticText(resPanel, wxID_ANY, _t("Statement ending balance:")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 20);
    m_endingCtrl = new wxStaticText(resPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
    m_endingCtrl->SetCanFocus(false);
    resSizer->Add(m_endingCtrl, 0, wxALIGN_RIGHT, 20);

    m_differenceLabel = new wxStaticText(resPanel, wxID_ANY, _t("Difference:"));
    resSizer->Add(m_differenceLabel,  0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 20);
    m_differenceCtrl = new wxStaticText(resPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
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

    m_btnReconcileLater  = new wxButton(bottomPanel, wxID_ANY, _t("&Finish later"));
    m_btnReconcileLater->Bind(wxEVT_BUTTON, &mmReconcileDialog::OnClose, this);

    m_btnReconcile       = new wxButton(bottomPanel, wxID_OK, _t("&Done"));
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
        wxString endvalue = InfoModel::instance().getString(wxString::Format("RECONCILE_ACCOUNT_%lld_END_BALANCE", m_account->m_id), "0.00");
        if (!CurrencyModel::instance().fromString(endvalue, endval, m_currency)) {
            endval = 0;
        }
        m_amountCtrl->SetValue(endval);
    }

    // get not reconciled transactions
    wxSharedPtr<mmDateRange> date_range;
    date_range = new mmCurrentMonthToDate;
    TrxModel::DataA trx_a = TrxModel::instance().find(
        TrxModel::DATE(OP_LE, mmDate::today()),
        TrxModel::STATUS(OP_NE, TrxStatus(TrxStatus::e_reconciled)),
        TrxCol::ACCOUNTID(m_account->m_id),
        TrxModel::IS_DELETED(false)
    );
    TrxModel::DataA all_trans2 = TrxModel::instance().find(  // get transfers
        TrxModel::DATE(OP_LE, mmDate::today()),
        TrxModel::STATUS(OP_NE, TrxStatus(TrxStatus::e_reconciled)),
        TrxCol::TOACCOUNTID(m_account->m_id),
        TrxModel::IS_DELETED(false)
    );

    trx_a.insert(trx_a.end(), all_trans2.begin(), all_trans2.end());
    std::stable_sort(trx_a.begin(), trx_a.end(), TrxData::SorterByDateTime());

    long ritemIndex = -1;
    long litemIndex = -1;
    m_itemDataMap.clear();
    long mapidx = 0;

    wxListCtrl* list;
    m_listLeft->DeleteAllItems();
    m_listRight->DeleteAllItems();
    long item;
    m_hiddenDuplicatedBalance = 0.0;
    for (const auto& trx_d : trx_a) {
        if (!m_settings[SETTING_INCLUDE_VOID] && trx_d.is_void()) {
            continue;
        }
        if (!m_settings[SETTING_INCLUDE_DUPLICATED] && trx_d.m_status.id() == TrxStatus::e_duplicate) {
            m_hiddenDuplicatedBalance += trx_d.m_amount;
            continue;
        }
        if (trx_d.is_deposit() ||
            (trx_d.is_transfer() && trx_d.m_to_account_id_n == m_account->m_id)
        ) {
            list = m_listRight;
            item = m_listRight->InsertItem(++ritemIndex, "");
        }
        else {
            list = m_listLeft;
            item = m_listLeft->InsertItem(++litemIndex, "");
        }
        setListItemData(&trx_d, list, item);
        m_itemDataMap.push_back(trx_d.m_id);
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
            if (CurrencyModel::instance().fromString(itext, value, m_currency)) {
               clearedbalance -= value;
            }
        }
    }
    for (long i = 0; i < m_listRight->GetItemCount(); ++i) {
        if (isListItemChecked(m_listRight, i)) {
            wxString itext = m_listRight->GetItemText(i, 4);
            double value;
            if (CurrencyModel::instance().fromString(itext, value, m_currency)) {
               clearedbalance += value;
            }
        }
    }

    double endbalance;
    if (!m_amountCtrl->GetDouble(endbalance)) {
        endbalance = 0.0;
    }

    m_previousCtrl->SetLabel(CurrencyModel::instance().toCurrency(m_reconciledBalance, m_currency));
    m_clearedBalanceCtrl->SetLabel(CurrencyModel::instance().toCurrency(clearedbalance, m_currency));
    m_endingCtrl->SetLabel(CurrencyModel::instance().toCurrency(endbalance, m_currency));
    m_endingCtrl->SetMinSize(m_endingCtrl->GetBestSize());
    m_endingCtrl->GetParent()->Layout();

    double diff = clearedbalance - endbalance - m_hiddenDuplicatedBalance;

    m_differenceCtrl->SetLabel(CurrencyModel::instance().toCurrency(diff, m_currency));

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

    Refresh();
    Layout();
    Update();
}

void mmReconcileDialog::OnListItemSelection(wxListEvent& WXUNUSED(event))
{
   updateButtonState();
}

void mmReconcileDialog::updateButtonState()
{
    bool hasSelection = false;
    if (m_listLeft->HasFocus() || m_listRight->HasFocus()) {
        hasSelection = m_listLeft->GetNextItem(-1,wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED) != -1 ||
                       m_listRight->GetNextItem(-1,wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED) != -1 ;
    }
    m_btnEdit->Enable(hasSelection);
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
    updateButtonState();
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
    menu.AppendCheckItem(ID_CHECK_SHOW_STATE_COL, _tu("Show &status column"));
    menu.AppendCheckItem(ID_CHECK_SHOW_NUMBER_COL, _tu("Show &number column"));
    menu.AppendSeparator();
    menu.AppendCheckItem(ID_CHECK_INCLUDE_VOID, _tu("Include &void transactions"));
    menu.AppendCheckItem(ID_CHECK_INCLUDE_DUPLICATED, _tu("Include &duplicate transactions"));

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
    TrxDialog dlg(this, m_account->m_id, {0, false}, false, TrxType(TrxType::e_withdrawal));
    int i = wxID_CANCEL;
    do {
        i = dlg.ShowModal();
        if (i != wxID_CANCEL) {
            m_checkingPanel->refreshList();
            int64 transid = dlg.GetTransactionID();
            const TrxData* trx = TrxModel::instance().get_id_data_n(transid);
            addTransaction2List(trx);
        }
    } while (i == wxID_NEW);
}

void mmReconcileDialog::addTransaction2List(const TrxData* trx_n)
{
    wxListCtrl* list = (trx_n->is_deposit() ||
        (trx_n->is_transfer() && trx_n->m_to_account_id_n == m_account->m_id)
    ) ? m_listRight : m_listLeft;
    long idx = getListIndexByDate(trx_n, list);
    if (idx == -1) {
        idx = list->GetItemCount();
    }
    long item = list->InsertItem(idx, "");
    setListItemData(trx_n, list, item);
    list->SetItemData(item, m_itemDataMap.size());
    m_itemDataMap.push_back(trx_n->m_id);
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
    int64 trx_id = m_itemDataMap[list->GetItemData(item)];
    TrxDialog dlg(this, trx_id, JournalKey(-1, trx_id));
    if (dlg.ShowModal() == wxID_OK) {
        m_checkingPanel->refreshList();
        const TrxData* trx_n = TrxModel::instance().get_id_data_n(trx_id);
        setListItemData(trx_n, list, item);
        long idx = getListIndexByDate(trx_n, list);
        if (idx != item) {
            moveItemData(list, item, idx);
        }
    }
}

long mmReconcileDialog::getListIndexByDate(const TrxData* trx_n, wxListCtrl* list)
{
    long idx = -1;
    for (long i = 0; i < list->GetItemCount(); ++i) {
        int64 other_id = m_itemDataMap[list->GetItemData(i)];
        const TrxData* other_trx_n = TrxModel::instance().get_id_data_n(other_id);
        if (trx_n->m_date() < other_trx_n->m_date()) {
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

void mmReconcileDialog::setListItemData(const TrxData* trx_n, wxListCtrl* list, long item)
{
    wxString prefix = trx_n->is_transfer()
        ? (trx_n->m_to_account_id_n == m_account->m_id ? "< " : "> ")
        : "";
    wxString payeeName = (trx_n->is_transfer())
        ? AccountModel::instance().get_id_name(trx_n->m_to_account_id_n == m_account->m_id
            ? trx_n->m_account_id
            : trx_n->m_to_account_id_n
        ) : PayeeModel::instance().get_id_name(trx_n->m_payee_id_n);
    list->SetItem(item, 1, mmGetDateTimeForDisplay(trx_n->m_date_time.isoDateTime()));
    list->SetItem(item, 2, trx_n->m_number);
    list->SetItem(item, 3, prefix + payeeName);
    list->SetItem(item, 4, CurrencyModel::instance().toString(trx_n->m_amount,m_currency));
    list->SetItem(item, 5, trx_n->m_status.key());
    list->SetItemImage(item, (trx_n->m_status.id() == TrxStatus::e_followup) ? 1 : 0);
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

    m_settings[SETTING_SHOW_STATE_COL] = InfoModel::instance().getBool("RECONCILE_DIALOG_SHOW_STATE_COL", true);
    if (!m_settings[SETTING_SHOW_STATE_COL]) {
        showHideColumn(false, 5, 1);
    }

    m_settings[SETTING_SHOW_NUMBER_COL] = InfoModel::instance().getBool("RECONCILE_DIALOG_SHOW_NUMBER_COL", true);
    if (!m_settings[SETTING_SHOW_NUMBER_COL]) {
        showHideColumn(false, 2, 0);
    }
    resizeColumns();
}

void mmReconcileDialog::OnClose(wxCommandEvent& event)
{
    auto saveItem = [](int64 id, bool state, bool final) {
        TrxData* trx_n = TrxModel::instance().unsafe_get_id_data_n(id);
        if (state) {
            trx_n->m_status = TrxStatus(final
                ? TrxStatus::e_reconciled
                : TrxStatus::e_followup
            );
        }
        else {
            if (trx_n->m_status.id() == TrxStatus::e_followup) {
                trx_n->m_status = TrxStatus(TrxStatus::e_unreconciled);
            }
        }
        TrxModel::instance().unsafe_save_trx_n(trx_n);
    };

    if (event.GetId() != wxID_CANCEL) {
        InfoModel::instance().setString(wxString::Format("RECONCILE_ACCOUNT_%lld_END_BALANCE", m_account->m_id), m_endingCtrl->GetLabelText());

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
