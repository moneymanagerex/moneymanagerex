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

#include "maincurrencydialog.h"
#include "currencydialog.h"
#include "constants.h"
#include "images_list.h"
#include "mmSimpleDialogs.h"
#include "mmTextCtrl.h"
#include "paths.h"
#include "util.h"
#include "validators.h"
#include "model/allmodel.h"
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>

#include <vector>
#include <wx/sstream.h>
#include <wx/xml/xml.h>
using namespace rapidjson;

wxIMPLEMENT_DYNAMIC_CLASS(mmMainCurrencyDialog, wxDialog);

wxBEGIN_EVENT_TABLE(mmMainCurrencyDialog, wxDialog)
    EVT_BUTTON(wxID_SELECTALL, mmMainCurrencyDialog::OnBtnSelect)
    EVT_BUTTON(wxID_CANCEL, mmMainCurrencyDialog::OnCancel)
    EVT_TEXT(wxID_ANY, mmMainCurrencyDialog::OnTextChanged)
    EVT_MENU(wxID_ANY, mmMainCurrencyDialog::OnMenuSelected)

    EVT_DATAVIEW_ITEM_ACTIVATED(wxID_ANY, mmMainCurrencyDialog::OnListItemActivated)
    EVT_DATAVIEW_SELECTION_CHANGED(wxID_ANY, mmMainCurrencyDialog::OnListItemSelected)
    EVT_DATAVIEW_ITEM_CONTEXT_MENU(wxID_ANY, mmMainCurrencyDialog::OnItemRightClick)

    EVT_BUTTON(HISTORY_ADD, mmMainCurrencyDialog::OnHistoryAdd)
    EVT_BUTTON(HISTORY_DELETE, mmMainCurrencyDialog::OnHistoryDelete)
    EVT_BUTTON(HISTORY_UPDATE, mmMainCurrencyDialog::OnHistoryUpdate)
    EVT_BUTTON(HISTORY_DELUNUSED, mmMainCurrencyDialog::OnHistoryDeleteUnused)
    EVT_LIST_ITEM_SELECTED(wxID_ANY, mmMainCurrencyDialog::OnHistorySelected)
    EVT_LIST_ITEM_DESELECTED(wxID_ANY, mmMainCurrencyDialog::OnHistoryDeselected)
    wxEND_EVENT_TABLE()

mmMainCurrencyDialog::~mmMainCurrencyDialog()
{
    Model_Infotable::instance().Set("CURRENCY_DIALOG_SIZE", GetSize());
}

mmMainCurrencyDialog::mmMainCurrencyDialog(
    wxWindow* parent
    , int currencyID
    , bool bEnableSelect
)   : currencyListBox_(nullptr)
    , buttonDownloadHistory_(nullptr)
    , buttonDelUnusedHistory_(nullptr)
    , bHistoryEnabled_(false)
    , bEnableSelect_(bEnableSelect)
    , m_static_dialog(false)
    , m_maskStr("")
{
    bHistoryEnabled_ = Option::instance().getCurrencyHistoryEnabled();

    ColName_[CURR_BASE]   = " ";
    ColName_[CURR_SYMBOL] = _("Symbol");
    ColName_[CURR_NAME]   = _("Name");
    ColName_[BASE_RATE]   = bHistoryEnabled_ ? _("Last Rate") : _("Fixed Rate");

    m_currency_id = currencyID == -1 ? Option::instance().getBaseCurrencyID() : currencyID;
    this->SetFont(parent->GetFont());
    Create(parent);
    bEnableSelect_ ? SetMinSize(wxSize(200, 350)) : SetMinSize(wxSize(500, 350));
    mmSetSize(this);
    Centre();
}

bool mmMainCurrencyDialog::Create(wxWindow* parent
    , wxWindowID id
    , const wxString& caption
    , const wxString& name
    , const wxPoint& pos
    , const wxSize& size
    , long style)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, wxGetTranslation(caption), pos, size, style, name);

    CreateControls();
    SetIcon(mmex::getProgramIcon());
    fillControls();
    Centre();

    return TRUE;
}

void mmMainCurrencyDialog::fillControls()
{
    int selected_index = currencyListBox_->GetSelectedRow();
    currencyListBox_->DeleteAllItems();

    cbShowAll_->SetValue(Model_Infotable::instance().GetBoolInfo("SHOW_HIDDEN_CURRENCIES", true));

    int base_currency_id = -1;
    if (Model_Currency::GetBaseCurrency()) {
        base_currency_id = Model_Currency::GetBaseCurrency()->CURRENCYID;
    }

    bool skip_unused = !cbShowAll_->IsChecked();
    for (const auto& currency : Model_Currency::instance().all(Model_Currency::COL_CURRENCYNAME))
    {
        int currencyID = currency.CURRENCYID;

        if (skip_unused && !(Model_Account::is_used(currency) || currencyID == base_currency_id))
            continue;
        if (!m_maskStr.IsEmpty())
        {
            if (!currency.CURRENCYNAME.Lower().Matches(m_maskStr) && !currency.CURRENCY_SYMBOL.Lower().Matches(m_maskStr))
                continue;
        }
        wxString amount;
        if (-1 == base_currency_id) // Not yet set
            amount = _("N/A");
        else
            amount = bHistoryEnabled_
            ? Model_Currency::toString(Model_CurrencyHistory::getLastRate(currencyID), nullptr, 4)
            : Model_Currency::toString(currency.BASECONVRATE, nullptr, 4);
        wxVector<wxVariant> data;
        data.push_back(wxVariant(base_currency_id == currencyID ? L"\u2713" : L""));
        data.push_back(wxVariant(currency.CURRENCY_SYMBOL));
        data.push_back(wxVariant(currency.CURRENCYNAME));
        data.push_back(wxVariant(amount));
        currencyListBox_->AppendItem(data, static_cast<wxUIntPtr>(currencyID));
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

void mmMainCurrencyDialog::CreateControls()
{
    wxBoxSizer* mainBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    this->SetSizer(mainBoxSizer);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    mainBoxSizer->Add(itemBoxSizer2, g_flagsExpand);

    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer22, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxBitmapButton* update_button = new wxBitmapButton(this, wxID_STATIC, mmBitmap(png::CURRATES, mmBitmapButtonSize));
    itemBoxSizer22->Add(update_button, g_flagsH);
    update_button->Connect(wxID_STATIC, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(mmMainCurrencyDialog::OnOnlineUpdateCurRate), nullptr, this);
    mmToolTip(update_button, _("Online update currency rate"));
    itemBoxSizer22->AddSpacer(4);

    itemBoxSizer22->Add(new wxStaticText(this, wxID_STATIC, _("Online Update")), g_flagsH);

    itemBoxSizer22->AddSpacer(15);
    cbShowAll_ = new wxCheckBox(this, wxID_SELECTALL, _("Show All"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    mmToolTip(cbShowAll_, _("Show all even the unused currencies"));
    cbShowAll_->Connect(wxID_SELECTALL, wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(mmMainCurrencyDialog::OnShowHiddenChbClick), nullptr, this);

    itemBoxSizer22->Add(cbShowAll_, g_flagsH);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, g_flagsExpand);

    //TODO:provide proper style and fix validator (does not working)
    currencyListBox_ = new wxDataViewListCtrl(this, wxID_ANY
        , wxDefaultPosition, wxDefaultSize /*, wxDV_HORIZ_RULES, mmDoubleValidator(4)*/);

    currencyListBox_->AppendTextColumn(ColName_[CURR_BASE], wxDATAVIEW_CELL_INERT, 30);
    currencyListBox_->AppendTextColumn(ColName_[CURR_SYMBOL], wxDATAVIEW_CELL_INERT, wxLIST_AUTOSIZE_USEHEADER, wxALIGN_LEFT, wxDATAVIEW_COL_SORTABLE);
    currencyListBox_->AppendTextColumn(ColName_[CURR_NAME], wxDATAVIEW_CELL_INERT, 170, wxALIGN_LEFT, wxDATAVIEW_COL_SORTABLE);
    currencyListBox_->AppendTextColumn(ColName_[BASE_RATE], wxDATAVIEW_CELL_EDITABLE, wxLIST_AUTOSIZE_USEHEADER);

    itemBoxSizer3->Add(currencyListBox_, g_flagsExpand);

    wxPanel* buttonsPanel = new wxPanel(this, wxID_ANY);
    itemBoxSizer2->Add(buttonsPanel, wxSizerFlags(g_flagsV).Center());

    wxBoxSizer* buttonsSizer = new wxBoxSizer(wxVERTICAL);
    buttonsPanel->SetSizer(buttonsSizer);
    wxStdDialogButtonSizer* itemBoxSizer66 = new wxStdDialogButtonSizer;
    buttonsSizer->Add(itemBoxSizer66);

    wxStdDialogButtonSizer* itemBoxSizer9 = new wxStdDialogButtonSizer;
    buttonsSizer->Add(itemBoxSizer9, wxSizerFlags(g_flagsExpand).Border(wxALL, 0));

    itemBoxSizer66->Add(new wxStaticText(buttonsPanel, wxID_STATIC, _("Search:")), g_flagsH);
    m_maskTextCtrl = new wxSearchCtrl(buttonsPanel, wxID_FIND);
    m_maskTextCtrl->SetFocus();
    itemBoxSizer66->Add(m_maskTextCtrl, g_flagsExpand);

    m_select_btn = new wxButton(buttonsPanel, wxID_SELECTALL, _("&Select"));
    itemBoxSizer9->Add(m_select_btn, wxSizerFlags(g_flagsExpand).Proportion(4));
    //mmToolTip(itemButtonSelect, _("Select the currently selected currency as the selected currency for the account"));

    if (!bEnableSelect_)
        m_select_btn->Disable();

    //Some interfaces has no any close buttons, it may confuse user. Cancel button added
    wxButton* itemCancelButton = new wxButton(buttonsPanel, wxID_CANCEL, wxGetTranslation(g_CloseLabel));
    itemBoxSizer9->Add(itemCancelButton, g_flagsH);
    itemCancelButton->SetFocus();

    //History Panel -----------------------------------------------------
    wxBoxSizer* rightBoxSizer = new wxBoxSizer(wxVERTICAL);
    mainBoxSizer->Add(rightBoxSizer, g_flagsExpand);

    historyStaticBox_ = new wxStaticBox(this, wxID_ANY, _("Currency History Options"));
    if (bEnableSelect_) historyStaticBox_->Hide();

    wxStaticBoxSizer* historyStaticBox_Sizer = new wxStaticBoxSizer(historyStaticBox_, wxVERTICAL);
    rightBoxSizer->Add(historyStaticBox_Sizer, g_flagsExpand);

    valueListBox_ = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
    historyStaticBox_Sizer->Add(valueListBox_, g_flagsExpand);

    wxListItem col0, col1, col2;
    // Add first column
    col0.SetId(0);
    col0.SetText(_("Date"));
    col0.SetWidth(90);
    valueListBox_->InsertColumn(0, col0);

    // Add second column
    col1.SetId(1);
    col1.SetText(_("Price"));
    col1.SetWidth(100);
    valueListBox_->InsertColumn(1, col1);

    // Add third column
    col2.SetId(2);
    col2.SetText(_("Type"));
    col2.SetWidth(90);
    valueListBox_->InsertColumn(2, col2);

    //History Buttons
    wxPanel* values_panel = new wxPanel(this, wxID_ANY);
    historyStaticBox_Sizer->Add(values_panel, wxSizerFlags(g_flagsV).Centre());
    wxStdDialogButtonSizer*  values_sizer = new wxStdDialogButtonSizer;
    values_panel->SetSizer(values_sizer);

    wxStaticText* datePickerLabel = new wxStaticText(values_panel, wxID_STATIC, _("Date"));
    values_sizer->Add(datePickerLabel, g_flagsH);

    valueDatePicker_ = new mmDatePickerCtrl(values_panel, wxID_ANY, wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN | wxDP_SHOWCENTURY);
    valueDatePicker_->SetMinSize(wxSize(120, -1));
    values_sizer->Add(valueDatePicker_, g_flagsH);
    mmToolTip(valueDatePicker_, _("Specify the date of currency value"));
    valueDatePicker_->Disable();

    wxStaticText* textBoxLabel = new wxStaticText(values_panel, wxID_STATIC, _("Value"));
    values_sizer->Add(textBoxLabel, g_flagsH);

    valueTextBox_ = new mmTextCtrl(values_panel, wxID_ANY, wxGetEmptyString(), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    valueTextBox_->SetMinSize(wxSize(120, -1));
    mmToolTip(valueTextBox_, _("Enter the currency value"));
    values_sizer->Add(valueTextBox_, g_flagsH);
    valueTextBox_->Disable();

    wxPanel* buttons_panel = new wxPanel(this, wxID_ANY);
    historyStaticBox_Sizer->Add(buttons_panel, wxSizerFlags(g_flagsV).Centre());
    wxStdDialogButtonSizer*  buttons_sizer = new wxStdDialogButtonSizer;
    buttons_panel->SetSizer(buttons_sizer);

    buttonDownloadHistory_ = new wxBitmapButton(buttons_panel, HISTORY_UPDATE, mmBitmap(png::CURRATES, mmBitmapButtonSize));
    mmToolTip(buttonDownloadHistory_, _("Download Currency Values history"));
    buttonDownloadHistory_->Disable();

    historyButtonAdd_ = new wxButton(buttons_panel, HISTORY_ADD, _("&Add / Update "), wxDefaultPosition, wxSize(-1, buttonDownloadHistory_->GetSize().GetY()));
    mmToolTip(historyButtonAdd_, _("Add Currency Values to history"));
    historyButtonAdd_->Disable();

    historyButtonDelete_ = new wxButton(buttons_panel, HISTORY_DELETE, _("&Delete "), wxDefaultPosition, wxSize(-1, buttonDownloadHistory_->GetSize().GetY()));
    mmToolTip(historyButtonDelete_, _("Delete selected Currency Values"));
    historyButtonDelete_->Disable();

    buttonDelUnusedHistory_ = new wxBitmapButton(buttons_panel, HISTORY_DELUNUSED, mmBitmap(png::VOID_STAT, mmBitmapButtonSize));
    mmToolTip(buttonDelUnusedHistory_, _("Delete Currency Values history for unused currencies and days"));
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

void mmMainCurrencyDialog::OnBtnAdd()
{
    mmCurrencyDialog dlg(this, 0);
    if (dlg.ShowModal() == wxID_OK)
    {
        m_currency_id = dlg.getCurrencyID();
    }

    fillControls();
}

void mmMainCurrencyDialog::OnBtnEdit()
{
    Model_Currency::Data *currency = Model_Currency::instance().get(m_currency_id);
    if (currency)
        mmCurrencyDialog(this, currency).ShowModal();
    fillControls();
}

void mmMainCurrencyDialog::OnBtnSelect(wxCommandEvent& /*event*/)
{
    if (m_currency_id > -1)
        EndModal(wxID_OK);
}

void mmMainCurrencyDialog::OnBtnDelete()
{
    int selected_index = currencyListBox_->GetSelectedRow();
    if (selected_index < 0) return;

    Model_Currency::Data* currency = Model_Currency::instance().get(m_currency_id);
    if (!currency) return;
    if (wxMessageBox(_("Do you really want to delete the selected Currency?")
        , _("Currency Dialog")
        , wxYES_NO | wxNO_DEFAULT | wxICON_ERROR) == wxYES)
    {
        Model_Currency::instance().remove(m_currency_id);
        m_currency_id = -1;
        fillControls();
    }
}

bool mmMainCurrencyDialog::Execute(wxWindow* parent, int& currencyID)
{
    bool result = false;

    mmMainCurrencyDialog dlg(parent, currencyID);
    if (dlg.ShowModal() == wxID_OK)
    {
        currencyID = dlg.m_currency_id;
        result = true;
    }
    dlg.Destroy();

    return result;
}

bool mmMainCurrencyDialog::Execute(int& currencyID)
{
    mmMainCurrencyDialog dlg(NULL, currencyID);
    dlg.m_static_dialog = true;
    dlg.SetTitle(_("Base Currency Selection"));
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

void mmMainCurrencyDialog::OnListItemSelected(wxDataViewEvent& event)
{
    bool is_selected = currencyListBox_->GetSelectedRow() > -1;
    m_select_btn->Enable(is_selected && bEnableSelect_);

    if (is_selected)
    {
        wxDataViewItem item = event.GetItem();
        m_currency_id = static_cast<int>(currencyListBox_->GetItemData(item));
        Model_Currency::Data* currency = Model_Currency::instance().get(m_currency_id);
        if (currency)
        {

            if (!bEnableSelect_)    // prevent user deleting currencies when editing accounts.
            {
                int baseCurrencyID = Option::instance().getBaseCurrencyID();
                if (bHistoryEnabled_)
                {
                    buttonDownloadHistory_->Enable(m_currency_id != baseCurrencyID);
                    buttonDelUnusedHistory_->Enable(m_currency_id != baseCurrencyID);
                    valueDatePicker_->Enable(m_currency_id != baseCurrencyID);
                    valueTextBox_->Enable(m_currency_id != baseCurrencyID);
                }
                else if (Model_CurrencyHistory::instance().find(Model_CurrencyHistory::CURRENCYID(m_currency_id)).size() > 0)
                {
                    if (wxMessageBox(wxString::Format(_(
                        "Historic rates for %s found, but \"Use currency history\" in options is disabled:\n"
                        "click no and enable it or click yes to remove all historic rates for %s"),
                        currency->CURRENCY_SYMBOL, currency->CURRENCY_SYMBOL), _("Currency Dialog")
                        , wxYES_NO | wxNO_DEFAULT | wxICON_WARNING) == wxYES)
                    {
                        Model_CurrencyHistory::instance().Savepoint();
                        for (const auto& r : Model_CurrencyHistory::instance().find(Model_CurrencyHistory::CURRENCYID(m_currency_id)))
                            Model_CurrencyHistory::instance().remove(r.id());
                        Model_CurrencyHistory::instance().ReleaseSavepoint();
                    }
                }
            }
            historyStaticBox_->SetLabel(wxString::Format(_("Currency History Options: %s "), currency->CURRENCYNAME));
        }
    }

    valueDatePicker_->SetValue(wxDateTime::Today());
    valueTextBox_->SetValue(wxEmptyString);
    ShowCurrencyHistory();
}

void mmMainCurrencyDialog::OnListItemActivated(wxDataViewEvent& /* event */)
{
    wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, wxID_ANY);

    if (bEnableSelect_)
        OnBtnSelect(evt);
    else
        OnBtnEdit();
}

bool mmMainCurrencyDialog::OnlineUpdateCurRate(int curr_id, bool hide)
{
    wxString msg = wxEmptyString;
    bool ok = getOnlineCurrencyRates(msg, curr_id, cbShowAll_->IsChecked());
    if (ok)
    {
        if (!hide)
        {
            wxMessageDialog msgDlg(this, msg, _("Online update currency rate"));
            msgDlg.ShowModal();
        }
        fillControls();
        ShowCurrencyHistory();
    }
    else
    {
        wxMessageDialog msgDlg(this, msg, _("Error"), wxOK | wxICON_ERROR);
        msgDlg.ShowModal();
    }
    return ok;
}

void mmMainCurrencyDialog::OnOnlineUpdateCurRate(wxCommandEvent& /*event*/)
{
    if (!m_static_dialog)    //Abort when trying to set base currency
    {
        OnlineUpdateCurRate(-1, false);
    }
}

void mmMainCurrencyDialog::OnMenuSelected(wxCommandEvent& event)
{
    switch (event.GetId())
    {
        case MENU_ITEM1:
            if (!SetBaseCurrency(m_currency_id))
                mmErrorDialogs::MessageError(this
                    , _("Unable to update history currency rates. Please update them manually!")
                    , _("Currency history error"));

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

void mmMainCurrencyDialog::OnItemRightClick(wxDataViewEvent& event)
{
    // disable this function when setting base currency
    if (m_static_dialog) return;

    wxCommandEvent ev(wxEVT_COMMAND_MENU_SELECTED, wxID_ANY);
    ev.SetEventObject(this);

    wxMenu* mainMenu = new wxMenu;

    mainMenu->Append(new wxMenuItem(mainMenu, MENU_ITEM1, _("Set as Base Currency")));
    mainMenu->Append(new wxMenuItem(mainMenu, MENU_ITEM2, _("Online Update Currency Rate")));
    mainMenu->AppendSeparator();
    mainMenu->Append(new wxMenuItem(mainMenu, wxID_ADD, _("&Add ")));
    mainMenu->Append(new wxMenuItem(mainMenu, wxID_EDIT, _("&Edit ")));
    mainMenu->Append(new wxMenuItem(mainMenu, wxID_REMOVE, _("&Remove ")));

    bool is_selected = currencyListBox_->GetSelectedRow() > -1;
    m_select_btn->Enable(is_selected);
    int baseCurrencyID = Option::instance().getBaseCurrencyID();
    mainMenu->Enable(MENU_ITEM1, baseCurrencyID != m_currency_id && is_selected);
    mainMenu->Enable(MENU_ITEM2, baseCurrencyID != m_currency_id && is_selected);

    Model_Currency::Data* currency = Model_Currency::instance().get(m_currency_id);
    mainMenu->Enable(wxID_REMOVE, !Model_Account::is_used(currency) && is_selected);

    mainMenu->Enable(wxID_EDIT, is_selected);

    PopupMenu(mainMenu);
    delete mainMenu;
    event.Skip();
} 

void mmMainCurrencyDialog::OnShowHiddenChbClick(wxCommandEvent& WXUNUSED(event))
{
    Model_Infotable::instance().Set("SHOW_HIDDEN_CURRENCIES", cbShowAll_->IsChecked());
    fillControls();
}

void mmMainCurrencyDialog::OnCancel(wxCommandEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}

void mmMainCurrencyDialog::ShowCurrencyHistory()
{
    if (m_static_dialog) return;    //Abort when trying to set base currency
    valueListBox_->DeleteAllItems();

    int baseCurrencyID = Option::instance().getBaseCurrencyID();
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

    Model_Currency::Data* currency = Model_Currency::instance().get(m_currency_id);
    Model_CurrencyHistory::Data_Set histData = Model_CurrencyHistory::instance()
        .find(Model_CurrencyHistory::CURRENCYID(m_currency_id));
    std::stable_sort(histData.begin(), histData.end(), SorterByCURRDATE());
    std::reverse(histData.begin(), histData.end());
    if (!histData.empty())
    {
        int idx = 0;
        for (const auto &d : histData)
        {
            wxListItem item;
            item.SetId(idx);
            item.SetData(d.CURRHISTID);
            valueListBox_->InsertItem(item);
            const wxString dispAmount = Model_Currency::toString(d.CURRVALUE, currency, 6);
            valueListBox_->SetItem(idx, 0, mmGetDateForDisplay(d.CURRDATE));
            valueListBox_->SetItem(idx, 1, dispAmount);


            const wxString& priceAmount = "* M"[d.CURRUPDTYPE];
            valueListBox_->SetItem(idx, 2, priceAmount);
            idx++;
        }
        valueListBox_->RefreshItems(0, --idx);
    }
}

void mmMainCurrencyDialog::OnHistoryAdd(wxCommandEvent& /*event*/)
{
    if (m_currency_id <= 0) return;

    wxString listStr;
    wxDateTime dt;
    double dPrice = 0.0;
    Model_Currency::Data* currency = Model_Currency::instance().get(m_currency_id);
    wxString currentPriceStr = valueTextBox_->GetValue().Trim();
    if (!Model_Currency::fromString(currentPriceStr, dPrice, currency) || (dPrice < 0.0))
        return mmErrorDialogs::ToolTip4Object(valueTextBox_, _("Invalid Entry"), _("Amount"));
    Model_CurrencyHistory::instance().addUpdate(m_currency_id, valueDatePicker_->GetValue(), dPrice, Model_CurrencyHistory::MANUAL);

    fillControls();
    ShowCurrencyHistory();
}

void mmMainCurrencyDialog::OnHistoryDelete(wxCommandEvent& WXUNUSED(event))
{
    if (valueListBox_->GetSelectedItemCount() <= 0) return;

    long item = -1;
    Model_CurrencyHistory::instance().Savepoint();
    for (;;)
    {
        item = valueListBox_->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (item == -1) break;
        Model_CurrencyHistory::instance().remove(static_cast<int>(valueListBox_->GetItemData(item)));
    }
    Model_CurrencyHistory::instance().ReleaseSavepoint();

    fillControls();
    ShowCurrencyHistory();
}

void mmMainCurrencyDialog::OnHistoryUpdate(wxCommandEvent& WXUNUSED(event))
{
    if (m_static_dialog) return;    //Abort when trying to set base currency
    Model_Currency::Data* CurrentCurrency = Model_Currency::instance().get(m_currency_id);
    if (!CurrentCurrency)
    {
        return mmErrorDialogs::MessageError(this, _("No currency selected!"), _("Currency history error"));
    }

    wxString base_currency_symbol;
    wxASSERT_MSG(Model_Currency::GetBaseCurrencySymbol(base_currency_symbol), "Could not find base currency symbol");

    int msgResult = wxMessageBox(_("Do you want to add also dates without any transaction?")
        , _("Currency Dialog")
        , wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);
    bool isCheckDate = msgResult == wxNO;

    wxString msg;
    const std::map<wxDateTime, int> DatesList = Model_Currency::DateUsed(m_currency_id);
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
            , wxString::Format(_("Unable to download %s currency rates")
                , CurrentCurrency->CURRENCY_SYMBOL)
            , _("Currency history error"));
    }

    bool isFound = !historical_rates.empty();
    if (isFound)
    {
        Model_CurrencyHistory::instance().Savepoint();
        if (isCheckDate)
        {
            for (const auto entry : DatesList)
            {
                wxLogDebug("%s %s", entry.first.FormatISODate(), entry.first.FormatISOTime());
                if (historical_rates.find(entry.first) != historical_rates.end())
                {
                    const wxDate date = entry.first;
                    wxLogDebug("%s", date.FormatISODate());
                    Model_CurrencyHistory::instance().addUpdate(m_currency_id, date, historical_rates[date], Model_CurrencyHistory::ONLINE);
                }
            }
        }
        else
        {
            for (auto &entry : historical_rates)
            {
                Model_CurrencyHistory::instance().addUpdate(m_currency_id, entry.first, entry.second, Model_CurrencyHistory::ONLINE);
            }
        }
        Model_CurrencyHistory::instance().ReleaseSavepoint();

        fillControls();
        ShowCurrencyHistory();
    }
    else
        mmErrorDialogs::MessageError(this
            , wxString::Format(_("Unable to download history for symbol %s. History rates not available!")
                , CurrentCurrency->CURRENCY_SYMBOL)
            , _("Currency history error"));
}

void mmMainCurrencyDialog::OnHistoryDeleteUnused(wxCommandEvent& WXUNUSED(event))
{
    if (m_static_dialog) return;    //Abort when trying to set base currency
    Model_CurrencyHistory::instance().Savepoint();
    auto currencies = Model_Currency::instance().all();
    for (auto &currency : currencies)
    {
        if (!Model_Account::is_used(currency))
        {
            for (const auto& r : Model_CurrencyHistory::instance().find(Model_CurrencyHistory::CURRENCYID(currency.CURRENCYID)))
                Model_CurrencyHistory::instance().remove(r.id());
        }
        else
        {
            std::map<wxDateTime, int> DatesList = Model_Currency::DateUsed(currency.CURRENCYID);
            wxDateTime CurrDate;
            for (const auto& r : Model_CurrencyHistory::instance().find(Model_CurrencyHistory::CURRENCYID(currency.CURRENCYID)))
            {
                CurrDate.ParseDate(r.CURRDATE);
                if (DatesList.find(CurrDate) == DatesList.end())
                    Model_CurrencyHistory::instance().remove(r.id());
            }
        }
    }
    Model_CurrencyHistory::instance().ReleaseSavepoint();

    fillControls();
    ShowCurrencyHistory();
}

void mmMainCurrencyDialog::OnHistorySelected(wxListEvent& event)
{
    long selectedIndex = event.GetIndex();
    long histId = valueListBox_->GetItemData(selectedIndex);
    Model_CurrencyHistory::Data *histData = Model_CurrencyHistory::instance().get(histId);

    if (histData->CURRHISTID > 0)
    {
        valueDatePicker_->SetValue(Model_CurrencyHistory::CURRDATE(*histData));
        valueTextBox_->SetValue(wxString::Format("%f", histData->CURRVALUE));
    }
}

void mmMainCurrencyDialog::OnHistoryDeselected(wxListEvent& WXUNUSED(event))
{
    valueDatePicker_->SetValue(wxDateTime::Today());
    valueTextBox_->SetValue(wxEmptyString);
}

bool mmMainCurrencyDialog::SetBaseCurrency(int& baseCurrencyID)
{
    int baseCurrencyOLD = Option::instance().getBaseCurrencyID();
    if (baseCurrencyID == baseCurrencyOLD)
        return true;

    if (bHistoryEnabled_)
    {
        if (wxMessageBox(_("Changing base currency will delete all history rates, proceed?")
            , _("Currency Dialog")
            , wxYES_NO | wxYES_DEFAULT | wxICON_WARNING) != wxYES)
            return true;
    }

    Option::instance().setBaseCurrency(baseCurrencyID);

    //Update baseconvrate
    Model_Currency::instance().Savepoint();
    auto currencies = Model_Currency::instance().all();
    for (auto& currency : currencies)
    {
        currency.BASECONVRATE = 1;
        Model_Currency::instance().save(&currency);
    }
    Model_Currency::instance().ReleaseSavepoint();

    //Delete currency history
    Model_CurrencyHistory::instance().Savepoint();
    for (const auto& r : Model_CurrencyHistory::instance().all())
        Model_CurrencyHistory::instance().remove(r.id());
    Model_CurrencyHistory::instance().ReleaseSavepoint();

    if (wxMessageBox(_("Do you want to update today currency rates?")
            , _("Currency Dialog")
            , wxYES_NO | wxYES_DEFAULT | wxICON_QUESTION) != wxYES)
        return true;
    OnlineUpdateCurRate();

    return true;
}

bool mmMainCurrencyDialog::ConvertHistoryRates(const std::vector<CurrencyHistoryRate>& Bce, std::vector<CurrencyHistoryRate>& ConvertedRates, const wxString& BaseCurrencySymbol)
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
            , wxString::Format(_("Unable to download history for base symbol %s. History rates not available!"), BaseCurrencySymbol)
            , _("Currency history error"));
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

void mmMainCurrencyDialog::OnTextChanged(wxCommandEvent& event)
{
    int id = event.GetId();
    if (id == wxID_FIND)
    {
        m_maskStr = event.GetString();
        if (!m_maskStr.empty()) m_maskStr = "*" + m_maskStr.Lower() + "*";
        fillControls();
    }
}

bool mmMainCurrencyDialog::GetOnlineHistory(const wxString &symbol, wxDateTime begin_date, std::map<wxDateTime, double> &historical_rates, wxString &msg)
{
    wxString base_currency_symbol;
    if (!Model_Currency::GetBaseCurrencySymbol(base_currency_symbol)) {
        msg = _("Could not find base currency symbol!");
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
