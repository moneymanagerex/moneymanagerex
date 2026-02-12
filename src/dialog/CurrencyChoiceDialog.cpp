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
    InfotableModel::instance().setSize("CURRENCY_DIALOG_SIZE", GetSize());
}

CurrencyChoiceDialog::CurrencyChoiceDialog(
    wxWindow* parent
    , int64 currencyID
    , bool bEnableSelect
)   :
    bEnableSelect_(bEnableSelect)
    , m_maskStr("")
{
    bHistoryEnabled_ = PreferencesModel::instance().getUseCurrencyHistory();

    ColName_[CURR_BASE]   = " ";
    ColName_[CURR_SYMBOL] = _t("Code");
    ColName_[CURR_NAME]   = _t("Name");
    ColName_[BASE_RATE]   = bHistoryEnabled_ ? _t("Last Rate") : _t("Fixed Rate");

    m_currency_id = currencyID == -1 ? PreferencesModel::instance().getBaseCurrencyID() : currencyID;
    this->SetFont(parent->GetFont());
    Create(parent);
    bEnableSelect_ ? SetMinSize(wxSize(200, 350)) : SetMinSize(wxSize(500, 350));
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
    int selected_index = currencyListBox_->GetSelectedRow();
    currencyListBox_->DeleteAllItems();

    cbShowAll_->SetValue(InfotableModel::instance().getBool("SHOW_HIDDEN_CURRENCIES", true));

    int64 base_currency_id = -1;
    if (CurrencyModel::GetBaseCurrency()) {
        base_currency_id = CurrencyModel::GetBaseCurrency()->CURRENCYID;
    }

    bool skip_unused = !cbShowAll_->IsChecked();
    for (const auto& currency : CurrencyModel::instance().all(CurrencyModel::COL_CURRENCYNAME))
    {
        int64 currencyID = currency.CURRENCYID;

        if (skip_unused && !(AccountModel::is_used(currency) || currencyID == base_currency_id))
            continue;
        if (!m_maskStr.IsEmpty())
        {
            if (!currency.CURRENCYNAME.Lower().Matches(m_maskStr) && !currency.CURRENCY_SYMBOL.Lower().Matches(m_maskStr))
                continue;
        }
        wxString amount;
        if (-1 == base_currency_id) // Not yet set
            amount = _t("N/A");
        else
            amount = bHistoryEnabled_
            ? CurrencyModel::toString(CurrencyHistoryModel::getLastRate(currencyID), nullptr, 4)
            : CurrencyModel::toString(currency.BASECONVRATE, nullptr, 4);
        wxVector<wxVariant> data;
        data.push_back(wxVariant(base_currency_id == currencyID ? L"\u2713" : L""));
        data.push_back(wxVariant(currency.CURRENCY_SYMBOL));
        data.push_back(wxVariant(currency.CURRENCYNAME));
        data.push_back(wxVariant(amount));
        currencyListBox_->AppendItem(data, static_cast<wxUIntPtr>(currencyID.GetValue()));
        if (selected_index == currencyListBox_->GetItemCount() - 1)
        {
            currencyListBox_->SelectRow(selected_index);
            m_currency_id = currencyID;
        }
        if (m_currency_id == currencyID)
        {
            selected_index = currencyListBox_->GetItemCount() - 1;
            currencyListBox_->SelectRow(selected_index);
        }
    }

    //Ensure that the selected item is visible.
    wxDataViewItem item(currencyListBox_->GetCurrentItem());
    currencyListBox_->EnsureVisible(item);
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
    cbShowAll_ = new wxCheckBox(this, wxID_SELECTALL, _t("&Show All"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    mmToolTip(cbShowAll_, _t("Show all even the unused currencies"));
    cbShowAll_->Connect(wxID_SELECTALL, wxEVT_COMMAND_CHECKBOX_CLICKED
        , wxCommandEventHandler(CurrencyChoiceDialog::OnShowHiddenChbClick), nullptr, this);

    itemBoxSizer22->Add(cbShowAll_, g_flagsH);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, g_flagsExpand);

    //TODO:provide proper style and fix validator (does not working)
    currencyListBox_ = new wxDataViewListCtrl(this, wxID_ANY
        , wxDefaultPosition, wxDefaultSize /*, wxDV_HORIZ_RULES, mmDoubleValidator(4)*/);

    currencyListBox_->AppendTextColumn(ColName_[CURR_BASE], wxDATAVIEW_CELL_INERT, 30);
    currencyListBox_->AppendTextColumn(ColName_[CURR_SYMBOL], wxDATAVIEW_CELL_INERT, wxLIST_AUTOSIZE_USEHEADER, wxALIGN_LEFT, wxDATAVIEW_COL_SORTABLE);
    currencyListBox_->AppendTextColumn(ColName_[CURR_NAME], wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_RESIZABLE);
    currencyListBox_->AppendTextColumn(ColName_[BASE_RATE], wxDATAVIEW_CELL_EDITABLE, wxLIST_AUTOSIZE_USEHEADER);

    itemBoxSizer3->Add(currencyListBox_, g_flagsExpand);

    wxBoxSizer* itemBoxSizerS = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizerS, wxSizerFlags(g_flagsExpand).Proportion(0));
    itemBoxSizerS->Add(new wxStaticText(this, wxID_STATIC, _t("Search")), g_flagsH);
    m_maskTextCtrl = new wxSearchCtrl(this, wxID_FIND);
    m_maskTextCtrl->SetFocus();
    itemBoxSizerS->Add(m_maskTextCtrl, g_flagsExpand);

    wxPanel* buttonsPanel = new wxPanel(this, wxID_ANY);
    itemBoxSizer2->Add(buttonsPanel, wxSizerFlags(g_flagsV).Center());

    wxBoxSizer* buttonsSizer = new wxBoxSizer(wxVERTICAL);
    buttonsPanel->SetSizer(buttonsSizer);
    wxStdDialogButtonSizer* itemBoxSizer66 = new wxStdDialogButtonSizer;
    buttonsSizer->Add(itemBoxSizer66);

    wxStdDialogButtonSizer* itemBoxSizer9 = new wxStdDialogButtonSizer;
    buttonsSizer->Add(itemBoxSizer9, wxSizerFlags(g_flagsExpand).Border(wxALL, 0));

    m_select_btn = new wxButton(buttonsPanel, wxID_SELECTALL, _t("&Select"));
    itemBoxSizer9->Add(m_select_btn, wxSizerFlags(g_flagsExpand).Proportion(4));
    //mmToolTip(itemButtonSelect, _t("Select the currently selected currency as the selected currency for the account"));

    if (!bEnableSelect_)
        m_select_btn->Disable();

    //Some interfaces has no any close buttons, it may confuse user. Cancel button added
    wxButton* itemCancelButton = new wxButton(buttonsPanel, wxID_CANCEL, wxGetTranslation(g_CloseLabel));
    itemBoxSizer9->Add(itemCancelButton, g_flagsH);
    itemCancelButton->SetFocus();

    //History Panel -----------------------------------------------------
    wxBoxSizer* rightBoxSizer = new wxBoxSizer(wxVERTICAL);
    mainBoxSizer->Add(rightBoxSizer, g_flagsExpand);

    historyStaticBox_ = new wxStaticBox(this, wxID_ANY, _t("Historical Currency Options"));
    if (bEnableSelect_) historyStaticBox_->Hide();

    wxStaticBoxSizer* historyStaticBox_Sizer = new wxStaticBoxSizer(historyStaticBox_, wxVERTICAL);
    rightBoxSizer->Add(historyStaticBox_Sizer, g_flagsExpand);

    valueListBox_ = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
    historyStaticBox_Sizer->Add(valueListBox_, g_flagsExpand);

    wxListItem col0, col1, col2;
    // Add first column
    col0.SetId(0);
    col0.SetText(_t("Date"));
    valueListBox_->InsertColumn(0, col0);

    // Add second column
    col1.SetId(1);
    col1.SetText(_t("Price"));
    valueListBox_->InsertColumn(1, col1);

    // Add third column
    col2.SetId(2);
    col2.SetText(_t("Type"));
    valueListBox_->InsertColumn(2, col2);

    //History Buttons
    wxBoxSizer* itemBoxSizerD = new wxBoxSizer(wxHORIZONTAL);
    historyStaticBox_Sizer->Add(itemBoxSizerD, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxStaticText* datePickerLabel = new wxStaticText(this, wxID_STATIC, _t("Date"));
    itemBoxSizerD->Add(datePickerLabel, g_flagsH);

    valueDatePicker_ = new mmDatePickerCtrl(this, wxID_ANY, wxDefaultDateTime
        , wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN | wxDP_SHOWCENTURY);
    valueDatePicker_->SetMinSize(wxSize(120, -1));
    itemBoxSizerD->Add(valueDatePicker_, g_flagsExpand);
    mmToolTip(valueDatePicker_, _t("Specify the date of currency value"));
    valueDatePicker_->Disable();

    wxStaticText* textBoxLabel = new wxStaticText(this, wxID_STATIC, _t("Value"));
    itemBoxSizerD->Add(textBoxLabel, g_flagsH);

    valueTextBox_ = new mmTextCtrl(this, wxID_ANY, wxGetEmptyString()
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    valueTextBox_->SetAltPrecision(6);
    valueTextBox_->SetMinSize(wxSize(120, -1));
    mmToolTip(valueTextBox_, _t("Enter the currency value"));
    itemBoxSizerD->Add(valueTextBox_, g_flagsExpand);
    valueTextBox_->Disable();

    wxPanel* buttons_panel = new wxPanel(this, wxID_ANY);
    historyStaticBox_Sizer->Add(buttons_panel, wxSizerFlags(g_flagsV).Centre());
    wxStdDialogButtonSizer*  buttons_sizer = new wxStdDialogButtonSizer;
    buttons_panel->SetSizer(buttons_sizer);

    buttonDownloadHistory_ = new wxBitmapButton(buttons_panel, HISTORY_UPDATE, mmBitmapBundle(png::CURRATES, mmBitmapButtonSize));
    mmToolTip(buttonDownloadHistory_, _t("Download Currency Values history"));
    buttonDownloadHistory_->Disable();

    historyButtonAdd_ = new wxButton(buttons_panel, HISTORY_ADD, _t("Add/&Update "), wxDefaultPosition, wxSize(-1, buttonDownloadHistory_->GetSize().GetY()));
    mmToolTip(historyButtonAdd_, _t("Add Currency Values to history"));
    historyButtonAdd_->Disable();

    historyButtonDelete_ = new wxButton(buttons_panel, HISTORY_DELETE, _t("&Delete "), wxDefaultPosition, wxSize(-1, buttonDownloadHistory_->GetSize().GetY()));
    mmToolTip(historyButtonDelete_, _t("Delete selected Currency Values"));
    historyButtonDelete_->Disable();

    buttonDelUnusedHistory_ = new wxBitmapButton(buttons_panel, HISTORY_DELUNUSED, mmBitmapBundle(png::VOID_STAT, mmBitmapButtonSize));
    mmToolTip(buttonDelUnusedHistory_, _t("Delete Historical Currency Values for unused currencies and days"));
    buttonDelUnusedHistory_->Disable();

    buttons_sizer->Add(buttonDownloadHistory_, g_flagsH);
    buttons_sizer->Add(historyButtonAdd_, g_flagsH);
    buttons_sizer->Add(historyButtonDelete_, g_flagsH);
    buttons_sizer->Add(buttonDelUnusedHistory_, g_flagsH);

    if (bEnableSelect_ || !bHistoryEnabled_) {
        historyStaticBox_->Hide();
        valueListBox_->Hide();
        buttons_panel->Hide();
        valueDatePicker_->Hide();
        valueDatePicker_->Hide();
        valueTextBox_->Hide();
        buttonDownloadHistory_->Hide();
        historyButtonAdd_->Hide();
        historyButtonDelete_->Hide();
        buttonDelUnusedHistory_->Hide();
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
    CurrencyModel::Data *currency = CurrencyModel::instance().get(m_currency_id);
    if (currency)
        CurrencyManager(this, currency).ShowModal();
    fillControls();
}

void CurrencyChoiceDialog::OnBtnSelect(wxCommandEvent& /*event*/)
{
    if (m_currency_id > -1)
        EndModal(wxID_OK);
}

void CurrencyChoiceDialog::OnBtnDelete()
{
    int selected_index = currencyListBox_->GetSelectedRow();
    if (selected_index < 0) return;

    CurrencyModel::Data* currency = CurrencyModel::instance().get(m_currency_id);
    if (!currency) return;
    if (wxMessageBox(_t("Do you want to delete the selected currency?")
        , _t("Currency Manager")
        , wxYES_NO | wxNO_DEFAULT | wxICON_ERROR) == wxYES)
    {
        CurrencyModel::instance().remove(m_currency_id);
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
    dlg.valueListBox_->Enable(false);
    dlg.valueTextBox_->Enable(false);
    dlg.valueDatePicker_->Enable(false);
    dlg.historyButtonAdd_->Enable(false);
    dlg.historyButtonDelete_->Enable(false);

    bool ok = dlg.ShowModal() == wxID_OK;
    currencyID = dlg.m_currency_id;
    dlg.Destroy();

    return ok;
}

void CurrencyChoiceDialog::OnListItemSelected(wxDataViewEvent& event)
{
    bool is_selected = currencyListBox_->GetSelectedRow() > -1;
    m_select_btn->Enable(is_selected && bEnableSelect_);

    if (is_selected)
    {
        wxDataViewItem item = event.GetItem();
        m_currency_id = static_cast<int64>(currencyListBox_->GetItemData(item));
        CurrencyModel::Data* currency = CurrencyModel::instance().get(m_currency_id);
        if (currency)
        {

            if (!bEnableSelect_)    // prevent user deleting currencies when editing accounts.
            {
                int64 baseCurrencyID = PreferencesModel::instance().getBaseCurrencyID();
                if (bHistoryEnabled_)
                {
                    buttonDownloadHistory_->Enable(m_currency_id != baseCurrencyID);
                    buttonDelUnusedHistory_->Enable(m_currency_id != baseCurrencyID);
                    valueDatePicker_->Enable(m_currency_id != baseCurrencyID);
                    valueTextBox_->Enable(m_currency_id != baseCurrencyID);
                    valueTextBox_->SetValue(0, CurrencyModel::instance().get(m_currency_id), 6);
                }
                else if (CurrencyHistoryModel::instance().find(CurrencyHistoryModel::CURRENCYID(m_currency_id)).size() > 0)
                {
                    if (wxMessageBox(wxString::Format(_t(
                        "Historical rates for %1$s found, but “Use historical currency” in options is disabled:\n"
                        "click No and enable it or click Yes to remove all historical rates for %2$s"),
                        currency->CURRENCY_SYMBOL, currency->CURRENCY_SYMBOL), _t("Currency Manager")
                        , wxYES_NO | wxNO_DEFAULT | wxICON_WARNING) == wxYES)
                    {
                        CurrencyHistoryModel::instance().Savepoint();
                        for (const auto& r : CurrencyHistoryModel::instance().find(CurrencyHistoryModel::CURRENCYID(m_currency_id)))
                            CurrencyHistoryModel::instance().remove(r.id());
                        CurrencyHistoryModel::instance().ReleaseSavepoint();
                    }
                }
            }
            historyStaticBox_->SetLabel(wxString::Format(_t("Currency History Options: %s"), currency->CURRENCYNAME));
        }
    }

    valueDatePicker_->SetValue(wxDateTime::Today());
    valueTextBox_->SetValue(wxEmptyString);
    ShowCurrencyHistory();
}

void CurrencyChoiceDialog::OnListItemActivated(wxDataViewEvent& /* event */)
{
    wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, wxID_ANY);

    if (bEnableSelect_)
        OnBtnSelect(evt);
    else
        OnBtnEdit();
}

bool CurrencyChoiceDialog::OnlineUpdateCurRate(int64 curr_id, bool hide)
{
    wxString msg = wxEmptyString;
    bool ok = getOnlineCurrencyRates(msg, curr_id, cbShowAll_->IsChecked());
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

    bool is_selected = currencyListBox_->GetSelectedRow() > -1;
    m_select_btn->Enable(is_selected);
    int64 baseCurrencyID = PreferencesModel::instance().getBaseCurrencyID();
    mainMenu->Enable(MENU_ITEM1, baseCurrencyID != m_currency_id && is_selected);
    mainMenu->Enable(MENU_ITEM2, baseCurrencyID != m_currency_id && is_selected);

    CurrencyModel::Data* currency = CurrencyModel::instance().get(m_currency_id);
    mainMenu->Enable(wxID_REMOVE, !AccountModel::is_used(currency) && is_selected);

    mainMenu->Enable(wxID_EDIT, is_selected);

    PopupMenu(mainMenu);
    delete mainMenu;
    event.Skip();
}

void CurrencyChoiceDialog::OnShowHiddenChbClick(wxCommandEvent& WXUNUSED(event))
{
    InfotableModel::instance().setBool("SHOW_HIDDEN_CURRENCIES", cbShowAll_->IsChecked());
    fillControls();
}

void CurrencyChoiceDialog::OnCancel(wxCommandEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}

void CurrencyChoiceDialog::ShowCurrencyHistory()
{
    if (m_static_dialog) return;    //Abort when trying to set base currency
    valueListBox_->DeleteAllItems();

    int64 baseCurrencyID = PreferencesModel::instance().getBaseCurrencyID();
    if (!bHistoryEnabled_ || m_currency_id <= 0 || m_currency_id == baseCurrencyID)
    {
        historyButtonAdd_->Disable();
        historyButtonDelete_->Disable();
        return;
    }
    else
    {
        historyButtonAdd_->Enable();
        historyButtonDelete_->Enable();
    }

    CurrencyModel::Data* currency = CurrencyModel::instance().get(m_currency_id);
    CurrencyHistoryModel::Data_Set histData = CurrencyHistoryModel::instance()
        .find(CurrencyHistoryModel::CURRENCYID(m_currency_id));
    std::stable_sort(histData.begin(), histData.end(), SorterByCURRDATE());
    std::reverse(histData.begin(), histData.end());
    if (!histData.empty())
    {
        int idx = 0;
        for (const auto &d : histData)
        {
            wxListItem item;
            item.SetId(idx);
            item.SetData(reinterpret_cast<void*>(d.CURRHISTID.GetValue()));
            valueListBox_->InsertItem(item);
            const wxString dispAmount = CurrencyModel::toString(d.CURRVALUE, currency, 6);
            valueListBox_->SetItem(idx, 0, mmGetDateTimeForDisplay(d.CURRDATE));
            valueListBox_->SetItem(idx, 1, dispAmount);


            const wxString& priceAmount = "* M"[d.CURRUPDTYPE.GetValue()];
            valueListBox_->SetItem(idx, 2, priceAmount);
            idx++;
        }
        valueListBox_->RefreshItems(0, --idx);
        valueListBox_->SetColumnWidth(0, wxLIST_AUTOSIZE);
        valueListBox_->SetColumnWidth(1, wxLIST_AUTOSIZE);
    }
}

void CurrencyChoiceDialog::OnHistoryAdd(wxCommandEvent& /*event*/)
{
    if (m_currency_id <= 0) return;

    double dPrice = 0.0;
    wxString currentPriceStr = valueTextBox_->GetValue().Trim();
    if (!CurrencyModel::fromString(currentPriceStr, dPrice, CurrencyModel::instance().get(m_currency_id)) || (dPrice < 0.0))
        return mmErrorDialogs::ToolTip4Object(valueTextBox_, _t("Invalid Entry"), _t("Amount"));
    CurrencyHistoryModel::instance().addUpdate(m_currency_id, valueDatePicker_->GetValue(), dPrice, CurrencyHistoryModel::MANUAL);

    fillControls();
    ShowCurrencyHistory();
}

void CurrencyChoiceDialog::OnHistoryDelete(wxCommandEvent& WXUNUSED(event))
{
    if (valueListBox_->GetSelectedItemCount() <= 0) return;

    long item = -1;
    CurrencyHistoryModel::instance().Savepoint();
    for (;;)
    {
        item = valueListBox_->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (item == -1) break;
        CurrencyHistoryModel::instance().remove(static_cast<int64>(valueListBox_->GetItemData(item)));
    }
    CurrencyHistoryModel::instance().ReleaseSavepoint();

    fillControls();
    ShowCurrencyHistory();
}

void CurrencyChoiceDialog::OnHistoryUpdate(wxCommandEvent& WXUNUSED(event))
{
    if (m_static_dialog) return;    //Abort when trying to set base currency
    CurrencyModel::Data* CurrentCurrency = CurrencyModel::instance().get(m_currency_id);
    if (!CurrentCurrency)
    {
        return mmErrorDialogs::MessageError(this, _t("No currency selected!"), _t("Currency history error"));
    }

    wxString base_currency_symbol;
    wxASSERT_MSG(CurrencyModel::GetBaseCurrencySymbol(base_currency_symbol), "Unable to find base currency symbol");

    int msgResult = wxMessageBox(_t("Do you want to add dates without any details?")
        , _t("Currency Manager")
        , wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);
    bool isCheckDate = msgResult == wxNO;

    wxString msg;
    const std::map<wxDateTime, int> DatesList = CurrencyModel::DateUsed(m_currency_id);
    wxDateTime begin_date = wxDateTime::Now().Subtract(wxDateSpan::Years(1));
    if (isCheckDate && !DatesList.empty()) {
        begin_date = DatesList.begin()->first;
    }
    wxLogDebug("Begin Date: %s", begin_date.FormatISODate());

    std::map<wxDateTime, double> historical_rates;
    bool isUpdStatus = GetOnlineHistory(CurrentCurrency->CURRENCY_SYMBOL, begin_date, historical_rates, msg);

    if (!isUpdStatus && !g_fiat_curr().Contains(CurrentCurrency->CURRENCY_SYMBOL)) {
        wxString coincap_id;
        double coincap_price_usd;
        isUpdStatus = getCoincapInfoFromSymbol(CurrentCurrency->CURRENCY_SYMBOL, coincap_id, coincap_price_usd, msg);
        if (isUpdStatus)
            isUpdStatus = getCoincapAssetHistory(coincap_id, begin_date, historical_rates, msg);
    }

    if (!isUpdStatus)
    {
        return mmErrorDialogs::MessageError(this
            , wxString::Format(_t("Unable to download %s currency rates")
                , CurrentCurrency->CURRENCY_SYMBOL)
            , _t("Historical currency error"));
    }

    bool isFound = !historical_rates.empty();
    if (isFound)
    {
        CurrencyHistoryModel::instance().Savepoint();
        if (isCheckDate)
        {
            for (const auto entry : DatesList)
            {
                wxLogDebug("%s %s", entry.first.FormatISODate(), entry.first.FormatISOTime());
                if (historical_rates.find(entry.first) != historical_rates.end())
                {
                    const wxDate date = entry.first;
                    wxLogDebug("%s", date.FormatISODate());
                    CurrencyHistoryModel::instance().addUpdate(m_currency_id, date, historical_rates[date], CurrencyHistoryModel::ONLINE);
                }
            }
        }
        else
        {
            for (auto &entry : historical_rates)
            {
                CurrencyHistoryModel::instance().addUpdate(m_currency_id, entry.first, entry.second, CurrencyHistoryModel::ONLINE);
            }
        }
        CurrencyHistoryModel::instance().ReleaseSavepoint();

        fillControls();
        ShowCurrencyHistory();
    }
    else
        mmErrorDialogs::MessageError(this
            , wxString::Format(_t("Unable to download history for symbol %s. Historical rates unavailable!")
                , CurrentCurrency->CURRENCY_SYMBOL)
            , _t("Historical currency error"));
}

void CurrencyChoiceDialog::OnHistoryDeleteUnused(wxCommandEvent& WXUNUSED(event))
{
    if (m_static_dialog) return;    //Abort when trying to set base currency
    CurrencyHistoryModel::instance().Savepoint();
    auto currencies = CurrencyModel::instance().all();
    for (auto &currency : currencies)
    {
        if (!AccountModel::is_used(currency))
        {
            for (const auto& r : CurrencyHistoryModel::instance().find(CurrencyHistoryModel::CURRENCYID(currency.CURRENCYID)))
                CurrencyHistoryModel::instance().remove(r.id());
        }
        else
        {
            std::map<wxDateTime, int> DatesList = CurrencyModel::DateUsed(currency.CURRENCYID);
            wxDateTime CurrDate;
            for (const auto& r : CurrencyHistoryModel::instance().find(CurrencyHistoryModel::CURRENCYID(currency.CURRENCYID)))
            {
                CurrDate.ParseDate(r.CURRDATE);
                if (DatesList.find(CurrDate) == DatesList.end())
                    CurrencyHistoryModel::instance().remove(r.id());
            }
        }
    }
    CurrencyHistoryModel::instance().ReleaseSavepoint();

    fillControls();
    ShowCurrencyHistory();
}

void CurrencyChoiceDialog::OnHistorySelected(wxListEvent& event)
{
    long selectedIndex = event.GetIndex();
    int64 histId = valueListBox_->GetItemData(selectedIndex);
    CurrencyHistoryModel::Data *histData = CurrencyHistoryModel::instance().get(histId);

    if (histData->CURRHISTID > 0)
    {
        valueDatePicker_->SetValue(CurrencyHistoryModel::CURRDATE(*histData));
        valueTextBox_->SetValue(wxString::Format("%f", histData->CURRVALUE));
    }
}

void CurrencyChoiceDialog::OnHistoryDeselected(wxListEvent& WXUNUSED(event))
{
    valueDatePicker_->SetValue(wxDateTime::Today());
    valueTextBox_->SetValue(wxEmptyString);
}

bool CurrencyChoiceDialog::SetBaseCurrency(int64& baseCurrencyID)
{
    int64 baseCurrencyOLD = PreferencesModel::instance().getBaseCurrencyID();
    if (baseCurrencyID == baseCurrencyOLD)
        return true;

    if (bHistoryEnabled_)
    {
        if (wxMessageBox(_t("Changing base currency will delete all historical rates, proceed?")
            , _t("Currency Manager")
            , wxYES_NO | wxYES_DEFAULT | wxICON_WARNING) != wxYES)
            return true;
    }

    PreferencesModel::instance().setBaseCurrencyID(baseCurrencyID);

    //Update baseconvrate
    CurrencyModel::instance().Savepoint();
    auto currencies = CurrencyModel::instance().all();
    for (auto& currency : currencies)
    {
        currency.BASECONVRATE = 1;
        CurrencyModel::instance().save(&currency);
    }
    CurrencyModel::instance().ReleaseSavepoint();

    //Delete historical currency
    CurrencyHistoryModel::instance().Savepoint();
    for (const auto& r : CurrencyHistoryModel::instance().all())
        CurrencyHistoryModel::instance().remove(r.id());
    CurrencyHistoryModel::instance().ReleaseSavepoint();

    if (wxMessageBox(_t("Do you want to update the currency rates?")
            , _t("Currency Manager")
            , wxYES_NO | wxYES_DEFAULT | wxICON_QUESTION) != wxYES)
        return true;
    OnlineUpdateCurRate();

    return true;
}

bool CurrencyChoiceDialog::ConvertHistoryRates(const std::vector<CurrencyHistoryRate>& Bce, std::vector<CurrencyHistoryRate>& ConvertedRates, const wxString& BaseCurrencySymbol)
{
    std::map<wxDateTime, double> BaseRatesList;
    for (auto &CurrencyHistoryBce : Bce)
    {
        if (CurrencyHistoryBce.Currency == BaseCurrencySymbol)
            BaseRatesList[CurrencyHistoryBce.Date] = CurrencyHistoryBce.Rate;
    }

    if (BaseRatesList.size() == 0)
    {
        mmErrorDialogs::MessageError(this
            , wxString::Format(_t("Unable to download history for base symbol %s. Historical rates unavailable!"), BaseCurrencySymbol)
            , _t("Currency history error"));
        return false;
    }

    for (auto &CurrencyHistoryBce : Bce)
    {
        CurrencyHistoryRate Rate;

        if (CurrencyHistoryBce.Currency == BaseCurrencySymbol)
        {
            Rate.Currency = "EUR";
            Rate.Rate = CurrencyHistoryBce.Rate;
        }
        else
        {
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
    if (id == wxID_FIND)
    {
        m_maskStr = event.GetString();
        if (!m_maskStr.empty()) m_maskStr = "*" + m_maskStr.Lower() + "*";
        fillControls();
    }
}

bool CurrencyChoiceDialog::GetOnlineHistory(const wxString &symbol, wxDateTime begin_date, std::map<wxDateTime, double> &historical_rates, wxString &msg)
{
    wxString base_currency_symbol;
    if (!CurrencyModel::GetBaseCurrencySymbol(base_currency_symbol)) {
        msg = _t("Unable to find base currency symbol!");
        return false;
    }

    wxString s = "%s%s=X";
    if (!g_fiat_curr().Contains(symbol))
        s = "%s-%s";

    const wxString period1 = wxString::Format("%lld", begin_date.GetTicks()); //"1577836800";
    const wxString URL = wxString::Format(mmex::weblink::YahooQuotesHistory
        , wxString::Format(s, symbol, base_currency_symbol)
        , wxString::Format("period1=%s&period2=9999999999&interval=1d", period1)
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
    for (rapidjson::SizeType i = 0; i < timestamp.Size(); i++)
    {
        wxASSERT(timestamp[i].IsInt());
        const auto time = wxDateTime(static_cast<time_t>(timestamp[i].GetInt())).GetDateOnly();
        if (quotes_closed[i].IsFloat())
        {
            double rate = quotes_closed[i].GetFloat();
            history[time] = rate;
            if (first_date > time)
            {
                first_date = time;
                first_price = rate;
            }
            if (rate != 1) only_1 = false;
        }
        else
        {
            wxLogDebug("%s %s", time.FormatISODate(), wxDateTime::GetWeekDayName(time.GetWeekDay()));
        }
    }

    // Skip rates = 1 (Yahoo returns 1 with invalid Symbols)
    if (only_1) return false;

    double closed_price = first_price;
    for (wxDateTime i = first_date; i < today; i.Add(wxDateSpan::Days(1)))
    {
        wxLogDebug("Date: %s %s", i.FormatISODate(), i.FormatISOTime());
        double rate = closed_price;
        if (history.find(i) != history.end())
        {
            rate = history[i];
            closed_price = rate;
        }
        historical_rates[i] = rate;
    }

    wxLogDebug("Date: %s %s Today: %s %s"
        , first_date.FormatISODate(), first_date.FormatISOTime()
        , today.FormatISODate(), today.FormatISOTime());
    return true;
}
