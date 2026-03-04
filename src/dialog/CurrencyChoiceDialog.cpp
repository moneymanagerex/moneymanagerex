/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2015 Gabriele-V
 Copyright (C) 2016, 2021 Nikolay

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

#include "base/defs.h"
#include <vector>
#include <wx/sstream.h>
#include <wx/xml/xml.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>

#include "base/constants.h"
#include "base/paths.h"
#include "base/images_list.h"
#include "util/_util.h"
#include "util/_simple.h"
#include "util/mmTextCtrl.h"
#include "util/mmCalcValidator.h"

#include "model/_all.h"

#include "manager/CurrencyManager.h"
#include "CurrencyChoiceDialog.h"

using namespace rapidjson;

wxIMPLEMENT_DYNAMIC_CLASS(CurrencyChoiceDialog, wxDialog);

wxBEGIN_EVENT_TABLE(CurrencyChoiceDialog, wxDialog)
    EVT_BUTTON(wxID_SELECTALL, CurrencyChoiceDialog::OnBtnSelect)
    EVT_BUTTON(wxID_CANCEL, CurrencyChoiceDialog::OnCancel)
    EVT_TEXT(wxID_ANY, CurrencyChoiceDialog::OnTextChanged)
    EVT_MENU(wxID_ANY, CurrencyChoiceDialog::OnMenuSelected)

    EVT_DATAVIEW_ITEM_ACTIVATED(wxID_ANY, CurrencyChoiceDialog::OnListItemActivated)
    EVT_DATAVIEW_SELECTION_CHANGED(wxID_ANY, CurrencyChoiceDialog::OnListItemSelected)
    EVT_DATAVIEW_ITEM_CONTEXT_MENU(wxID_ANY, CurrencyChoiceDialog::OnItemRightClick)

    EVT_BUTTON(HISTORY_ADD, CurrencyChoiceDialog::OnHistoryAdd)
    EVT_BUTTON(HISTORY_DELETE, CurrencyChoiceDialog::OnHistoryDelete)
    EVT_BUTTON(HISTORY_UPDATE, CurrencyChoiceDialog::OnHistoryUpdate)
    EVT_BUTTON(HISTORY_DELUNUSED, CurrencyChoiceDialog::OnHistoryDeleteUnused)
    EVT_LIST_ITEM_SELECTED(wxID_ANY, CurrencyChoiceDialog::OnHistorySelected)
    EVT_LIST_ITEM_DESELECTED(wxID_ANY, CurrencyChoiceDialog::OnHistoryDeselected)
    wxEND_EVENT_TABLE()

CurrencyChoiceDialog::~CurrencyChoiceDialog()
{
    InfoModel::instance().setSize("CURRENCY_DIALOG_SIZE", GetSize());
}

CurrencyChoiceDialog::CurrencyChoiceDialog(
    wxWindow* parent,
    int64 currencyID,
    bool bEnableSelect
) :
    m_select_en(bEnableSelect),
    m_maskStr("")
{
    m_history_en = PrefModel::instance().getUseCurrencyHistory();

    m_col_name_m[CURR_BASE]   = " ";
    m_col_name_m[CURR_SYMBOL] = _t("Code");
    m_col_name_m[CURR_NAME]   = _t("Name");
    m_col_name_m[BASE_RATE]   = m_history_en ? _t("Last Rate") : _t("Fixed Rate");

    m_currency_id = currencyID == -1 ? PrefModel::instance().getBaseCurrencyID() : currencyID;
    this->SetFont(parent->GetFont());
    Create(parent);
    m_select_en ? SetMinSize(wxSize(200, 350)) : SetMinSize(wxSize(500, 350));
    mmSetSize(this);
    Centre();
    mmThemeAutoColour(this);
}

bool CurrencyChoiceDialog::Create(wxWindow* parent
    , wxWindowID id
    , const wxString& caption
    , const wxString& name
    , const wxPoint& pos
    , const wxSize& size
    , long style)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, wxGetTranslation(caption), pos, size, style, name);

    wxAcceleratorEntry entries[1];
    entries[0].Set(wxACCEL_NORMAL, WXK_F5, wxID_EXECUTE);
    wxAcceleratorTable accel(1, entries);
    SetAcceleratorTable(accel);

    CreateControls();
    SetIcon(mmex::getProgramIcon());
    fillControls();
    Centre();

    return true;
}

void CurrencyChoiceDialog::fillControls()
{
    int selected_index = w_currency_list->GetSelectedRow();
    w_currency_list->DeleteAllItems();

    w_show_all_cb->SetValue(InfoModel::instance().getBool("SHOW_HIDDEN_CURRENCIES", true));

    int64 base_currency_id = -1;
    if (CurrencyModel::GetBaseCurrency()) {
        base_currency_id = CurrencyModel::GetBaseCurrency()->m_id;
    }

    bool skip_unused = !w_show_all_cb->IsChecked();
    for (const auto& currency : CurrencyModel::instance().find_all(
        CurrencyCol::COL_ID_CURRENCYNAME
    )) {
        int64 currencyID = currency.m_id;

        if (skip_unused &&
            !(CurrencyModel::is_used(currency.m_id) || currencyID == base_currency_id)
        )
            continue;
        if (!m_maskStr.IsEmpty()) {
            if (!currency.m_name.Lower().Matches(m_maskStr) && !currency.m_symbol.Lower().Matches(m_maskStr))
                continue;
        }
        wxString amount;
        if (-1 == base_currency_id) // Not yet set
            amount = _t("N/A");
        else
            amount = m_history_en
            ? CurrencyModel::toString(CurrencyHistoryModel::getLastRate(currencyID), nullptr, 4)
            : CurrencyModel::toString(currency.m_base_conv_rate, nullptr, 4);
        wxVector<wxVariant> data;
        data.push_back(wxVariant(base_currency_id == currencyID ? L"\u2713" : L""));
        data.push_back(wxVariant(currency.m_symbol));
        data.push_back(wxVariant(currency.m_name));
        data.push_back(wxVariant(amount));
        w_currency_list->AppendItem(data, static_cast<wxUIntPtr>(currencyID.GetValue()));
        if (selected_index == w_currency_list->GetItemCount() - 1) {
            w_currency_list->SelectRow(selected_index);
            m_currency_id = currencyID;
        }
        if (m_currency_id == currencyID) {
            selected_index = w_currency_list->GetItemCount() - 1;
            w_currency_list->SelectRow(selected_index);
        }
    }

    //Ensure that the selected item is visible.
    wxDataViewItem item(w_currency_list->GetCurrentItem());
    w_currency_list->EnsureVisible(item);
}

void CurrencyChoiceDialog::CreateControls()
{
    wxBoxSizer* mainBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    this->SetSizer(mainBoxSizer);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    mainBoxSizer->Add(itemBoxSizer2, g_flagsExpand);

    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer22, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxBitmapButton* update_button = new wxBitmapButton(this, wxID_EXECUTE, mmBitmapBundle(png::CURRATES, mmBitmapButtonSize));
    itemBoxSizer22->Add(update_button, g_flagsH);
    update_button->Connect(wxID_EXECUTE, wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(CurrencyChoiceDialog::OnOnlineUpdateCurRate), nullptr, this);
    mmToolTip(update_button, _t("Online update currency rate"));
    itemBoxSizer22->AddSpacer(4);

    itemBoxSizer22->Add(new wxStaticText(this, wxID_STATIC, _t("Online Update")), g_flagsH);

    itemBoxSizer22->AddSpacer(15);
    w_show_all_cb = new wxCheckBox(this, wxID_SELECTALL, _t("&Show All"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    mmToolTip(w_show_all_cb, _t("Show all even the unused currencies"));
    w_show_all_cb->Connect(wxID_SELECTALL, wxEVT_COMMAND_CHECKBOX_CLICKED
        , wxCommandEventHandler(CurrencyChoiceDialog::OnShowHiddenChbClick), nullptr, this);

    itemBoxSizer22->Add(w_show_all_cb, g_flagsH);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, g_flagsExpand);

    //TODO:provide proper style and fix validator (does not working)
    w_currency_list = new wxDataViewListCtrl(this, wxID_ANY
        , wxDefaultPosition, wxDefaultSize /*, wxDV_HORIZ_RULES, mmDoubleValidator(4)*/);

    w_currency_list->AppendTextColumn(
        m_col_name_m[CURR_BASE], wxDATAVIEW_CELL_INERT,
        30
    );
    w_currency_list->AppendTextColumn(
        m_col_name_m[CURR_SYMBOL], wxDATAVIEW_CELL_INERT,
        wxLIST_AUTOSIZE_USEHEADER, wxALIGN_LEFT, wxDATAVIEW_COL_SORTABLE
    );
    w_currency_list->AppendTextColumn(
        m_col_name_m[CURR_NAME], wxDATAVIEW_CELL_INERT,
        wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT,
        wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_RESIZABLE
    );
    w_currency_list->AppendTextColumn(
        m_col_name_m[BASE_RATE], wxDATAVIEW_CELL_EDITABLE,
        wxLIST_AUTOSIZE_USEHEADER
    );

    itemBoxSizer3->Add(w_currency_list, g_flagsExpand);

    wxBoxSizer* itemBoxSizerS = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizerS, wxSizerFlags(g_flagsExpand).Proportion(0));
    itemBoxSizerS->Add(new wxStaticText(this, wxID_STATIC, _t("Search")), g_flagsH);
    w_mask_text = new wxSearchCtrl(this, wxID_FIND);
    w_mask_text->SetFocus();
    itemBoxSizerS->Add(w_mask_text, g_flagsExpand);

    wxPanel* buttonsPanel = new wxPanel(this, wxID_ANY);
    itemBoxSizer2->Add(buttonsPanel, wxSizerFlags(g_flagsV).Center());

    wxBoxSizer* buttonsSizer = new wxBoxSizer(wxVERTICAL);
    buttonsPanel->SetSizer(buttonsSizer);
    wxStdDialogButtonSizer* itemBoxSizer66 = new wxStdDialogButtonSizer;
    buttonsSizer->Add(itemBoxSizer66);

    wxStdDialogButtonSizer* itemBoxSizer9 = new wxStdDialogButtonSizer;
    buttonsSizer->Add(itemBoxSizer9, wxSizerFlags(g_flagsExpand).Border(wxALL, 0));

    w_select_btn = new wxButton(buttonsPanel, wxID_SELECTALL, _t("&Select"));
    itemBoxSizer9->Add(w_select_btn, wxSizerFlags(g_flagsExpand).Proportion(4));
    //mmToolTip(itemButtonSelect, _t("Select the currently selected currency as the selected currency for the account"));

    if (!m_select_en)
        w_select_btn->Disable();

    //Some interfaces has no any close buttons, it may confuse user. Cancel button added
    wxButton* itemCancelButton = new wxButton(buttonsPanel, wxID_CANCEL, wxGetTranslation(g_CloseLabel));
    itemBoxSizer9->Add(itemCancelButton, g_flagsH);
    itemCancelButton->SetFocus();

    //History Panel -----------------------------------------------------
    wxBoxSizer* rightBoxSizer = new wxBoxSizer(wxVERTICAL);
    mainBoxSizer->Add(rightBoxSizer, g_flagsExpand);

    w_hostory_box = new wxStaticBox(this, wxID_ANY, _t("Historical Currency Options"));
    if (m_select_en)
        w_hostory_box->Hide();

    wxStaticBoxSizer* historyStaticBox_Sizer = new wxStaticBoxSizer(w_hostory_box, wxVERTICAL);
    rightBoxSizer->Add(historyStaticBox_Sizer, g_flagsExpand);

    w_history_list = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
    historyStaticBox_Sizer->Add(w_history_list, g_flagsExpand);

    wxListItem col0, col1, col2;
    // Add first column
    col0.SetId(0);
    col0.SetText(_t("Date"));
    w_history_list->InsertColumn(0, col0);

    // Add second column
    col1.SetId(1);
    col1.SetText(_t("Price"));
    w_history_list->InsertColumn(1, col1);

    // Add third column
    col2.SetId(2);
    col2.SetText(_t("Type"));
    w_history_list->InsertColumn(2, col2);

    //History Buttons
    wxBoxSizer* itemBoxSizerD = new wxBoxSizer(wxHORIZONTAL);
    historyStaticBox_Sizer->Add(itemBoxSizerD, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxStaticText* datePickerLabel = new wxStaticText(this, wxID_STATIC, _t("Date"));
    itemBoxSizerD->Add(datePickerLabel, g_flagsH);

    w_date_picker = new mmDatePickerCtrl(this, wxID_ANY, wxDefaultDateTime
        , wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN | wxDP_SHOWCENTURY);
    w_date_picker->SetMinSize(wxSize(120, -1));
    itemBoxSizerD->Add(w_date_picker, g_flagsExpand);
    mmToolTip(w_date_picker, _t("Specify the date of currency value"));
    w_date_picker->Disable();

    wxStaticText* textBoxLabel = new wxStaticText(this, wxID_STATIC, _t("Value"));
    itemBoxSizerD->Add(textBoxLabel, g_flagsH);

    w_value_text = new mmTextCtrl(this, wxID_ANY, wxGetEmptyString()
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    w_value_text->SetAltPrecision(6);
    w_value_text->SetMinSize(wxSize(120, -1));
    mmToolTip(w_value_text, _t("Enter the currency value"));
    itemBoxSizerD->Add(w_value_text, g_flagsExpand);
    w_value_text->Disable();

    wxPanel* buttons_panel = new wxPanel(this, wxID_ANY);
    historyStaticBox_Sizer->Add(buttons_panel, wxSizerFlags(g_flagsV).Centre());
    wxStdDialogButtonSizer*  buttons_sizer = new wxStdDialogButtonSizer;
    buttons_panel->SetSizer(buttons_sizer);

    w_download_btn = new wxBitmapButton(buttons_panel, HISTORY_UPDATE, mmBitmapBundle(png::CURRATES, mmBitmapButtonSize));
    mmToolTip(w_download_btn, _t("Download Currency Values history"));
    w_download_btn->Disable();

    w_add_btn = new wxButton(buttons_panel, HISTORY_ADD, _t("Add/&Update "), wxDefaultPosition, wxSize(-1, w_download_btn->GetSize().GetY()));
    mmToolTip(w_add_btn, _t("Add Currency Values to history"));
    w_add_btn->Disable();

    w_delete_btn = new wxButton(buttons_panel, HISTORY_DELETE, _t("&Delete "), wxDefaultPosition, wxSize(-1, w_download_btn->GetSize().GetY()));
    mmToolTip(w_delete_btn, _t("Delete selected Currency Values"));
    w_delete_btn->Disable();

    w_delete_unused_btn = new wxBitmapButton(buttons_panel, HISTORY_DELUNUSED, mmBitmapBundle(png::VOID_STAT, mmBitmapButtonSize));
    mmToolTip(w_delete_unused_btn, _t("Delete Historical Currency Values for unused currencies and days"));
    w_delete_unused_btn->Disable();

    buttons_sizer->Add(w_download_btn, g_flagsH);
    buttons_sizer->Add(w_add_btn, g_flagsH);
    buttons_sizer->Add(w_delete_btn, g_flagsH);
    buttons_sizer->Add(w_delete_unused_btn, g_flagsH);

    if (m_select_en || !m_history_en) {
        w_hostory_box->Hide();
        w_history_list->Hide();
        buttons_panel->Hide();
        w_date_picker->Hide();
        w_date_picker->Hide();
        w_value_text->Hide();
        w_download_btn->Hide();
        w_add_btn->Hide();
        w_delete_btn->Hide();
        w_delete_unused_btn->Hide();
        datePickerLabel->Hide();
        textBoxLabel->Hide();
    }
}

void CurrencyChoiceDialog::OnBtnAdd()
{
    CurrencyManager dlg(this, 0);
    if (dlg.ShowModal() == wxID_OK)
    {
        m_currency_id = dlg.getCurrencyID();
    }

    fillControls();
}

void CurrencyChoiceDialog::OnBtnEdit()
{
    const CurrencyData *data_n = CurrencyModel::instance().get_id_data_n(m_currency_id);
    if (data_n)
        CurrencyManager(this, data_n).ShowModal();
    fillControls();
}

void CurrencyChoiceDialog::OnBtnSelect(wxCommandEvent& /*event*/)
{
    if (m_currency_id > -1)
        EndModal(wxID_OK);
}

void CurrencyChoiceDialog::OnBtnDelete()
{
    int selected_index = w_currency_list->GetSelectedRow();
    if (selected_index < 0) return;

    const CurrencyData* data_n = CurrencyModel::instance().get_id_data_n(m_currency_id);
    if (!data_n) return;
    if (wxMessageBox(_t("Do you want to delete the selected currency?")
        , _t("Currency Manager")
        , wxYES_NO | wxNO_DEFAULT | wxICON_ERROR) == wxYES)
    {
        CurrencyModel::instance().purge_id(m_currency_id);
        m_currency_id = -1;
        fillControls();
    }
}

bool CurrencyChoiceDialog::Execute(wxWindow* parent, int64& currencyID)
{
    bool result = false;

    CurrencyChoiceDialog dlg(parent, currencyID);
    if (dlg.ShowModal() == wxID_OK)
    {
        currencyID = dlg.m_currency_id;
        result = true;
    }
    dlg.Destroy();

    return result;
}

bool CurrencyChoiceDialog::Execute(int64& currencyID)
{
    CurrencyChoiceDialog dlg(nullptr, currencyID);
    dlg.m_static_dialog = true;
    dlg.SetTitle(_t("Base Currency Selection"));
    dlg.w_history_list->Enable(false);
    dlg.w_value_text->Enable(false);
    dlg.w_date_picker->Enable(false);
    dlg.w_add_btn->Enable(false);
    dlg.w_delete_btn->Enable(false);

    bool ok = dlg.ShowModal() == wxID_OK;
    currencyID = dlg.m_currency_id;
    dlg.Destroy();

    return ok;
}

void CurrencyChoiceDialog::OnListItemSelected(wxDataViewEvent& event)
{
    bool is_selected = w_currency_list->GetSelectedRow() > -1;
    w_select_btn->Enable(is_selected && m_select_en);

    if (is_selected) {
        wxDataViewItem item = event.GetItem();
        m_currency_id = static_cast<int64>(w_currency_list->GetItemData(item));
        const CurrencyData* data_n = CurrencyModel::instance().get_id_data_n(m_currency_id);
        if (data_n) {

            // prevent user deleting currencies when editing accounts.
            if (!m_select_en) {
                int64 baseCurrencyID = PrefModel::instance().getBaseCurrencyID();
                if (m_history_en) {
                    w_download_btn->Enable(m_currency_id != baseCurrencyID);
                    w_delete_unused_btn->Enable(m_currency_id != baseCurrencyID);
                    w_date_picker->Enable(m_currency_id != baseCurrencyID);
                    w_value_text->Enable(m_currency_id != baseCurrencyID);
                    w_value_text->SetValue(0, CurrencyModel::instance().get_id_data_n(m_currency_id), 6);
                }
                else if (CurrencyHistoryModel::instance().find(
                    CurrencyHistoryCol::CURRENCYID(m_currency_id)
                ).size() > 0) {
                    if (wxMessageBox(
                        wxString::Format(
                            _t("Historical rates for %1$s found, but “Use historical currency” in options is disabled:\n"
                                "click No and enable it or click Yes to remove all historical rates for %2$s"
                            ),
                            data_n->m_symbol,
                            data_n->m_symbol
                        ),
                        _t("Currency Manager"),
                        wxYES_NO | wxNO_DEFAULT | wxICON_WARNING
                    ) == wxYES) {
                        CurrencyHistoryModel::instance().db_savepoint();
                        for (const auto& r : CurrencyHistoryModel::instance().find(
                            CurrencyHistoryCol::CURRENCYID(m_currency_id)
                        )) {
                            CurrencyHistoryModel::instance().purge_id(r.id());
                        }
                        CurrencyHistoryModel::instance().db_release_savepoint();
                    }
                }
            }
            w_hostory_box->SetLabel(wxString::Format(
                _t("Currency History Options: %s"),
                data_n->m_name
            ));
        }
    }

    w_date_picker->SetValue(wxDateTime::Today());
    w_value_text->SetValue(wxEmptyString);
    ShowCurrencyHistory();
}

void CurrencyChoiceDialog::OnListItemActivated(wxDataViewEvent& /* event */)
{
    wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, wxID_ANY);

    if (m_select_en)
        OnBtnSelect(evt);
    else
        OnBtnEdit();
}

bool CurrencyChoiceDialog::OnlineUpdateCurRate(int64 curr_id, bool hide)
{
    wxString msg = wxEmptyString;
    bool ok = getOnlineCurrencyRates(msg, curr_id, w_show_all_cb->IsChecked());
    if (ok)
    {
        if (!hide)
        {
            wxMessageDialog msgDlg(this, msg, _t("Online update currency rate"));
            msgDlg.ShowModal();
        }
        fillControls();
        ShowCurrencyHistory();
    }
    else
    {
        wxMessageDialog msgDlg(this, msg, _t("Error"), wxOK | wxICON_ERROR);
        msgDlg.ShowModal();
    }
    return ok;
}

void CurrencyChoiceDialog::OnOnlineUpdateCurRate(wxCommandEvent& /*event*/)
{
    if (!m_static_dialog)    //Abort when trying to set base currency
    {
        OnlineUpdateCurRate(-1, false);
    }
}

void CurrencyChoiceDialog::OnMenuSelected(wxCommandEvent& event)
{
    switch (event.GetId())
    {
        case MENU_ITEM1:
            if (!SetBaseCurrency(m_currency_id))
                mmErrorDialogs::MessageError(this
                    , _t("Unable to update historical currency rates. Please update them manually!")
                    , _t("Historical currency error"));

            fillControls();
            ShowCurrencyHistory();
            break;
        case MENU_ITEM2:
            OnlineUpdateCurRate(m_currency_id, false);
            break;
        case wxID_EDIT:
            this->OnBtnEdit();
            break;
        case wxID_ADD:
            OnBtnAdd();
            break;
        case wxID_REMOVE:
            OnBtnDelete();
            break;
        default:
            break;
    }
}

void CurrencyChoiceDialog::OnItemRightClick(wxDataViewEvent& event)
{
    // disable this function when setting base currency
    if (m_static_dialog) return;

    wxCommandEvent ev(wxEVT_COMMAND_MENU_SELECTED, wxID_ANY);
    ev.SetEventObject(this);

    wxMenu* mainMenu = new wxMenu;

    mainMenu->Append(new wxMenuItem(mainMenu, MENU_ITEM1, _t("Set as Base Currency")));
    mainMenu->Append(new wxMenuItem(mainMenu, MENU_ITEM2, _t("Online Update Currency Rate")));
    mainMenu->AppendSeparator();
    mainMenu->Append(new wxMenuItem(mainMenu, wxID_ADD, _t("&Add ")));
    mainMenu->Append(new wxMenuItem(mainMenu, wxID_EDIT, _t("&Edit ")));
    mainMenu->Append(new wxMenuItem(mainMenu, wxID_REMOVE, _t("&Remove ")));

    bool is_selected = w_currency_list->GetSelectedRow() > -1;
    w_select_btn->Enable(is_selected);
    int64 baseCurrencyID = PrefModel::instance().getBaseCurrencyID();
    mainMenu->Enable(MENU_ITEM1, baseCurrencyID != m_currency_id && is_selected);
    mainMenu->Enable(MENU_ITEM2, baseCurrencyID != m_currency_id && is_selected);

    const CurrencyData* currency_n = CurrencyModel::instance().get_id_data_n(m_currency_id);
    mainMenu->Enable(wxID_REMOVE, !CurrencyModel::is_used(currency_n->m_id) && is_selected);

    mainMenu->Enable(wxID_EDIT, is_selected);

    PopupMenu(mainMenu);
    delete mainMenu;
    event.Skip();
}

void CurrencyChoiceDialog::OnShowHiddenChbClick(wxCommandEvent& WXUNUSED(event))
{
    InfoModel::instance().setBool("SHOW_HIDDEN_CURRENCIES", w_show_all_cb->IsChecked());
    fillControls();
}

void CurrencyChoiceDialog::OnCancel(wxCommandEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}

void CurrencyChoiceDialog::ShowCurrencyHistory()
{
    //Abort when trying to set base currency
    if (m_static_dialog)
        return;
    w_history_list->DeleteAllItems();

    int64 base_currency_id = PrefModel::instance().getBaseCurrencyID();
    if (!m_history_en || m_currency_id <= 0 || m_currency_id == base_currency_id) {
        w_add_btn->Disable();
        w_delete_btn->Disable();
        return;
    }
    else {
        w_add_btn->Enable();
        w_delete_btn->Enable();
    }

    const CurrencyData* currency = CurrencyModel::instance().get_id_data_n(m_currency_id);
    CurrencyHistoryModel::DataA histData = CurrencyHistoryModel::instance().find(
        CurrencyHistoryCol::CURRENCYID(m_currency_id)
    );
    std::stable_sort(histData.begin(), histData.end(), CurrencyHistoryData::SorterByCURRDATE());
    std::reverse(histData.begin(), histData.end());
    if (!histData.empty()) {
        int idx = 0;
        for (const auto& ch_d : histData) {
            wxListItem item;
            item.SetId(idx);
            item.SetData(reinterpret_cast<void*>(ch_d.m_id.GetValue()));
            w_history_list->InsertItem(item);
            const wxString dispAmount = CurrencyModel::toString(ch_d.m_base_conv_rate, currency, 6);
            w_history_list->SetItem(idx, 0, mmGetDateTimeForDisplay(ch_d.m_date.isoDate()));
            w_history_list->SetItem(idx, 1, dispAmount);


            const wxString& priceAmount = "* M"[ch_d.m_update_type.id()];
            w_history_list->SetItem(idx, 2, priceAmount);
            idx++;
        }
        w_history_list->RefreshItems(0, --idx);
        w_history_list->SetColumnWidth(0, wxLIST_AUTOSIZE);
        w_history_list->SetColumnWidth(1, wxLIST_AUTOSIZE);
    }
}

void CurrencyChoiceDialog::OnHistoryAdd(wxCommandEvent& /*event*/)
{
    if (m_currency_id <= 0) return;

    double dPrice = 0.0;
    wxString currentPriceStr = w_value_text->GetValue().Trim();
    if (!CurrencyModel::fromString(
        currentPriceStr, dPrice,
        CurrencyModel::instance().get_id_data_n(m_currency_id)
    ) || dPrice < 0.0)
        return mmErrorDialogs::ToolTip4Object(w_value_text, _t("Invalid Entry"), _t("Amount"));
    CurrencyHistoryModel::instance().addUpdate(m_currency_id, w_date_picker->GetValue(), dPrice, CurrencyHistoryModel::MANUAL);

    fillControls();
    ShowCurrencyHistory();
}

void CurrencyChoiceDialog::OnHistoryDelete(wxCommandEvent& WXUNUSED(event))
{
    if (w_history_list->GetSelectedItemCount() <= 0) return;

    long item = -1;
    CurrencyHistoryModel::instance().db_savepoint();
    for (;;)
    {
        item = w_history_list->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (item == -1) break;
        CurrencyHistoryModel::instance().purge_id(static_cast<int64>(w_history_list->GetItemData(item)));
    }
    CurrencyHistoryModel::instance().db_release_savepoint();

    fillControls();
    ShowCurrencyHistory();
}

void CurrencyChoiceDialog::OnHistoryUpdate(wxCommandEvent& WXUNUSED(event))
{
    //Abort when trying to set base currency
    if (m_static_dialog)
        return;
    const CurrencyData* currency_n = CurrencyModel::instance().get_id_data_n(m_currency_id);
    if (!currency_n) {
        return mmErrorDialogs::MessageError(this,
            _t("No currency selected!"),
            _t("Currency history error")
        );
    }

    wxString base_currency_symbol;
    wxASSERT_MSG(
        CurrencyModel::GetBaseCurrencySymbol(base_currency_symbol),
        "Unable to find base currency symbol"
    );

    int msgResult = wxMessageBox(
        _t("Do you want to add dates without any details?"),
        _t("Currency Manager"),
        wxYES_NO | wxNO_DEFAULT | wxICON_WARNING
    );
    bool isCheckDate = (msgResult == wxNO);

    wxString msg;
    const std::map<wxDateTime, int> DatesList = CurrencyModel::DateUsed(m_currency_id);
    wxDateTime begin_date = wxDateTime::Now().Subtract(wxDateSpan::Years(1));
    if (isCheckDate && !DatesList.empty()) {
        begin_date = DatesList.begin()->first;
    }
    wxLogDebug("Begin Date: %s", begin_date.FormatISODate());

    std::map<wxDateTime, double> historical_rates;
    bool isUpdStatus = GetOnlineHistory(
        currency_n->m_symbol, begin_date, historical_rates, msg
    );

    if (!isUpdStatus && !g_fiat_curr().Contains(currency_n->m_symbol)) {
        wxString coincap_id;
        double coincap_price_usd;
        isUpdStatus = getCoincapInfoFromSymbol(
            currency_n->m_symbol, coincap_id, coincap_price_usd, msg
        );
        if (isUpdStatus)
            isUpdStatus = getCoincapAssetHistory(coincap_id, begin_date, historical_rates, msg);
    }

    if (!isUpdStatus) {
        return mmErrorDialogs::MessageError(this,
            wxString::Format(_t("Unable to download %s currency rates"),
                currency_n->m_symbol
            ),
            _t("Historical currency error")
        );
    }

    if (historical_rates.empty()) {
        mmErrorDialogs::MessageError(this,
            wxString::Format(_t("Unable to download history for symbol %s. Historical rates unavailable!"),
                currency_n->m_symbol
            ),
            _t("Historical currency error")
        );
        return;
    }

    CurrencyHistoryModel::instance().db_savepoint();
    if (isCheckDate) {
        for (const auto entry : DatesList) {
            const wxDate date = entry.first;
            wxLogDebug("%s %s", date.FormatISODate(), date.FormatISOTime());
            if (historical_rates.find(date) == historical_rates.end())
                continue;
            wxLogDebug("%s", date.FormatISODate());
            CurrencyHistoryModel::instance().addUpdate(
                m_currency_id, date, historical_rates[date], CurrencyHistoryModel::ONLINE
            );
        }
    }
    else {
        for (auto &entry : historical_rates) {
            CurrencyHistoryModel::instance().addUpdate(
                m_currency_id, entry.first, entry.second, CurrencyHistoryModel::ONLINE
            );
        }
    }
    CurrencyHistoryModel::instance().db_release_savepoint();

    fillControls();
    ShowCurrencyHistory();
}

void CurrencyChoiceDialog::OnHistoryDeleteUnused(wxCommandEvent& WXUNUSED(event))
{
    //Abort when trying to set base currency
    if (m_static_dialog)
        return;
    CurrencyHistoryModel::instance().db_savepoint();
    auto currency_a = CurrencyModel::instance().find_all();
    for (const auto& currency_d : currency_a) {
        if (!CurrencyModel::is_used(currency_d.m_id)) {
            for (const auto& ch_d : CurrencyHistoryModel::instance().find(
                CurrencyHistoryCol::CURRENCYID(currency_d.m_id)
            ))
                CurrencyHistoryModel::instance().purge_id(ch_d.id());
        }
        else {
            std::map<wxDateTime, int> date_used_m = CurrencyModel::DateUsed(currency_d.m_id);
            for (const auto& ch_d : CurrencyHistoryModel::instance().find(
                CurrencyHistoryCol::CURRENCYID(currency_d.m_id)
            )) {
                wxDateTime date = ch_d.m_date.getDateTime();
                if (date_used_m.find(date) == date_used_m.end())
                    CurrencyHistoryModel::instance().purge_id(ch_d.id());
            }
        }
    }
    CurrencyHistoryModel::instance().db_release_savepoint();

    fillControls();
    ShowCurrencyHistory();
}

void CurrencyChoiceDialog::OnHistorySelected(wxListEvent& event)
{
    long selectedIndex = event.GetIndex();
    int64 histId = w_history_list->GetItemData(selectedIndex);
    const CurrencyHistoryData* ch_n = CurrencyHistoryModel::instance().get_id_data_n(histId);

    if (ch_n->m_id > 0) {
        w_date_picker->SetValue(CurrencyHistoryModel::CURRDATE(*ch_n));
        w_value_text->SetValue(wxString::Format("%f", ch_n->m_base_conv_rate));
    }
}

void CurrencyChoiceDialog::OnHistoryDeselected(wxListEvent& WXUNUSED(event))
{
    w_date_picker->SetValue(wxDateTime::Today());
    w_value_text->SetValue(wxEmptyString);
}

bool CurrencyChoiceDialog::SetBaseCurrency(int64& baseCurrencyID)
{
    int64 baseCurrencyOLD = PrefModel::instance().getBaseCurrencyID();
    if (baseCurrencyID == baseCurrencyOLD)
        return true;

    if (m_history_en && wxMessageBox(
            _t("Changing base currency will delete all historical rates, proceed?"),
            _t("Currency Manager"),
            wxYES_NO | wxYES_DEFAULT | wxICON_WARNING
    ) != wxYES) {
        return true;
    }

    PrefModel::instance().setBaseCurrencyID(baseCurrencyID);

    // Update baseconvrate
    CurrencyModel::instance().db_savepoint();
    auto currency_a = CurrencyModel::instance().find_all();
    for (auto& currency_d : currency_a) {
        currency_d.m_base_conv_rate = 1;
        CurrencyModel::instance().save_data_n(currency_d);
    }
    CurrencyModel::instance().db_release_savepoint();

    // Delete historical currency
    CurrencyHistoryModel::instance().db_savepoint();
    for (const auto& r : CurrencyHistoryModel::instance().find_all())
        CurrencyHistoryModel::instance().purge_id(r.id());
    CurrencyHistoryModel::instance().db_release_savepoint();

    if (wxMessageBox(_t("Do you want to update the currency rates?")
            , _t("Currency Manager")
            , wxYES_NO | wxYES_DEFAULT | wxICON_QUESTION) != wxYES)
        return true;
    OnlineUpdateCurRate();

    return true;
}

bool CurrencyChoiceDialog::ConvertHistoryRates(
    const std::vector<CurrencyHistoryRate>& Bce,
    std::vector<CurrencyHistoryRate>& ConvertedRates,
    const wxString& BaseCurrencySymbol
) {
    std::map<wxDateTime, double> BaseRatesList;
    for (auto &CurrencyHistoryBce : Bce) {
        if (CurrencyHistoryBce.Currency == BaseCurrencySymbol)
            BaseRatesList[CurrencyHistoryBce.Date] = CurrencyHistoryBce.Rate;
    }

    if (BaseRatesList.size() == 0) {
        mmErrorDialogs::MessageError(this
            , wxString::Format(_t("Unable to download history for base symbol %s. Historical rates unavailable!"), BaseCurrencySymbol)
            , _t("Currency history error"));
        return false;
    }

    for (auto &CurrencyHistoryBce : Bce) {
        CurrencyHistoryRate Rate;

        if (CurrencyHistoryBce.Currency == BaseCurrencySymbol) {
            Rate.Currency = "EUR";
            Rate.Rate = CurrencyHistoryBce.Rate;
        }
        else {
            Rate.Currency = CurrencyHistoryBce.Currency;
            Rate.Rate = BaseRatesList[CurrencyHistoryBce.Date] / CurrencyHistoryBce.Rate;
        }

        Rate.BaseCurrency = BaseCurrencySymbol;
        Rate.Date = CurrencyHistoryBce.Date;

        ConvertedRates.push_back(Rate);
    }
    return true;
}

void CurrencyChoiceDialog::OnTextChanged(wxCommandEvent& event)
{
    int id = event.GetId();
    if (id == wxID_FIND) {
        m_maskStr = event.GetString();
        if (!m_maskStr.empty())
            m_maskStr = "*" + m_maskStr.Lower() + "*";
        fillControls();
    }
}

bool CurrencyChoiceDialog::GetOnlineHistory(
    const wxString& symbol,
    wxDateTime begin_date,
    std::map<wxDateTime, double>& historical_rates,
    wxString& msg
) {
    wxString base_currency_symbol;
    if (!CurrencyModel::GetBaseCurrencySymbol(base_currency_symbol)) {
        msg = _t("Unable to find base currency symbol!");
        return false;
    }

    wxString s = "%s%s=X";
    if (!g_fiat_curr().Contains(symbol))
        s = "%s-%s";

    const wxString period1 = wxString::Format("%lld", begin_date.GetTicks()); //"1577836800";
    const wxString URL = wxString::Format(mmex::weblink::YahooQuotesHistory,
        wxString::Format(s, symbol, base_currency_symbol),
        wxString::Format("period1=%s&period2=9999999999&interval=1d", period1)
    );

    wxString json_data;
    auto err_code = http_get_data(URL, json_data);
    if (err_code != CURLE_OK) {
        msg = json_data;
        return false;
    }

    Document json_doc;
    if (json_doc.Parse(json_data.utf8_str()).HasParseError()) {
        return false;
    }

    if (!json_doc.HasMember("chart") || !json_doc["chart"].IsObject())
        return false;

    Value chart = json_doc["chart"].GetObject();

    wxASSERT(chart.HasMember("error"));
    if (!chart.HasMember("error") || !chart["error"].IsNull())
        return false;

    if (!chart.HasMember("result") || !chart["result"].IsArray())
        return false;
    Value result = chart["result"].GetArray();

    if (!result.IsArray() || !result.Begin()->IsObject())
        return false;
    Value data = result.Begin()->GetObject();

    if (!data.HasMember("timestamp") || !data["timestamp"].IsArray())
        return false;
    Value timestamp = data["timestamp"].GetArray();

    if (!data.HasMember("indicators") || !data.IsObject())
        return false;
    Value indicators = data["indicators"].GetObject();

    if (!indicators.HasMember("adjclose") || !indicators["adjclose"].IsArray())
        return false;
    Value quote_array = indicators["adjclose"].GetArray();
    Value quotes = quote_array.Begin()->GetObject();
    if (!quotes.HasMember("adjclose") || !quotes["adjclose"].IsArray())
        return false;
    Value quotes_closed = quotes["adjclose"].GetArray();

    if (timestamp.Size() != quotes_closed.Size())
        return false;

    std::map<wxDate, double> history;
    const wxDateTime today = wxDateTime::Today();
    wxDateTime first_date = today;
    double first_price = 0;

    bool only_1 = true;
    for (rapidjson::SizeType i = 0; i < timestamp.Size(); i++) {
        wxASSERT(timestamp[i].IsInt());
        const auto time = wxDateTime(static_cast<time_t>(timestamp[i].GetInt())).GetDateOnly();
        if (quotes_closed[i].IsFloat()) {
            double rate = quotes_closed[i].GetFloat();
            history[time] = rate;
            if (first_date > time) {
                first_date = time;
                first_price = rate;
            }
            if (rate != 1) only_1 = false;
        }
        else {
            wxLogDebug("%s %s",
                time.FormatISODate(),
                wxDateTime::GetWeekDayName(time.GetWeekDay())
            );
        }
    }

    // Skip rates = 1 (Yahoo returns 1 with invalid Symbols)
    if (only_1)
        return false;

    double closed_price = first_price;
    for (wxDateTime i = first_date; i < today; i.Add(wxDateSpan::Days(1))) {
        wxLogDebug("Date: %s %s", i.FormatISODate(), i.FormatISOTime());
        double rate = closed_price;
        if (history.find(i) != history.end()) {
            rate = history[i];
            closed_price = rate;
        }
        historical_rates[i] = rate;
    }

    wxLogDebug("Date: %s %s Today: %s %s",
        first_date.FormatISODate(), first_date.FormatISOTime(),
        today.FormatISODate(), today.FormatISOTime()
    );
    return true;
}
