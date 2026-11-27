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

#include "base/_defs.h"
#include <vector>
#include <wx/sstream.h>
#include <wx/xml/xml.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>

#include "base/_constants.h"
#include "util/mmPath.h"
#include "util/mmImage.h"
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
    InfoModel::instance().saveSize("CURRENCY_DIALOG_SIZE", GetSize());
}

CurrencyChoiceDialog::CurrencyChoiceDialog(
    wxWindow* parent,
    int64 currencyID,
    bool bEnableSelect
) :
    m_maskStr(""),
    m_select_en(bEnableSelect)
{
    m_history_en = PrefModel::instance().getUseCurrencyHistory();

    m_col_name_m[CURR_BASE]   = " ";
    m_col_name_m[CURR_SYMBOL] = _t("Code");
    m_col_name_m[CURR_NAME]   = _t("Name");
    m_col_name_m[BASE_RATE]   = m_history_en ? _t("Last Rate") : _t("Fixed Rate");

    m_currency_id = currencyID == -1 ? PrefModel::instance().getBaseCurrencyID() : currencyID;
    if (parent) {
        this->SetFont(parent->GetFont());
    }
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
    SetIcon(mmPath::getProgramIcon());
    fillControls();
    Centre();

    return true;
}

void CurrencyChoiceDialog::fillControls()
{
    int selected_index = w_currency_list->GetSelectedRow();
    w_currency_list->DeleteAllItems();

    w_show_all_cb->SetValue(InfoModel::instance().getBool("SHOW_HIDDEN_CURRENCIES", true));

    int64 base_currency_id = PrefModel::instance().getBaseCurrencyID();

    bool only_used = !w_show_all_cb->IsChecked();
    for (const auto& currency_d : CurrencyModel::instance().find_data_a(
        TableClause::ORDERBY(CurrencyCol::NAME_CURRENCYNAME)
    )) {
        int64 currency_id = currency_d.m_id;
        bool is_used = CurrencyModel::instance().find_id_isUsed(currency_id, true);
        if (only_used && !is_used)
            continue;

        if (!m_maskStr.IsEmpty()) {
            if (!currency_d.m_name.Lower().Matches(m_maskStr) &&
                !currency_d.m_symbol.Lower().Matches(m_maskStr)
            )
                continue;
        }

        wxString amount = (base_currency_id > -1)
            ? CurrencyModel::instance().toString(
                m_history_en
                    ? CurrencyHistoryModel::instance().get_id_last_rate(currency_id)
                    : currency_d.m_base_conv_rate,
                nullptr, 4
            )
            : _t("N/A");
        wxVector<wxVariant> data;
        data.push_back(wxVariant(base_currency_id == currency_id ? L"\u2713" : L""));
        data.push_back(wxVariant(currency_d.m_symbol));
        data.push_back(wxVariant(currency_d.m_name));
        data.push_back(wxVariant(amount));
        w_currency_list->AppendItem(data, static_cast<wxUIntPtr>(currency_id.GetValue()));
        if (selected_index == w_currency_list->GetItemCount() - 1) {
            w_currency_list->SelectRow(selected_index);
            m_currency_id = currency_id;
        }
        if (m_currency_id == currency_id) {
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

    wxBitmapButton* update_button = new wxBitmapButton(this, wxID_EXECUTE, mmImage::bitmapBundle(mmImage::png::CURRATES, mmImage::bitmapButtonSize));
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

    w_date_picker = new mmDatePicker(this, wxID_ANY, wxDefaultDateTime
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

    w_download_btn = new wxBitmapButton(buttons_panel, HISTORY_UPDATE, mmImage::bitmapBundle(mmImage::png::CURRATES, mmImage::bitmapButtonSize));
    mmToolTip(w_download_btn, _t("Download Currency Values history"));
    w_download_btn->Disable();

    w_add_btn = new wxButton(buttons_panel, HISTORY_ADD, _t("Add/&Update "), wxDefaultPosition, wxSize(-1, w_download_btn->GetSize().GetY()));
    mmToolTip(w_add_btn, _t("Add Currency Values to history"));
    w_add_btn->Disable();

    w_delete_btn = new wxButton(buttons_panel, HISTORY_DELETE, _t("&Delete "), wxDefaultPosition, wxSize(-1, w_download_btn->GetSize().GetY()));
    mmToolTip(w_delete_btn, _t("Delete selected Currency Values"));
    w_delete_btn->Disable();

    w_delete_unused_btn = new wxBitmapButton(buttons_panel, HISTORY_DELUNUSED, mmImage::bitmapBundle(mmImage::png::VOID_STAT, mmImage::bitmapButtonSize));
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
    const CurrencyData *currency_n = CurrencyModel::instance().get_idN_data_n(m_currency_id);
    if (currency_n)
        CurrencyManager(this, currency_n).ShowModal();
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

    const CurrencyData* currency_n = CurrencyModel::instance().get_idN_data_n(
        m_currency_id
    );
    if (!currency_n)
        return;

    if (wxMessageBox(
        _t("Do you want to delete the selected currency?"),
        _t("Currency Manager"),
        wxYES_NO | wxNO_DEFAULT | wxICON_ERROR
    ) != wxYES)
        return;

    CurrencyModel::instance().purge_id(m_currency_id);
    m_currency_id = -1;
    fillControls();
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
    CurrencyChoiceDialog dlg(NULL, currencyID);
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
        const CurrencyData* data_n = CurrencyModel::instance().get_idN_data_n(m_currency_id);
        if (data_n) {

            // prevent user deleting currencies when editing accounts.
            if (!m_select_en) {
                int64 baseCurrencyID = PrefModel::instance().getBaseCurrencyID();
                if (m_history_en) {
                    w_download_btn->Enable(m_currency_id != baseCurrencyID);
                    w_delete_unused_btn->Enable(m_currency_id != baseCurrencyID);
                    w_date_picker->Enable(m_currency_id != baseCurrencyID);
                    w_value_text->Enable(m_currency_id != baseCurrencyID);
                    w_value_text->SetValue(
                        0,
                        CurrencyModel::instance().get_idN_data_n(m_currency_id),
                        6
                    );
                }
                else if (CurrencyHistoryModel::instance().find_count(
                    CurrencyHistoryCol::WHERE_CURRENCYID(OP_EQ, m_currency_id)
                ) > 0) {
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
                        CurrencyHistoryModel::instance().purge_currencyId_all(m_currency_id);
                    }
                }
            }
            w_hostory_box->SetLabel(wxString::Format(
                _t("Currency History Options: %s"),
                data_n->m_name
            ));
        }
    }

    w_date_picker->setValue(wxDateTime::Today());
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

    //wxCommandEvent ev(wxEVT_COMMAND_MENU_SELECTED, wxID_ANY);
    //ev.SetEventObject(this);

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

    const CurrencyData* currency_n = CurrencyModel::instance().get_idN_data_n(
        m_currency_id
    );
    if (currency_n) {
        mainMenu->Enable(wxID_REMOVE,
            is_selected &&
            !CurrencyModel::instance().find_id_isUsed(currency_n->m_id, true)
        );
    }

    mainMenu->Enable(wxID_EDIT, is_selected);

    PopupMenu(mainMenu);
    delete mainMenu;
    event.Skip();
}

void CurrencyChoiceDialog::OnShowHiddenChbClick(wxCommandEvent& WXUNUSED(event))
{
    InfoModel::instance().saveBool("SHOW_HIDDEN_CURRENCIES", w_show_all_cb->IsChecked());
    fillControls();
}

void CurrencyChoiceDialog::OnCancel(wxCommandEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}

void CurrencyChoiceDialog::ShowCurrencyHistory()
{
    // Abort when trying to set base currency
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

    const CurrencyData* currency = CurrencyModel::instance().get_idN_data_n(m_currency_id);
    CurrencyHistoryModel::DataA uh_a = CurrencyHistoryModel::instance().find_data_a(
        CurrencyHistoryCol::WHERE_CURRENCYID(OP_EQ, m_currency_id),
        TableClause::ORDERBY(CurrencyHistoryCol::NAME_CURRDATE, true)
    );
    if (uh_a.empty())
        return;

    int idx = 0;
    for (const CurrencyHistoryData& uh_d : uh_a) {
        wxListItem item;
        item.SetId(idx);
        item.SetData(reinterpret_cast<void*>(uh_d.m_id.GetValue()));
        w_history_list->InsertItem(item);
        const wxString dispAmount = CurrencyModel::instance().toString(
            uh_d.m_base_conv_rate, currency, 6
        );
        w_history_list->SetItem(idx, 0, mmGetDateTimeForDisplay(uh_d.m_date.isoDate()));
        w_history_list->SetItem(idx, 1, dispAmount);


        const wxString& priceAmount = "* M"[uh_d.m_update_type.id()];
        w_history_list->SetItem(idx, 2, priceAmount);
        idx++;
    }

    w_history_list->RefreshItems(0, --idx);
    w_history_list->SetColumnWidth(0, wxLIST_AUTOSIZE);
    w_history_list->SetColumnWidth(1, wxLIST_AUTOSIZE);
}

void CurrencyChoiceDialog::OnHistoryAdd(wxCommandEvent& /*event*/)
{
    if (m_currency_id <= 0) return;

    double dPrice = 0.0;
    wxString currentPriceStr = w_value_text->GetValue().Trim();
    if (!CurrencyModel::instance().fromString(
        currentPriceStr, dPrice,
        CurrencyModel::instance().get_idN_data_n(m_currency_id)
    ) || dPrice < 0.0)
        return mmErrorDialogs::ToolTip4Object(w_value_text, _t("Invalid Entry"), _t("Amount"));
    CurrencyHistoryModel::instance().save_record(
        m_currency_id,
        mmDate(w_date_picker->GetValue()),
        dPrice,
        UpdateType(UpdateType::e_manual)
    );

    fillControls();
    ShowCurrencyHistory();
}

void CurrencyChoiceDialog::OnHistoryDelete(wxCommandEvent& WXUNUSED(event))
{
    if (w_history_list->GetSelectedItemCount() <= 0) return;

    CurrencyHistoryModel::instance().db_savepoint();
    long item = -1;
    for (;;) {
        item = w_history_list->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (item == -1)
            break;
        CurrencyHistoryModel::instance().purge_id(
            static_cast<int64>(w_history_list->GetItemData(item))
        );
    }
    CurrencyHistoryModel::instance().db_release_savepoint();

    fillControls();
    ShowCurrencyHistory();
}

void CurrencyChoiceDialog::OnHistoryUpdate(wxCommandEvent& WXUNUSED(event))
{
    // Abort when trying to set base currency
    if (m_static_dialog)
        return;

    const CurrencyData* currency_n = CurrencyModel::instance().get_idN_data_n(m_currency_id);
    if (!currency_n) {
        return mmErrorDialogs::MessageError(this,
            _t("No currency selected!"),
            _t("Currency history error")
        );
    }

    wxString base_currency_symbol;
    wxASSERT_MSG(
        CurrencyModel::instance().get_base_symbol(base_currency_symbol),
        "Unable to find base currency symbol"
    );

    int msgResult = wxMessageBox(
        _t("Do you want to add dates without any details?"),
        _t("Currency Manager"),
        wxYES_NO | wxNO_DEFAULT | wxICON_WARNING
    );
    bool isCheckDate = (msgResult == wxNO);

    wxString msg;
    std::set<mmDate> date_m = CurrencyModel::instance().find_id_date_m(
        m_currency_id
    );
    mmDate begin_date = (isCheckDate && !date_m.empty())
        ? *date_m.begin()
        : mmDate::today().minusDateSpan(wxDateSpan::Years(1));
    wxLogDebug("Begin Date: %s", begin_date.isoDate());

    std::map<mmDate, double> date_rate_m;
    bool isUpdStatus = GetOnlineHistory(
        currency_n->m_symbol, begin_date, date_rate_m, msg
    );

    if (!isUpdStatus && !g_fiat_curr().Contains(currency_n->m_symbol)) {
        wxString coincap_id;
        double coincap_price_usd;
        isUpdStatus = getCoincapInfoFromSymbol(
            currency_n->m_symbol, coincap_id, coincap_price_usd, msg
        );
        if (isUpdStatus)
            isUpdStatus = getCoincapAssetHistory(coincap_id, begin_date, date_rate_m, msg);
    }

    if (!isUpdStatus) {
        return mmErrorDialogs::MessageError(this,
            wxString::Format(_t("Unable to download %s currency rates"),
                currency_n->m_symbol
            ),
            _t("Historical currency error")
        );
    }

    if (date_rate_m.empty()) {
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
        for (const mmDate& date : date_m) {
            if (date_rate_m.find(date) == date_rate_m.end())
                continue;
            wxLogDebug("%s", date.isoDate());
            CurrencyHistoryModel::instance().save_record(
                m_currency_id,
                date,
                date_rate_m[date],
                UpdateType(UpdateType::e_online)
            );
        }
    }
    else {
        for (auto& date_rate : date_rate_m) {
            CurrencyHistoryModel::instance().save_record(
                m_currency_id,
                date_rate.first,
                date_rate.second,
                UpdateType(UpdateType::e_online)
            );
        }
    }
    CurrencyHistoryModel::instance().db_release_savepoint();

    fillControls();
    ShowCurrencyHistory();
}

void CurrencyChoiceDialog::OnHistoryDeleteUnused(wxCommandEvent& WXUNUSED(event))
{
    // Abort when trying to set base currency
    if (m_static_dialog)
        return;

    CurrencyHistoryModel::instance().db_savepoint();
    for (int64 currency_id : CurrencyModel::instance().find_id_a()) {
        if (!CurrencyModel::instance().find_id_isUsed(currency_id, true)) {
            CurrencyHistoryModel::instance().purge_currencyId_all(currency_id);
        }
        else {
            std::set<mmDate> date_m = CurrencyModel::instance().find_id_date_m(currency_id);
            for (const CurrencyHistoryData& uh_d : CurrencyHistoryModel::instance().find_data_a(
                CurrencyHistoryCol::WHERE_CURRENCYID(OP_EQ, currency_id)
            )) {
                if (date_m.find(uh_d.m_date) == date_m.end())
                    CurrencyHistoryModel::instance().purge_id(uh_d.m_id);
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
    CurrencyHistoryData* ch_n = CurrencyHistoryModel::instance().unsafe_get_idN_data_n(
        histId
    );

    if (ch_n->m_id > 0) {
        w_date_picker->setValue(ch_n->m_date.dateTime());
        w_value_text->SetValue(wxString::Format("%f", ch_n->m_base_conv_rate));
    }
}

void CurrencyChoiceDialog::OnHistoryDeselected(wxListEvent& WXUNUSED(event))
{
    w_date_picker->setValue(wxDateTime::Today());
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

    PrefModel::instance().saveBaseCurrencyID(baseCurrencyID);

    // Update base_conv_rate
    CurrencyModel::instance().db_savepoint();
    for (CurrencyData& currency_d : CurrencyModel::instance().find_data_a()) {
        currency_d.m_base_conv_rate = 1.0;
        CurrencyModel::instance().save_data_n(currency_d);
    }
    CurrencyModel::instance().db_release_savepoint();

    CurrencyHistoryModel::instance().purge_all();

    if (wxMessageBox(_t("Do you want to update the currency rates?"),
        _t("Currency Manager"),
        wxYES_NO | wxYES_DEFAULT | wxICON_QUESTION
    ) != wxYES)
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
    mmDate begin_date,
    std::map<mmDate, double>& date_rate_m,
    wxString& msg
) {
    wxString base_currency_symbol;
    if (!CurrencyModel::instance().get_base_symbol(base_currency_symbol)) {
        msg = _t("Unable to find base currency symbol!");
        return false;
    }

    wxString s = "%s%s=X";
    if (!g_fiat_curr().Contains(symbol))
        s = "%s-%s";

    time_t begin_ticks = begin_date.dateTime().GetTicks();
    const wxString period1 = wxString::Format("%lld", begin_ticks); //"1577836800";
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

    std::map<mmDate, double> date_close_m;
    mmDate today = mmDate::today();
    mmDate first_date = today;
    double first_rate = 0;

    bool only_1 = true;
    for (rapidjson::SizeType i = 0; i < timestamp.Size(); i++) {
        wxASSERT(timestamp[i].IsInt());
        mmDate date = mmDate(wxDateTime(static_cast<time_t>(timestamp[i].GetInt())));
        if (quotes_closed[i].IsFloat()) {
            double rate = quotes_closed[i].GetFloat();
            date_close_m[date] = rate;
            if (date < first_date) {
                first_date = date;
                first_rate = rate;
            }
            if (rate != 1)
                only_1 = false;
        }
        else {
            wxLogDebug("%s %s",
                date.isoDate(),
                wxDateTime::GetEnglishWeekDayName(date.dateTime().GetWeekDay())
            );
        }
    }

    // Skip rates = 1 (Yahoo returns 1 with invalid Symbols)
    if (only_1)
        return false;

    double rate = first_rate;
    for (mmDate date = first_date; date < today; date.addDateSpan(wxDateSpan::Days(1))) {
        wxLogDebug("Date: %s", date.isoDate());
        if (date_close_m.find(date) != date_close_m.end()) {
            rate = date_close_m[date];
        }
        date_rate_m[date] = rate;
    }

    wxLogDebug("Date: %s Today: %s", first_date.isoDate(), today.isoDate());
    return true;
}
