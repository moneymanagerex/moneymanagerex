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

#include "ReconcileDialog.h"

#include "base/_constants.h"
#include "base/mmDate.h"
#include "util/mmImage.h"
#include "util/mmPath.h"
#include "util/mmDateRange.h"
#include "util/mmCalcValidator.h"

#include "model/AccountModel.h"
#include "model/InfoModel.h"
#include "model/PayeeModel.h"
#include "model/PrefModel.h"
#include "model/TrxModel.h"

#include "TrxDialog.h"

wxIMPLEMENT_DYNAMIC_CLASS(ReconcileDialog, wxDialog);

ReconcileDialog::ReconcileDialog()
{
}

ReconcileDialog::ReconcileDialog(
    wxWindow* parent_win,
    const AccountData* account_n,
    JournalPanel* journal_panel_n
) {
    m_account_n = account_n;
    w_journal = journal_panel_n;
    m_reconciledBalance = journal_panel_n->todayReconciledBalance();
    m_currency_n = CurrencyModel::instance().get_idN_data_n(account_n->m_currency_id);
    m_ignore  = false;
    this->SetFont(parent_win->GetFont());

    Create(
        parent_win, -1,
        _t("Reconcile account") + " '" + m_account_n->m_name + "'",
        wxDefaultPosition, wxDefaultSize,
        wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX,
        ""
    );
    CreateControls();

    m_settings[SETTING_INCLUDE_VOID] = InfoModel::instance().getBool(
        "RECONCILE_DIALOG_INCLUDE_VOID", false
    );
    m_settings[SETTING_INCLUDE_DUPLICATED] = InfoModel::instance().getBool(
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
    Bind(wxEVT_MENU, &ReconcileDialog::OnNew, this, wxID_NEW);
    Bind(wxEVT_MENU, &ReconcileDialog::OnClose, this, wxID_SAVE);

    SetIcon(mmPath::getProgramIcon());
    applyColumnSettings();
    Fit();
    SetSize(InfoModel::instance().getSize("RECONCILE_DIALOG_SIZE"));
}

ReconcileDialog::~ReconcileDialog()
{
    wxSize size = GetSize();
    InfoModel::instance().saveSize(
        "RECONCILE_DIALOG_SIZE",
        size
    );
    InfoModel::instance().saveBool(
        "RECONCILE_DIALOG_SHOW_STATE_COL",
        m_settings[SETTING_SHOW_STATE_COL]
    );
    InfoModel::instance().saveBool(
        "RECONCILE_DIALOG_SHOW_NUMBER_COL",
        m_settings[SETTING_SHOW_NUMBER_COL]
    );
    InfoModel::instance().saveBool(
        "RECONCILE_DIALOG_INCLUDE_VOID",
        m_settings[SETTING_INCLUDE_VOID]
    );
    InfoModel::instance().saveBool(
        "RECONCILE_DIALOG_INCLUDE_DUPLICATED",
        m_settings[SETTING_INCLUDE_DUPLICATED]
    );
}

void ReconcileDialog::CreateControls()
{
    // --- Top panel: ---
    wxPanel* topPanel = new wxPanel(this);
    topPanel->SetAutoLayout(true);
    wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);

    topSizer->Add(
        new wxStaticText(topPanel, wxID_ANY, _t("Statement ending balance:")),
        0,
        wxALIGN_CENTER_VERTICAL | wxRIGHT,
        5
    );
    w_amount_text = new mmTextCtrl(topPanel, wxID_ANY,
        "",
        wxDefaultPosition, wxSize(150,-1),
        wxALIGN_RIGHT | wxTE_PROCESS_ENTER,
        mmCalcValidator()
    );
    w_amount_text->Bind(wxEVT_TEXT, &ReconcileDialog::OnAmountChanged, this);

    topSizer->Add(w_amount_text, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);

    w_calc_btn = new wxBitmapButton(topPanel, wxID_ANY,
        mmImage::bitmapBundle(mmImage::png::CALCULATOR, mmImage::bitmapButtonSize)
    );
    w_calc_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ReconcileDialog::OnCalculator, this);
    w_calc_btn->SetCanFocus(false);
    mmToolTip(w_calc_btn, _t("Open Calculator"));
    topSizer->Add(w_calc_btn, 0, wxRIGHT, 20);
    w_calc_popup = new mmCalcPopup(w_calc_btn, w_amount_text, true);
    w_calc_popup->SetCanFocus(false);

    topSizer->AddStretchSpacer();
    w_edit_btn = new wxButton(topPanel, wxID_ANY, _t("&Edit"));
    w_edit_btn->Bind(wxEVT_BUTTON, &ReconcileDialog::OnEdit, this);
    w_edit_btn->SetCanFocus(false);
    w_edit_btn->Enable(false);

    topSizer->Add(w_edit_btn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);

    wxButton* btn = new wxButton(topPanel, wxID_ANY, _t("&New"));
    btn->Bind(wxEVT_BUTTON, &ReconcileDialog::OnNew, this);
    btn->SetCanFocus(false);
    topSizer->Add(btn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 20);

    btn = new wxButton(topPanel, wxID_ANY, _t("&All (un)cleared"));
    btn->Bind(wxEVT_BUTTON, &ReconcileDialog::OnToggle, this);
    btn->SetCanFocus(false);
    topSizer->Add(btn, 0, wxRIGHT, 20);

    wxBitmapButton* bbtn = new wxBitmapButton(topPanel, ID_BUTTON,
        mmImage::bitmapBundle(mmImage::png::OPTIONS, mmImage::bitmapButtonSize)
    );
    bbtn->Bind(wxEVT_BUTTON, &ReconcileDialog::OnSettings, this);
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

    w_images.push_back(mmImage::bitmapBundle(mmImage::png::UNRECONCILED));
    w_images.push_back(mmImage::bitmapBundle(mmImage::png::RECONCILED));

    wxStaticText* leftLabel = new wxStaticText(leftlistPanel, wxID_ANY,
        _t("Withdrawals")
    );
    w_left_list = new wxListCtrl(leftlistPanel, wxID_ANY,
        wxDefaultPosition, wxDefaultSize,
        wxLC_REPORT | wxLC_SINGLE_SEL
    );
    addColumns(w_left_list);
    w_left_list->SetMinSize(wxSize(250,100));

    w_left_list->Bind(wxEVT_LEFT_DOWN, &ReconcileDialog::OnLeftItemLeftClick, this);
    w_left_list->Bind(wxEVT_RIGHT_DOWN, &ReconcileDialog::OnLeftItemRightClick, this);
    w_left_list->Bind(wxEVT_KEY_DOWN, &ReconcileDialog::OnListKeyDown, this);
    w_left_list->Bind(wxEVT_SET_FOCUS, &ReconcileDialog::OnLeftFocus, this);
    w_left_list->Bind(wxEVT_KILL_FOCUS, &ReconcileDialog::OnLeftFocusKill, this);
    w_left_list->Bind(wxEVT_LIST_ITEM_SELECTED, &ReconcileDialog::OnListItemSelection, this);
    w_left_list->Bind(wxEVT_LIST_ITEM_DESELECTED, &ReconcileDialog::OnListItemSelection, this);

    w_left_list->SetSmallImages(w_images);
    w_left_list->SetNormalImages(w_images);

    leftSizer->Add(leftLabel, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 5);
    leftSizer->Add(w_left_list,  1, wxEXPAND | wxALL, 5);

    leftlistPanel->SetSizer(leftSizer);

    wxPanel* rightlistPanel = new wxPanel(midPanel);
    wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);

    wxStaticText* rightLabel = new wxStaticText(rightlistPanel, wxID_ANY, _t("Deposits"));
    w_right_list = new wxListCtrl(rightlistPanel, wxID_ANY,
        wxDefaultPosition, wxDefaultSize,
        wxLC_REPORT | wxLC_SINGLE_SEL
    );
    addColumns(w_right_list);
    w_right_list->SetMinSize(wxSize(250,100));

    w_right_list->Bind(wxEVT_LEFT_DOWN, &ReconcileDialog::OnRightItemLeftClick, this);
    w_right_list->Bind(wxEVT_RIGHT_DOWN, &ReconcileDialog::OnRightItemRightClick, this);
    w_right_list->Bind(wxEVT_KEY_DOWN, &ReconcileDialog::OnListKeyDown, this);
    w_right_list->Bind(wxEVT_SET_FOCUS, &ReconcileDialog::OnRightFocus, this);
    w_right_list->Bind(wxEVT_KILL_FOCUS, &ReconcileDialog::OnRightFocusKill, this);
    w_right_list->Bind(wxEVT_LIST_ITEM_SELECTED, &ReconcileDialog::OnListItemSelection, this);
    w_right_list->Bind(wxEVT_LIST_ITEM_DESELECTED, &ReconcileDialog::OnListItemSelection, this);

    w_right_list->SetSmallImages(w_images);
    w_right_list->SetNormalImages(w_images);

    rightSizer->Add(rightLabel, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 5);
    rightSizer->Add(w_right_list, 1, wxEXPAND | wxALL, 5);

    rightlistPanel->SetSizer(rightSizer);

    midSizer->Add(leftlistPanel, 1, wxEXPAND | wxALL, 5);
    midSizer->Add(rightlistPanel, 1, wxEXPAND | wxALL, 5);

    midPanel->SetSizer(midSizer);
    midPanel->Bind(wxEVT_SIZE, &ReconcileDialog::OnSize, this);

    // --- Result: ----
    wxPanel* resPanelOut = new wxPanel(this);
    resPanelOut->SetCanFocus(false);
    wxBoxSizer* resPanelOutSizer = new wxBoxSizer(wxHORIZONTAL);

    wxPanel* resPanel = new wxPanel(resPanelOut);
    wxFlexGridSizer* resSizer = new wxFlexGridSizer(2, 5, 5);

    resSizer->Add(
        new wxStaticText(resPanel, wxID_ANY, _t("Statement opening balance:")),
        0,
        wxALIGN_CENTER_VERTICAL | wxRIGHT,
        20
    );
    w_previous_bal_text = new wxStaticText(resPanel, wxID_ANY,
        "",
        wxDefaultPosition, wxDefaultSize,
        wxALIGN_RIGHT
    );
    w_previous_bal_text->SetCanFocus(false);
    resSizer->Add(w_previous_bal_text, 0, wxALIGN_RIGHT, 20);

    resSizer->Add(
        new wxStaticText(resPanel, wxID_ANY, _t("Cleared balance:")),
        0,
        wxALIGN_CENTER_VERTICAL | wxRIGHT,
        20
    );
    w_cleared_bal_text = new wxStaticText(resPanel, wxID_ANY,
        "",
        wxDefaultPosition, wxDefaultSize,
        wxALIGN_RIGHT
    );
    w_cleared_bal_text->SetCanFocus(false);
    resSizer->Add(w_cleared_bal_text, 0, wxALIGN_RIGHT, 20);

    resSizer->Add(
        new wxStaticText(resPanel, wxID_ANY, _t("Statement ending balance:")),
        0,
        wxALIGN_CENTER_VERTICAL | wxRIGHT,
        20
    );
    w_ending_bal_text = new wxStaticText(resPanel, wxID_ANY,
        "",
        wxDefaultPosition, wxDefaultSize,
        wxALIGN_RIGHT
    );
    w_ending_bal_text->SetCanFocus(false);
    resSizer->Add(w_ending_bal_text, 0, wxALIGN_RIGHT, 20);

    w_diff_label = new wxStaticText(resPanel, wxID_ANY, _t("Difference:"));
    resSizer->Add(w_diff_label, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 20);
    w_diff_text = new wxStaticText(resPanel, wxID_ANY,
        "",
        wxDefaultPosition, wxDefaultSize,
        wxALIGN_RIGHT
    );
    w_diff_text->SetCanFocus(false);
    resSizer->Add(w_diff_text, 0, wxALIGN_RIGHT, 20);
    resPanel->SetSizer(resSizer);

    resPanelOutSizer->AddStretchSpacer(1);
    resPanel->SetCanFocus(false);
    resPanelOutSizer->Add(resPanel, 0, wxALL, 10);
    resPanelOut->SetSizer(resPanelOutSizer);

    // --- Button panel ---
    wxPanel* bottomPanel = new wxPanel(this);
    wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);

    w_cancel_btn = new wxButton(bottomPanel, wxID_CANCEL, _t("&Cancel "));

    w_later_btn = new wxButton(bottomPanel, wxID_ANY, _t("&Finish later"));
    w_later_btn->Bind(wxEVT_BUTTON, &ReconcileDialog::OnClose, this);

    w_reconcile_btn = new wxButton(bottomPanel, wxID_OK, _t("&Done"));
    w_reconcile_btn->Bind(wxEVT_BUTTON, &ReconcileDialog::OnClose, this);

    bottomSizer->AddStretchSpacer();
    bottomSizer->Add(w_cancel_btn, 0, wxRIGHT, 10);
    bottomSizer->Add(w_later_btn, 0, wxRIGHT, 10);
    bottomSizer->Add(w_reconcile_btn, 0, 0, 10);

    bottomSizer->AddStretchSpacer();
    bottomPanel->SetSizer(bottomSizer);

    // -- settings menu ---
    Bind(wxEVT_MENU, &ReconcileDialog::OnMenuItemChecked, this, ID_CHECK_SHOW_STATE_COL);
    Bind(wxEVT_MENU, &ReconcileDialog::OnMenuItemChecked, this, ID_CHECK_SHOW_NUMBER_COL);
    Bind(wxEVT_MENU, &ReconcileDialog::OnMenuItemChecked, this, ID_CHECK_INCLUDE_VOID);
    Bind(wxEVT_MENU, &ReconcileDialog::OnMenuItemChecked, this, ID_CHECK_INCLUDE_DUPLICATED);

    // --- Main layout ---
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(topPanel,    0, wxEXPAND | wxALL, 10);
    mainSizer->Add(midPanel,    1, wxEXPAND | wxLEFT | wxRIGHT, 10);
    mainSizer->Add(resPanelOut, 0, wxEXPAND | wxALL, 10);
    mainSizer->Add(bottomPanel, 0, wxEXPAND | wxALL, 10);
    SetSizerAndFit(mainSizer);
}

void ReconcileDialog::FillControls(bool init)
{
    if (init) {
        double endval;
        wxString endvalue = InfoModel::instance().getString(
            wxString::Format("RECONCILE_ACCOUNT_%lld_END_BALANCE", m_account_n->m_id),
            "0.00"
        );
        if (!CurrencyModel::instance().fromString(endvalue, endval, m_currency_n)) {
            endval = 0;
        }
        w_amount_text->SetValue(endval);
    }

    // get not reconciled transactions
    wxSharedPtr<mmDateRange> date_range;
    date_range = new mmCurrentMonthToDate;
    TrxModel::DataA trx_a = TrxModel::instance().find_data_a(
        TrxModel::WHERE_DATE(OP_LE, mmDate::today()),
        TrxModel::WHERE_STATUS(OP_NE, TrxStatus(TrxStatus::e_reconciled)),
        TableClause::BEGIN_OR(),
            TrxCol::WHERE_ACCOUNTID(OP_EQ, m_account_n->m_id),
            TrxCol::WHERE_TOACCOUNTID(OP_EQ, m_account_n->m_id),
        TableClause::END(),
        TrxModel::WHERE_IS_DELETED(false),
        TableClause::ORDERBY(TrxCol::NAME_TRANSDATE)
    );

    long ritemIndex = -1;
    long litemIndex = -1;
    m_itemDataMap.clear();
    long mapidx = 0;

    wxListCtrl* list;
    w_left_list->DeleteAllItems();
    w_right_list->DeleteAllItems();
    long item;
    m_hiddenDuplicatedBalance = 0.0;
    for (const auto& trx_d : trx_a) {
        if (!m_settings[SETTING_INCLUDE_VOID] && trx_d.is_void()) {
            continue;
        }
        if (!m_settings[SETTING_INCLUDE_DUPLICATED] &&
            trx_d.m_status.id() == TrxStatus::e_duplicate
        ) {
            m_hiddenDuplicatedBalance += trx_d.m_amount;
            continue;
        }
        if (trx_d.is_deposit() ||
            (trx_d.is_transfer() && trx_d.m_to_account_id_n == m_account_n->m_id)
        ) {
            list = w_right_list;
            item = w_right_list->InsertItem(++ritemIndex, "");
        }
        else {
            list = w_left_list;
            item = w_left_list->InsertItem(++litemIndex, "");
        }
        setListItemData(&trx_d, list, item);
        m_itemDataMap.push_back(trx_d.m_id);
        list->SetItemData(item, mapidx++);
    }
}

void ReconcileDialog::UpdateAll()
{
    double clearedbalance = m_reconciledBalance;
    for (long i = 0; i < w_left_list->GetItemCount(); ++i) {
        if (isListItemChecked(w_left_list, i)) {
            wxString itext = w_left_list->GetItemText(i, 4);
            double value;
            if (CurrencyModel::instance().fromString(itext, value, m_currency_n)) {
               clearedbalance -= value;
            }
        }
    }
    for (long i = 0; i < w_right_list->GetItemCount(); ++i) {
        if (isListItemChecked(w_right_list, i)) {
            wxString itext = w_right_list->GetItemText(i, 4);
            double value;
            if (CurrencyModel::instance().fromString(itext, value, m_currency_n)) {
               clearedbalance += value;
            }
        }
    }

    double endbalance;
    if (!w_amount_text->GetDouble(endbalance)) {
        endbalance = 0.0;
    }

    w_previous_bal_text->SetLabel(CurrencyModel::instance().toCurrency(
        m_reconciledBalance,
        m_currency_n
    ));
    w_cleared_bal_text->SetLabel(CurrencyModel::instance().toCurrency(
        clearedbalance,
        m_currency_n
    ));
    w_ending_bal_text->SetLabel(CurrencyModel::instance().toCurrency(
        endbalance,
        m_currency_n
    ));
    w_ending_bal_text->SetMinSize(w_ending_bal_text->GetBestSize());
    w_ending_bal_text->GetParent()->Layout();

    double diff = clearedbalance - endbalance - m_hiddenDuplicatedBalance;

    w_diff_text->SetLabel(CurrencyModel::instance().toCurrency(diff, m_currency_n));

    wxFont font = w_diff_text->GetFont();
    int ps = w_previous_bal_text->GetFont().GetPointSize();
    if (diff > 0.005 || diff < -0.005) {
        font.SetWeight(wxFONTWEIGHT_BOLD);
        font.SetPointSize(ps +2);
    }
    else {
        font.SetWeight(wxFONTWEIGHT_NORMAL);
        font.SetPointSize(ps);
    }
    w_diff_label->SetFont(font);
    w_diff_text->SetFont(font);

    Refresh();
    Layout();
    Update();
}

void ReconcileDialog::OnListItemSelection(wxListEvent& WXUNUSED(event))
{
   updateButtonState();
}

void ReconcileDialog::updateButtonState()
{
    bool hasSelection = false;
    if (w_left_list->HasFocus() || w_right_list->HasFocus()) {
        hasSelection = (
            w_left_list->GetNextItem(-1,wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED) != -1 ||
            w_right_list->GetNextItem(-1,wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED) != -1
        );
    }
    w_edit_btn->Enable(hasSelection);
}

void ReconcileDialog::OnCalculator(wxCommandEvent& WXUNUSED(event))
{
    w_calc_popup->Popup();
}

void ReconcileDialog::OnAmountChanged(wxCommandEvent& WXUNUSED(event))
{
    UpdateAll();
}

void ReconcileDialog::OnLeftItemLeftClick(wxMouseEvent& event)
{
    processLeftClick(w_left_list, event.GetPosition());
    event.Skip();
}

void ReconcileDialog::OnRightItemLeftClick(wxMouseEvent& event)
{
    processLeftClick(w_right_list, event.GetPosition());
    event.Skip();
}

void ReconcileDialog::processLeftClick(wxListCtrl* list, wxPoint pt)
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

void ReconcileDialog::OnLeftItemRightClick(wxMouseEvent& event)
{
    wxPoint pt = event.GetPosition();
    int flags = 0;
    processRightClick(w_left_list, w_left_list->HitTest(pt, flags));
    event.Skip();
}

void ReconcileDialog::OnRightItemRightClick(wxMouseEvent& event)
{
    wxPoint pt = event.GetPosition();
    int flags = 0;
    processRightClick(w_right_list, w_right_list->HitTest(pt, flags));
    event.Skip();
}

void ReconcileDialog::processRightClick(wxListCtrl* list, long item)
{
    if (item != -1) {
       editTransaction(list, item);
    }
    else {
        newTransaction();
    }
    UpdateAll();
}

void ReconcileDialog::OnListKeyDown(wxKeyEvent& event)
{
    wxListCtrl* list = nullptr;
    if (w_left_list->GetSelectedItemCount() > 0) {
        list = w_left_list;
    }
    else if (w_right_list->GetSelectedItemCount() > 0) {
        list = w_right_list;
    }
    if (list) {
        switch(event.GetKeyCode()) {
            case WXK_LEFT:
            case WXK_RIGHT:
                list = list == w_left_list ? w_right_list : w_left_list;
                list->SetFocus();
                if (list->GetItemCount() > 0) {
                    long idx = list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_FOCUSED);
                    list->SetItemState(idx > -1 ? idx : 0,
                        wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED
                    );
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
                            list->SetItemState(idx + 1,
                                wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED
                            );
                        }
                        UpdateAll();
                    }
                }
                break;
        }
    }
    event.Skip();
}

void ReconcileDialog::OnLeftFocus(wxFocusEvent& event)
{
    handleListFocus(w_left_list);
    event.Skip();
}

void ReconcileDialog::OnRightFocus(wxFocusEvent& event)
{
    handleListFocus(w_right_list);
    event.Skip();
}

void ReconcileDialog::handleListFocus(wxListCtrl* list)
{
    if (list->GetItemCount() > 0) {
        long idx = list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_FOCUSED);
        list->SetItemState(idx > -1 ? idx : 0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    }
    updateButtonState();
}

void ReconcileDialog::OnLeftFocusKill(wxFocusEvent& event)
{
    handleListFocusKill(w_left_list);
    event.Skip();
}

void ReconcileDialog::OnRightFocusKill(wxFocusEvent& event)
{
    handleListFocusKill(w_right_list);
    event.Skip();
}

void ReconcileDialog::handleListFocusKill(wxListCtrl* list)
{
    resetListSelections(list);
}

void ReconcileDialog::OnNew(wxCommandEvent& WXUNUSED(event))
{
    newTransaction();
}

void ReconcileDialog::OnSettings(wxCommandEvent& WXUNUSED(event))
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

void ReconcileDialog::OnMenuSelected(wxCommandEvent& WXUNUSED(event))
{
    wxLogDebug("Menu selected");
}

void ReconcileDialog::OnMenuItemChecked(wxCommandEvent& event)
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

void ReconcileDialog::showHideColumn(bool show, int col, int cs) {
    if (!show) {
        m_colwidth[cs] = w_left_list->GetColumnWidth(col);
    }
    w_left_list->SetColumnWidth(col, show ? m_colwidth[cs] : 0);
    w_right_list->SetColumnWidth(col, show ? m_colwidth[cs] : 0);
}

void ReconcileDialog::newTransaction()
{
    int rc;
    TrxDialog dlg(this, JournalKey(), false, m_account_n->m_id, TrxType());
    do {
        dlg.SetReturnCode(0);  // necessary for wxWidgets >= 3.3.4 to show dialog again
        rc = dlg.ShowModal();
        if (rc != wxID_CANCEL) {
            w_journal->refreshList();
            int64 transid = dlg.trx_id();
            const TrxData* trx = TrxModel::instance().get_idN_data_n(transid);
            addTransaction2List(trx);
        }
    } while (rc == wxID_NEW);
}

void ReconcileDialog::addTransaction2List(const TrxData* trx_n)
{
    wxListCtrl* list = (trx_n->is_deposit() ||
        (trx_n->is_transfer() && trx_n->m_to_account_id_n == m_account_n->m_id)
    ) ? w_right_list : w_left_list;
    long idx = getListIndexByDate(trx_n, list);
    if (idx == -1) {
        idx = list->GetItemCount();
    }
    long item = list->InsertItem(idx, "");
    setListItemData(trx_n, list, item);
    list->SetItemData(item, m_itemDataMap.size());
    m_itemDataMap.push_back(trx_n->m_id);
}

void ReconcileDialog::OnEdit(wxCommandEvent& WXUNUSED(event))
{
    wxListCtrl* list = nullptr;
    if (w_left_list->GetSelectedItemCount() > 0) {
        list = w_left_list;
    }
    else if (w_right_list->GetSelectedItemCount() > 0) {
        list = w_right_list;
    }
    if (list) {
        long item = list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        editTransaction(list, item);
        UpdateAll();
    }
}

void ReconcileDialog::editTransaction(wxListCtrl* list, long item)
{
    int64 trx_id = m_itemDataMap[list->GetItemData(item)];
    TrxDialog dlg(this, JournalKey(-1, trx_id));
    if (dlg.ShowModal() == wxID_OK) {
        w_journal->refreshList();
        const TrxData* trx_n = TrxModel::instance().get_idN_data_n(trx_id);
        setListItemData(trx_n, list, item);
        long idx = getListIndexByDate(trx_n, list);
        if (idx != item) {
            moveItemData(list, item, idx);
        }
    }
}

long ReconcileDialog::getListIndexByDate(const TrxData* trx_n, wxListCtrl* list)
{
    long idx = -1;
    for (long i = 0; i < list->GetItemCount(); ++i) {
        int64 other_id = m_itemDataMap[list->GetItemData(i)];
        const TrxData* other_trx_n = TrxModel::instance().get_idN_data_n(other_id);
        if (trx_n->m_date() < other_trx_n->m_date()) {
            idx = i;
            break;
        }
    }
    return idx;
}

void ReconcileDialog::moveItemData(wxListCtrl* list, int row1, int row2)
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
    long item = list->InsertItem(
        row2 > row1
            ? (row2 > list->GetItemCount() ? list->GetItemCount() : row2 - 1)
            : row2,
        ""
    );

    for (int i = 0; i < list->GetColumnCount(); i++) {
        list->SetItem(item, i , coldata[i]);
    }
    list->SetItemData(item, rowData);
    list->SetItemImage(item, rowischecked ? 1 : 0);
    list->SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
}

void ReconcileDialog::setListItemData(const TrxData* trx_n, wxListCtrl* list, long item)
{
    wxString prefix = trx_n->is_transfer()
        ? (trx_n->m_to_account_id_n == m_account_n->m_id ? "< " : "> ")
        : "";
    wxString payeeName = (trx_n->is_transfer())
        ? AccountModel::instance().get_id_name(trx_n->m_to_account_id_n == m_account_n->m_id
            ? trx_n->m_account_id
            : trx_n->m_to_account_id_n
        )
        : PayeeModel::instance().get_id_name(trx_n->m_payee_id_n);
    list->SetItem(item, 1, mmGetDateTimeForDisplay(trx_n->m_isoDateTime()));
    list->SetItem(item, 2, trx_n->m_number);
    list->SetItem(item, 3, prefix + payeeName);
    list->SetItem(item, 4, CurrencyModel::instance().toString(trx_n->m_amount,m_currency_n));
    list->SetItem(item, 5, trx_n->m_status.key());
    list->SetItemImage(item, (trx_n->m_status.id() == TrxStatus::e_followup) ? 1 : 0);
}

void ReconcileDialog::OnToggle(wxCommandEvent& WXUNUSED(event))
{
    DoWindowsFreezeThaw(this);
    bool isChecked = true;
    for (long i = 0; i < w_left_list->GetItemCount(); ++i) {
        if (!isListItemChecked(w_left_list, i)) {
            isChecked = false;
            break;
        }
    }
    if (isChecked) {
        for (long i = 0; i < w_right_list->GetItemCount(); ++i) {
            if (!isListItemChecked(w_right_list, i)) {
                isChecked = false;
                break;
            }
        }
    }
    for (long i = 0; i < w_left_list->GetItemCount(); ++i) {
        w_left_list->SetItemImage(i, isChecked ? 0 : 1);
    }
    for (long i = 0; i < w_right_list->GetItemCount(); ++i) {
        w_right_list->SetItemImage(i, isChecked ? 0 : 1);
    }
    UpdateAll();
    resetListSelections(w_left_list);
    resetListSelections(w_right_list);
    DoWindowsFreezeThaw(this);
}

void ReconcileDialog::resetListSelections(wxListCtrl* list)
{
    if (list->GetSelectedItemCount() > 0) {
        long item = list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (item > -1) {
            list->SetItemState(item, 0, wxLIST_STATE_SELECTED);
        }
    }
}

bool ReconcileDialog::isListItemChecked(wxListCtrl* list, long item)
{
    bool checked = false;
    wxListItem litem;
    litem.SetId(item);
    if (list->GetItem(litem)) {
        checked = litem.GetImage() == 1;
    }
    return checked;
}

void ReconcileDialog::OnSize(wxSizeEvent& event)
{
    resizeColumns();
    event.Skip();
}

void ReconcileDialog::resizeColumns()
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

    setColWidth(w_left_list);
    setColWidth(w_right_list);
}

void ReconcileDialog::applyColumnSettings()
{
    m_colwidth[0] = w_left_list->GetColumnWidth(2);
    m_colwidth[1] = w_left_list->GetColumnWidth(5);

    m_settings[SETTING_SHOW_STATE_COL] = InfoModel::instance().getBool(
        "RECONCILE_DIALOG_SHOW_STATE_COL", true
    );
    if (!m_settings[SETTING_SHOW_STATE_COL]) {
        showHideColumn(false, 5, 1);
    }

    m_settings[SETTING_SHOW_NUMBER_COL] = InfoModel::instance().getBool(
        "RECONCILE_DIALOG_SHOW_NUMBER_COL", true
    );
    if (!m_settings[SETTING_SHOW_NUMBER_COL]) {
        showHideColumn(false, 2, 0);
    }
    resizeColumns();
}

void ReconcileDialog::OnClose(wxCommandEvent& event)
{
    auto saveItem = [](int64 id, bool state, bool final) {
        TrxData* trx_n = TrxModel::instance().unsafe_get_idN_data_n(id);
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
        InfoModel::instance().saveString(
            wxString::Format("RECONCILE_ACCOUNT_%lld_END_BALANCE", m_account_n->m_id),
            w_ending_bal_text->GetLabelText()
        );

        // Save state:
        for (long i = 0; i < w_left_list->GetItemCount(); ++i) {
            saveItem(
                m_itemDataMap[w_left_list->GetItemData(i)],
                isListItemChecked(w_left_list, i),
                event.GetId() == wxID_OK
            );
        }
        for (long i = 0; i < w_right_list->GetItemCount(); ++i) {
            saveItem(
                m_itemDataMap[w_right_list->GetItemData(i)],
                isListItemChecked(w_right_list, i),
                event.GetId() == wxID_OK
            );
        }
    }

    EndModal(wxID_OK);
}
