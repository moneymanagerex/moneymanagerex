/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2015 Gabriele-V
 Copyright (C) 2016 Nikolay

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
#include "defs.h"
#include "images_list.h"
#include "mmCalculator.h"
#include "mmSimpleDialogs.h"
#include "mmtextctrl.h"
#include "paths.h"
#include "util.h"
#include "validators.h"
#include "model/Model_Account.h"
#include "model/Model_Currency.h"
#include "model/Model_CurrencyHistory.h"
#include "model/Model_Infotable.h"
#include "model/Model_Setting.h"

#include <vector>
#include <wx/sstream.h>
#include <wx/xml/xml.h>

wxIMPLEMENT_DYNAMIC_CLASS(mmMainCurrencyDialog, wxDialog);

wxBEGIN_EVENT_TABLE(mmMainCurrencyDialog, wxDialog)
    EVT_BUTTON(wxID_ADD, mmMainCurrencyDialog::OnBtnAdd)
    EVT_BUTTON(wxID_EDIT, mmMainCurrencyDialog::OnBtnEdit)
    EVT_BUTTON(wxID_SELECTALL, mmMainCurrencyDialog::OnBtnSelect)
    EVT_BUTTON(wxID_REMOVE, mmMainCurrencyDialog::OnBtnDelete)
    EVT_BUTTON(wxID_CANCEL, mmMainCurrencyDialog::OnCancel)

    EVT_MENU_RANGE(MENU_ITEM1, MENU_ITEM2, mmMainCurrencyDialog::OnMenuSelected)

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

mmMainCurrencyDialog::mmMainCurrencyDialog(
    wxWindow* parent
    , int currencyID
    , bool bEnableSelect
) : currencyListBox_(),
    bEnableSelect_(bEnableSelect)
{
    ColName_[CURR_BASE]   = " ";
    ColName_[CURR_SYMBOL] = _("Symbol");
    ColName_[CURR_NAME]   = _("Name");
    ColName_[BASE_RATE]   = _("Last Rate");

    currencyID_ = currencyID == -1 ? Model_Infotable::instance().GetBaseCurrencyId() : currencyID;
    long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, _("Currency Dialog"), wxDefaultPosition, wxDefaultSize, style);
}

bool mmMainCurrencyDialog::Create(wxWindow* parent
    , wxWindowID id
    , const wxString& caption
    , const wxPoint& pos
    , const wxSize& size
    , long style)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style);

    CreateControls();
    SetIcon(mmex::getProgramIcon());
    fillControls();
    Centre();

    return TRUE;
}

void mmMainCurrencyDialog::fillControls()
{
    currencyListBox_->DeleteAllItems();
    int baseCurrencyID = Model_Infotable::instance().GetBaseCurrencyId();

    cbShowAll_->SetValue(Model_Infotable::instance().GetBoolInfo("SHOW_HIDDEN_CURRENCIES", true));

    bool skip_unused = !cbShowAll_->IsChecked();
    for (const auto& currency : Model_Currency::instance().all(Model_Currency::COL_CURRENCYNAME))
    {
        int currencyID = currency.CURRENCYID;
        bool currency_is_base = false;
        if (Model_Currency::GetBaseCurrency()) currency_is_base = Model_Currency::GetBaseCurrency()->CURRENCYID == currencyID;

        if (skip_unused && !(Model_Account::is_used(currency) || currency_is_base)) continue;

        wxVector<wxVariant> data;
        data.push_back(wxVariant(baseCurrencyID == currencyID));
        data.push_back(wxVariant(currency.CURRENCY_SYMBOL));
        data.push_back(wxVariant(currency.CURRENCYNAME));
        data.push_back(wxVariant(wxString()<<Model_CurrencyHistory::getLastRate(currencyID)));
        currencyListBox_->AppendItem(data, (wxUIntPtr)currencyID);
        if (selectedIndex_ == currencyListBox_->GetItemCount() - 1)
        {
            currencyListBox_->SelectRow(selectedIndex_);
            itemButtonEdit_->Enable();
            currencyID_ = currencyID;
        }
        if (currencyID_ == currencyID)
        {
            selectedIndex_ = currencyListBox_->GetItemCount() - 1;
            currencyListBox_->SelectRow(selectedIndex_);
            itemButtonEdit_->Enable();
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

    wxBitmapButton* update_button = new wxBitmapButton(this
        , wxID_STATIC, mmBitmap(png::UPDATE));
    itemBoxSizer22->Add(update_button, g_flags);
    update_button->Connect(wxID_STATIC, wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(mmMainCurrencyDialog::OnOnlineUpdateCurRate), nullptr, this);
    update_button->SetToolTip(_("Online update currency rate"));
    itemBoxSizer22->AddSpacer(4);

    itemBoxSizer22->Add(new wxStaticText(this, wxID_STATIC
        , _("Online Update")), g_flags);

    itemBoxSizer22->AddSpacer(15);
    cbShowAll_ = new wxCheckBox(this, wxID_SELECTALL, _("Show All"), wxDefaultPosition
        , wxDefaultSize, wxCHK_2STATE);
    cbShowAll_->SetToolTip(_("Show all even the unused currencies"));
    cbShowAll_->Connect(wxID_SELECTALL, wxEVT_COMMAND_CHECKBOX_CLICKED,
        wxCommandEventHandler(mmMainCurrencyDialog::OnShowHiddenChbClick), nullptr, this);

    itemBoxSizer22->Add(cbShowAll_, g_flags);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, g_flagsExpand);

    //TODO:provide proper style and fix validator (does not working)
    currencyListBox_ = new wxDataViewListCtrl( this
        , wxID_ANY, wxDefaultPosition, wxSize(-1, 200)/*, wxDV_HORIZ_RULES, mmDoubleValidator(4)*/);

    currencyListBox_->AppendToggleColumn(ColName_[CURR_BASE], wxDATAVIEW_CELL_INERT, 30);
    currencyListBox_->AppendTextColumn(ColName_[CURR_SYMBOL], wxDATAVIEW_CELL_INERT, 60
        , wxALIGN_LEFT, wxDATAVIEW_COL_SORTABLE);
    currencyListBox_->AppendTextColumn(ColName_[CURR_NAME], wxDATAVIEW_CELL_INERT, 170
        , wxALIGN_LEFT, wxDATAVIEW_COL_SORTABLE);
    currencyListBox_->AppendTextColumn(ColName_[BASE_RATE], wxDATAVIEW_CELL_EDITABLE, 80);

    itemBoxSizer3->Add(currencyListBox_, g_flagsExpand);

    wxPanel* buttonsPanel = new wxPanel(this, wxID_ANY);
    itemBoxSizer2->Add(buttonsPanel, wxSizerFlags(g_flags).Center());

    wxBoxSizer* buttonsSizer = new wxBoxSizer(wxVERTICAL);
    buttonsPanel->SetSizer(buttonsSizer);
    wxStdDialogButtonSizer* itemBoxSizer66 = new wxStdDialogButtonSizer;
    buttonsSizer->Add(itemBoxSizer66);

    wxButton* itemButton7 = new wxButton(buttonsPanel, wxID_ADD, _("&Add "));
    itemBoxSizer66->Add(itemButton7, g_flags);

    itemButtonEdit_ = new wxButton(buttonsPanel, wxID_EDIT, _("&Edit "));
    itemBoxSizer66->Add(itemButtonEdit_, g_flags);
    itemButtonEdit_->Disable();

    itemButtonDelete_ = new wxButton(buttonsPanel, wxID_REMOVE, _("&Remove "));
    itemBoxSizer66->Add(itemButtonDelete_, g_flags);
    itemButtonDelete_->Disable();

    wxStdDialogButtonSizer* itemBoxSizer9 = new wxStdDialogButtonSizer;
    buttonsSizer->Add(itemBoxSizer9, wxSizerFlags(g_flagsExpand).Border(wxALL, 0));

    wxButton* itemButtonSelect = new wxButton(buttonsPanel, wxID_SELECTALL, _("&Select"));
    itemBoxSizer9->Add(itemButtonSelect, wxSizerFlags(g_flagsExpand).Proportion(4));
    //itemButtonSelect->SetToolTip(_("Select the currently selected currency as the selected currency for the account"));

    if (bEnableSelect_ == false)
        itemButtonSelect->Disable();

    //Some interfaces has no any close buttons, it may confuse user. Cancel button added
    wxButton* itemCancelButton = new wxButton(buttonsPanel, wxID_CANCEL, wxGetTranslation(g_CloseLabel));
    itemBoxSizer9->Add(itemCancelButton, g_flags);
    itemCancelButton->SetFocus();

    //History Panel
    wxBoxSizer* rightBoxSizer = new wxBoxSizer(wxVERTICAL);
    mainBoxSizer->Add(rightBoxSizer, g_flagsExpand);

    historyStaticBox_ = new wxStaticBox(this, wxID_ANY, _("Currency History Options"));

    wxStaticBoxSizer* historyStaticBox_Sizer = new wxStaticBoxSizer(historyStaticBox_, wxVERTICAL);
    rightBoxSizer->Add(historyStaticBox_Sizer, g_flagsExpand);

    valueListBox_ = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(250, 150)
        , wxLC_REPORT);
    historyStaticBox_Sizer->Add(valueListBox_, g_flagsExpand);

    wxListItem col0, col1, col2;
    // Add first column
    col0.SetId(0);
    col0.SetText(_("Date"));
    col0.SetWidth(90);
    valueListBox_->InsertColumn(0, col0);

    // Add second column
    col1.SetId(1);
    col1.SetText(_("Value"));
    col1.SetWidth(100);
    valueListBox_->InsertColumn(1, col1);

    // Add third column
    col2.SetId(2);
    col2.SetText(_("Diff."));
    col2.SetWidth(90);
    valueListBox_->InsertColumn(2, col2);

    //History Buttons
    wxPanel* values_panel = new wxPanel(this, wxID_ANY);
    historyStaticBox_Sizer->Add(values_panel, wxSizerFlags(g_flags).Centre());
    wxStdDialogButtonSizer*  values_sizer = new wxStdDialogButtonSizer;
    values_panel->SetSizer(values_sizer);


    values_sizer->Add(new wxStaticText(values_panel, wxID_STATIC, _("Date")), g_flags);

    valueDatePicker_ = new wxDatePickerCtrl(values_panel, wxID_ANY, wxDefaultDateTime,
        wxDefaultPosition, wxSize(120, -1), wxDP_DROPDOWN | wxDP_SHOWCENTURY);
    values_sizer->Add(valueDatePicker_, g_flags);
    valueDatePicker_->SetToolTip(_("Specify the date of currency value"));

    values_sizer->Add(new wxStaticText(values_panel, wxID_STATIC, _("Value")), g_flags);

    valueTextBox_ = new mmTextCtrl(values_panel, wxID_ANY, wxGetEmptyString()
        , wxDefaultPosition, wxSize(120, -1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER
        , mmCalcValidator());
    valueTextBox_->SetToolTip(_("Enter the currency value"));
    values_sizer->Add(valueTextBox_, g_flags);

    wxPanel* buttons_panel = new wxPanel(this, wxID_ANY);
    historyStaticBox_Sizer->Add(buttons_panel, wxSizerFlags(g_flags).Centre());
    wxStdDialogButtonSizer*  buttons_sizer = new wxStdDialogButtonSizer;
    buttons_panel->SetSizer(buttons_sizer);

    wxBitmapButton* buttonDownload = new wxBitmapButton(buttons_panel, HISTORY_UPDATE, mmBitmap(png::UPDATE));
    buttonDownload->SetToolTip(_("Download Currency Values history"));
    historyButtonAdd_ = new wxButton(buttons_panel, HISTORY_ADD, _("&Add / Update "), wxDefaultPosition
        , wxSize(-1, buttonDownload->GetSize().GetY()));
    historyButtonAdd_->SetToolTip(_("Add Currency Values to history"));
    historyButtonDelete_ = new wxButton(buttons_panel, HISTORY_DELETE, _("&Delete "), wxDefaultPosition
        , wxSize(-1, buttonDownload->GetSize().GetY()));
    historyButtonDelete_->SetToolTip(_("Delete selected Currency Values"));

    wxBitmapButton* buttonDelUnusedHistory = new wxBitmapButton(buttons_panel
        , HISTORY_DELUNUSED, mmBitmap(png::VOID_STAT));
    buttonDelUnusedHistory->SetToolTip(_("Delete Currency Values history for unused currencies"));
    buttons_sizer->Add(buttonDownload, g_flags);
    buttons_sizer->Add(historyButtonAdd_, g_flags);
    buttons_sizer->Add(historyButtonDelete_, g_flags);
    buttons_sizer->Add(buttonDelUnusedHistory, g_flags);

    this->SetMinSize(wxSize(800,550));
    this->Fit();
}

void mmMainCurrencyDialog::OnBtnAdd(wxCommandEvent& /*event*/)
{
    mmCurrencyDialog dlg(this, 0);
    if (dlg.ShowModal() == wxID_OK)
    {
        currencyID_ = dlg.getCurrencyID();
        selectedIndex_ = -1;
    }

    fillControls();
}

void mmMainCurrencyDialog::OnBtnEdit(wxCommandEvent& /*event*/)
{
    Model_Currency::Data *currency = Model_Currency::instance().get(currencyID_);
    if (currency)
        mmCurrencyDialog(this, currency).ShowModal();
    fillControls();
}

void mmMainCurrencyDialog::OnBtnSelect(wxCommandEvent& /*event*/)
{
    if (selectedIndex_ > -1) EndModal(wxID_OK);
}

void mmMainCurrencyDialog::OnBtnDelete(wxCommandEvent& /*event*/)
{
    if (selectedIndex_ < 0) return;

    Model_Currency::Data* currency = Model_Currency::instance().get(currencyID_);
    if (!currency) return;
    if (wxMessageBox(_("Do you really want to delete the selected Currency?")
                        , _("Currency Dialog")
                        , wxYES_NO | wxNO_DEFAULT | wxICON_ERROR) == wxYES)
    {
        Model_Currency::instance().remove(currencyID_);
        selectedIndex_ = -1;
        currencyID_ = -1;
        fillControls();
    }
}

bool mmMainCurrencyDialog::Execute(wxWindow* parent, int& currencyID)
{
    bool result = false;

    mmMainCurrencyDialog dlg(parent, currencyID);
    if (dlg.ShowModal() == wxID_OK)
    {
        currencyID = dlg.currencyID_;
        result = true;
    }
    dlg.Destroy();

    return result;
}

void mmMainCurrencyDialog::OnListItemSelected(wxDataViewEvent& event)
{
    wxDataViewItem item = event.GetItem();
    selectedIndex_ = currencyListBox_->ItemToRow(item);

    wxString currName;
    if (selectedIndex_ >= 0)
    {
        currencyID_ = (int)currencyListBox_->GetItemData(item);
        Model_Currency::Data* currency = Model_Currency::instance().get(currencyID_);
        if (currency)
        {
            currName = currency->CURRENCYNAME;
            itemButtonEdit_->Enable();
        }
    }
    if (!bEnableSelect_)    // prevent user deleting currencies when editing accounts.
    {
        int baseCurrencyID = Model_Infotable::instance().GetBaseCurrencyId();
        Model_Currency::Data* currency = Model_Currency::instance().get(currencyID_);
        if (currency)
        {
            itemButtonDelete_->Enable(!Model_Account::is_used(currency) && currencyID_ != baseCurrencyID);
            currName = currency->CURRENCYNAME;
        }
    }

    historyStaticBox_->SetLabel(_("Currency History Options: ") + currName + " ");

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
        OnBtnEdit(evt);
}

bool mmMainCurrencyDialog::onlineUpdateCurRate(int curr_id, bool hide)
{
    wxString base_symbol = wxEmptyString;
    wxString msg = wxEmptyString;
    wxString site = mmex::weblink::YahooQuotes;
    bool ok = true;

    Model_Currency::Data * base_currency = Model_Currency::GetBaseCurrency();
    if (base_currency)
        base_symbol = base_currency->CURRENCY_SYMBOL.Upper();

    ok = !base_symbol.empty();
    if (!ok)
        msg = _("Could not find base currency symbol!");

    auto currencies = Model_Currency::instance().all();
    wxString sOutput;
    if (ok)
    {
        wxString buffer = wxEmptyString;
        for (const auto &currency : currencies)
        {
            const wxString symbol = currency.CURRENCY_SYMBOL.Upper();
            if (curr_id > 0 && currency.CURRENCYID != curr_id) continue;
            if (!symbol.IsEmpty()) buffer << symbol << base_symbol << "=X+";
        }
        if (buffer.Right(1).Contains("+")) buffer.RemoveLast(1);
        site = wxString::Format(site, buffer);

        int err_code = site_content(site, sOutput);
        if (err_code != wxURL_NOERR)
        {
            msg = sOutput;
            ok = false;
        }
    }

    if (ok)
    {
        wxString CurrencySymbol, sName;
        double dRate = 1;

        std::map<wxString, std::pair<double, wxString> > currency_data;

        // Break it up into lines
        wxStringTokenizer tkz(sOutput, "\r\n");

        while (tkz.HasMoreTokens())
        {
            wxString csvline = tkz.GetNextToken();

            wxRegEx pattern("\"(...)...=X\",([^,][0-9.]+),\"([^\"]*)\",\"([^\"]*)\"");
            if (pattern.Matches(csvline))
            {
                CurrencySymbol = pattern.GetMatch(csvline, 1);
                pattern.GetMatch(csvline, 2).ToDouble(&dRate);
                sName = pattern.GetMatch(csvline, 4);
                currency_data[CurrencySymbol] = std::make_pair(dRate, sName);
            }
        }

        msg = _("Currency rate updated");
        msg << "\n\n";

        Model_CurrencyHistory::instance().Savepoint();
        for (auto &currency : currencies)
        {
            if (!cbShowAll_->IsChecked() && !Model_Account::is_used(currency)) continue;

            const wxString currency_symbol = currency.CURRENCY_SYMBOL.Upper();
            if (!currency_symbol.IsEmpty())
            {
                if (currency_data.find(currency_symbol) != currency_data.end())
                {
                    msg << wxString::Format("%s\t: %0.6f -> %0.6f\n"
                        , currency_symbol, currency.BASECONVRATE, currency_data[currency_symbol].first);
                    currency.BASECONVRATE = currency_data[currency_symbol].first;
                    if (base_symbol == currency_symbol)
                        continue;
                    Model_CurrencyHistory::instance().addUpdate(currency.CURRENCYID,
                        wxDateTime::Today(), currency.BASECONVRATE, Model_CurrencyHistory::ONLINE);
                }
                else
                {
                    if (curr_id < 0)
                        msg << wxString::Format(_("%s\t: %s\n"), currency_symbol, _("Invalid value "));
                }
            }
        }

        //Model_Currency::instance().save(currencies);  BASECONVRATE IS FIXED AND USED IF HISTORY NOT FOUND 
        Model_CurrencyHistory::instance().ReleaseSavepoint();

        wxMessageDialog msgDlg(this, msg, _("Currency rate updated"));
        if (!hide)
            msgDlg.ShowModal();
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
    onlineUpdateCurRate(-1,false);
}

void mmMainCurrencyDialog::OnMenuSelected(wxCommandEvent& event)
{
    switch (event.GetId())
    {
        case MENU_ITEM1:
        {
            if (!SetBaseCurrency(currencyID_))
                mmErrorDialogs::MessageError(this, "Unable to update history currency rates. Please update them manually!", "Currency history error");
            itemButtonDelete_->Disable();
            fillControls();
            ShowCurrencyHistory();
        } break;
        case MENU_ITEM2: onlineUpdateCurRate(currencyID_,false); break;
    } 
}

void mmMainCurrencyDialog::OnItemRightClick(wxDataViewEvent& event)
{
    wxCommandEvent ev(wxEVT_COMMAND_MENU_SELECTED, wxID_ANY) ;
    ev.SetEventObject( this );

    wxMenu* mainMenu = new wxMenu;
    mainMenu->Append(new wxMenuItem(mainMenu, MENU_ITEM1, _("Set as Base Currency")));
    mainMenu->Append(new wxMenuItem(mainMenu, MENU_ITEM2, _("Online Update Currency Rate")));
    
    int baseCurrencyID = Model_Infotable::instance().GetBaseCurrencyId();
    if (baseCurrencyID == currencyID_)
        mainMenu->Enable(MENU_ITEM1, false);

    PopupMenu(mainMenu);
    delete mainMenu;
    event.Skip();
}

void mmMainCurrencyDialog::OnShowHiddenChbClick(wxCommandEvent& event)
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
    valueListBox_->DeleteAllItems();

    int baseCurrencyID = Model_Infotable::instance().GetBaseCurrencyId();
    if (currencyID_ <= 0 || currencyID_ == baseCurrencyID)
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

    Model_CurrencyHistory::Data_Set histData = Model_CurrencyHistory::instance().find(Model_CurrencyHistory::CURRENCYID(currencyID_));
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
            const wxDate dtdt = Model_CurrencyHistory::CURRDATE(d);
            const wxString dispAmount = wxString::Format(wxT("%f"), d.CURRVALUE);
            valueListBox_->SetItem(idx, 0, mmGetDateForDisplay(dtdt));
            valueListBox_->SetItem(idx, 1, dispAmount);
            const wxString& priceAmount = wxEmptyString;
            valueListBox_->SetItem(idx, 2, priceAmount);
            idx++;
        }
        valueListBox_->RefreshItems(0, --idx);
    }
}

void mmMainCurrencyDialog::OnHistoryAdd(wxCommandEvent& /*event*/)
{
    if (currencyID_ <= 0)
        return;

    wxString listStr;
    wxDateTime dt;
    double dPrice = 0.0;
    Model_Currency::Data* currency = Model_Currency::instance().get(currencyID_);
    wxString currentPriceStr = valueTextBox_->GetValue().Trim();
    if (!Model_Currency::fromString(currentPriceStr, dPrice, currency) || (dPrice < 0.0))
        return;
    Model_CurrencyHistory::instance().addUpdate(currencyID_, valueDatePicker_->GetValue(), dPrice, Model_CurrencyHistory::MANUAL);

    fillControls();
    ShowCurrencyHistory();
}

void mmMainCurrencyDialog::OnHistoryDelete(wxCommandEvent& /*event*/)
{
    if (valueListBox_->GetSelectedItemCount() <= 0)
        return;

    long item = -1;
    Model_CurrencyHistory::instance().Savepoint();
    for (;;)
    {
        item = valueListBox_->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

        if (item == -1)
            break;
        Model_CurrencyHistory::instance().remove((int)valueListBox_->GetItemData(item));
    }
    Model_CurrencyHistory::instance().ReleaseSavepoint();

    fillControls();
    ShowCurrencyHistory();
}

void mmMainCurrencyDialog::OnHistoryUpdate(wxCommandEvent& /*event*/)
{
    if (currencyID_ <= 0)
        return;

    //if (wxMessageBox(_("Update currency history will take some time. Do you want to proceed?")
    //    , _("Currency Dialog")
    //    , wxYES_NO | wxYES_DEFAULT | wxICON_ERROR) != wxYES)
    //    return;

    bool UpdStatus = true;
    if (_BceCurrencyHistoryRatesList.empty())
        UpdStatus = mmMainCurrencyDialog::HistoryDownloadBce();

    if (UpdStatus == false)
    {
        mmErrorDialogs::MessageError(this, "Unable to download currency rates!", "Currency history error");
        return;
    }

    wxString base_symbol = wxEmptyString;
    Model_Currency::Data* base_currency = Model_Currency::GetBaseCurrency();
    if (base_currency)
        base_symbol = base_currency->CURRENCY_SYMBOL.Upper();

    if (base_symbol.empty())
    {
        mmErrorDialogs::MessageError(this, "Could not find base currency symbol!", "Currency history error");
            return;
    }

    std::vector<CurrencyHistoryRate> CurrencyHistoryRatesList;
    if (base_symbol == "EUR")
        CurrencyHistoryRatesList = _BceCurrencyHistoryRatesList;
    else if (!ConvertHistoryRates(_BceCurrencyHistoryRatesList, CurrencyHistoryRatesList, base_symbol))
        return;

    Model_Currency::Data* CurrentCurrency = Model_Currency::instance().get(currencyID_);

    const int msgResult = wxMessageBox(_("Do you want to add also dates without any transaction?")
        , _("Currency Dialog"), wxYES_NO | wxNO_DEFAULT | wxICON_ERROR);
    bool CheckDate;
    if (msgResult == wxYES)
        CheckDate = false;
    else if (msgResult == wxNO)
        CheckDate = true;
    else
        return;

    const std::map<wxDateTime, int> DatesList = Model_Currency::DateUsed(currencyID_);
    bool Found = false;
    Model_CurrencyHistory::instance().Savepoint();
    for (auto &CurrencyHistory : CurrencyHistoryRatesList)
    {
        if (CurrencyHistory.Currency == CurrentCurrency->CURRENCY_SYMBOL.Upper())
        {
            Found = true;
            if (CheckDate && DatesList.find(CurrencyHistory.Date) == DatesList.end())
                continue;
            Model_CurrencyHistory::instance().addUpdate(currencyID_, CurrencyHistory.Date, CurrencyHistory.Rate, Model_CurrencyHistory::ONLINE);
        }
    }
    Model_CurrencyHistory::instance().ReleaseSavepoint();

    if (Found)
    {
        fillControls();
        ShowCurrencyHistory();
    }
    else
        mmErrorDialogs::MessageError(this,
            wxString::Format("Unable to download history for symbol %S. History rates not available!", CurrentCurrency->CURRENCY_SYMBOL.Upper()),
            "Currency history error");
}

void mmMainCurrencyDialog::OnHistoryDeleteUnused(wxCommandEvent& /*event*/)
{
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
        valueTextBox_->SetValue(wxString::Format(wxT("%f"), histData->CURRVALUE));
    }
}

void mmMainCurrencyDialog::OnHistoryDeselected(wxListEvent& event)
{
    valueDatePicker_->SetValue(wxDateTime::Today());
    valueTextBox_->SetValue(wxEmptyString);
}

bool mmMainCurrencyDialog::SetBaseCurrency(int& baseCurrencyID)
{
    int baseCurrencyOLD = Model_Infotable::instance().GetBaseCurrencyId();
    if (baseCurrencyID == baseCurrencyOLD)
        return true;

    Model_Infotable::instance().SetBaseCurrency(baseCurrencyID);

    Model_Currency::Data* BaseCurrency = Model_Currency::instance().get(baseCurrencyID);
    BaseCurrency->BASECONVRATE = 1;
    Model_Currency::instance().save(BaseCurrency);

    Model_Currency::Data* BaseCurrencyOLD = Model_Currency::instance().get(baseCurrencyOLD);
    BaseCurrencyOLD->BASECONVRATE = 0;
    Model_Currency::instance().save(BaseCurrencyOLD);

    Model_CurrencyHistory::instance().Savepoint();
    for (const auto& r : Model_CurrencyHistory::instance().find(Model_CurrencyHistory::CURRENCYID(baseCurrencyID)))
        Model_CurrencyHistory::instance().remove(r.id());
    Model_CurrencyHistory::instance().ReleaseSavepoint();

    if (wxMessageBox(_("Do you want to update today currency rates?")
            , _("Currency Dialog")
            , wxYES_NO | wxYES_DEFAULT | wxICON_ERROR) != wxYES)
        return true;
    onlineUpdateCurRate();

    if (wxMessageBox(_("Do you want to update all currency history? Any custom currency rate will be deleted!")
            , _("Currency Dialog")
            , wxYES_NO | wxYES_DEFAULT | wxICON_ERROR) != wxYES)
        return true;

    Model_CurrencyHistory::instance().Savepoint();
    for (const auto& r : Model_CurrencyHistory::instance().all())
        Model_CurrencyHistory::instance().remove(r.id());
    Model_CurrencyHistory::instance().ReleaseSavepoint();
    onlineUpdateCurRate();

    if (_BceCurrencyHistoryRatesList.empty())
    {
        if (!mmMainCurrencyDialog::HistoryDownloadBce())
            return false;
    }

    std::vector<CurrencyHistoryRate> CurrencyHistoryRatesList;
    if (BaseCurrency->CURRENCY_SYMBOL == "EUR")
        CurrencyHistoryRatesList = _BceCurrencyHistoryRatesList;
    else if (!ConvertHistoryRates(_BceCurrencyHistoryRatesList, CurrencyHistoryRatesList, BaseCurrency->CURRENCY_SYMBOL))
        return false;

    Model_CurrencyHistory::instance().Savepoint();
    auto currencies = Model_Currency::instance().all();
    for (auto &currency : currencies)
    {
        if (Model_Account::is_used(currency) && currency.CURRENCY_SYMBOL != BaseCurrency->CURRENCY_SYMBOL)
        {
            bool UpdStatus = false;
            std::map<wxDateTime, int> DatesList = Model_Currency::DateUsed(currency.CURRENCYID);
            for (auto &CurrencyHistory : CurrencyHistoryRatesList)
            {
                if (CurrencyHistory.Currency == currency.CURRENCY_SYMBOL.Upper())
                {
                    UpdStatus = true;
                    if (DatesList.find(CurrencyHistory.Date) == DatesList.end())
                        continue;
                    Model_CurrencyHistory::instance().addUpdate(currency.CURRENCYID, CurrencyHistory.Date, CurrencyHistory.Rate, Model_CurrencyHistory::ONLINE);
                }
            }
            if (!UpdStatus)
                mmErrorDialogs::MessageError(this,
                wxString::Format("Unable to download history for symbol %S. History rates not available!", currency.CURRENCY_SYMBOL.Upper()),
                "Currency history error");
        }
    }
    Model_CurrencyHistory::instance().ReleaseSavepoint();

    fillControls();
    ShowCurrencyHistory();
    return true;
}

bool mmMainCurrencyDialog::HistoryDownloadBce()
{
    wxString XmlContent;
    if (site_content(mmex::weblink::BceCurrencyHistory, XmlContent) != wxURL_NOERR)
        return false;

    wxStringInputStream XmlContentStream(XmlContent);
    wxXmlDocument XmlDocument;
    if (!XmlDocument.Load(XmlContentStream))
        return false;
    if (XmlDocument.GetRoot()->GetName() != "gesmes:Envelope")
        return false;

    wxXmlNode* XmlRoot = XmlDocument.GetRoot()->GetChildren();
    while (XmlRoot->GetName() != "Cube")
        XmlRoot = XmlRoot->GetNext();
    if (XmlRoot->GetName() != "Cube")
        return false;

    double Rate;
    wxDateTime HistoryDate;
    XmlRoot = XmlRoot->GetChildren(); //Go inside <Cube>
    while (XmlRoot) //<Cube time="2015-07-03">
    {
        HistoryDate.ParseDate(XmlRoot->GetAttribute("time"));
        CurrencyHistoryRate CurrencyHistory;
        wxXmlNode* XmlRate = XmlRoot->GetChildren();
        while (XmlRate) //<Cube currency="USD" rate="1.1096"/>
        {
            CurrencyHistory.BaseCurrency = "EUR";
            CurrencyHistory.Date = HistoryDate;
            CurrencyHistory.Currency = XmlRate->GetAttribute("currency");
            XmlRate->GetAttribute("rate").ToDouble(&Rate);
            CurrencyHistory.Rate = Rate;

            _BceCurrencyHistoryRatesList.push_back(CurrencyHistory);
            XmlRate = XmlRate->GetNext();
        }
        XmlRoot = XmlRoot->GetNext();
    }

    if (_BceCurrencyHistoryRatesList.size() == 0)
        return false;

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
        mmErrorDialogs::MessageError(this,
            wxString::Format("Unable to download history for base symbol %S. History rates not available!", BaseCurrencySymbol),
            "Currency history error");
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