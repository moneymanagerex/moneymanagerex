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

#include "maincurrencydialog.h"
#include "currencydialog.h"
#include "constants.h"
#include "util.h"
#include "defs.h"
#include "paths.h"
#include "mmCalculator.h"
#include "validators.h"
#include "model/Model_Account.h"
#include "model/Model_Currency.h"
#include "model/Model_Infotable.h"
#include "model/Model_Setting.h"

#include "../resources/checkupdate.xpm"

IMPLEMENT_DYNAMIC_CLASS(mmMainCurrencyDialog, wxDialog)

BEGIN_EVENT_TABLE(mmMainCurrencyDialog, wxDialog)
    EVT_BUTTON(wxID_ADD, mmMainCurrencyDialog::OnBtnAdd)
    EVT_BUTTON(wxID_EDIT, mmMainCurrencyDialog::OnBtnEdit)
    EVT_BUTTON(wxID_SELECTALL, mmMainCurrencyDialog::OnBtnSelect)
    EVT_BUTTON(wxID_REMOVE, mmMainCurrencyDialog::OnBtnDelete)
    EVT_BUTTON(wxID_CANCEL, mmMainCurrencyDialog::OnCancel)

    EVT_MENU_RANGE(MENU_ITEM1, MENU_ITEM2, mmMainCurrencyDialog::OnMenuSelected)

    EVT_DATAVIEW_ITEM_ACTIVATED(wxID_ANY, mmMainCurrencyDialog::OnListItemActivated)
    EVT_DATAVIEW_SELECTION_CHANGED(wxID_ANY, mmMainCurrencyDialog::OnListItemSelected)
    EVT_DATAVIEW_ITEM_VALUE_CHANGED(wxID_ANY, mmMainCurrencyDialog::OnValueChanged)
    EVT_DATAVIEW_ITEM_CONTEXT_MENU(wxID_ANY, mmMainCurrencyDialog::OnItemRightClick)
END_EVENT_TABLE()

mmMainCurrencyDialog::mmMainCurrencyDialog(
    wxWindow* parent
    , bool bEnableSelect
) : currencyListBox_(),
    bEnableSelect_(bEnableSelect)
{
    ColName_[CURR_BASE]   = " ";
    ColName_[CURR_SYMBOL] = _("Symbol");
    ColName_[CURR_NAME]   = _("Name");
    ColName_[BASE_RATE]   = _("Base Rate");

    currencyID_ = Model_Infotable::instance().GetBaseCurrencyId();
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
        data.push_back(wxVariant(wxString()<<currency.BASECONVRATE));
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
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer22, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxBitmapButton* update_button = new wxBitmapButton(this
        , wxID_STATIC, wxBitmap(checkupdate_xpm));
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
    currencyListBox_->AppendTextColumn(ColName_[CURR_SYMBOL], wxDATAVIEW_CELL_INERT, 60);
    currencyListBox_->AppendTextColumn(ColName_[CURR_NAME], wxDATAVIEW_CELL_INERT, 170);
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

    if (bEnableSelect_ == false) {
        itemButtonSelect->Disable();
    }

    //Some interfaces has no any close buttons, it may confuse user. Cancel button added
    wxButton* itemCancelButton = new wxButton(buttonsPanel, wxID_CANCEL, _("&Cancel "));
    itemBoxSizer9->Add(itemCancelButton, g_flags);
    itemCancelButton->SetFocus();

    this->SetMinSize(wxSize(350,450));
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

    int baseCurrencyID = Model_Infotable::instance().GetBaseCurrencyId();

    Model_Currency::Data* currency = Model_Currency::instance().get(currencyID_);
    if (!currency) return;
    if (Model_Account::is_used(currency) || currencyID_ == baseCurrencyID)
    {
        wxMessageBox(_("Attempt to delete a currency being used by an account\n or as the base currency.")
            ,_("Currency Dialog"), wxOK|wxICON_ERROR);
    }
    else
    {
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
}

bool mmMainCurrencyDialog::Execute(wxWindow* parent, int& currencyID)
{
    bool result = false;

    mmMainCurrencyDialog dlg(parent);
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
    if (selectedIndex_ >= 0)
    {
        currencyID_ = (int)currencyListBox_->GetItemData(item);
        Model_Currency::Data* currency = Model_Currency::instance().get(currencyID_);
        if (currency)
            curr_rate_ = currency->BASECONVRATE;    
        itemButtonEdit_->Enable();
    }
    if (!bEnableSelect_)    // prevent user deleting currencies when editing accounts.
    {
        Model_Currency::Data* currency = Model_Currency::instance().get(currencyID_);
        if (currency)
            itemButtonDelete_->Enable(!Model_Account::is_used(currency));
    }
}

void mmMainCurrencyDialog::OnListItemActivated(wxDataViewEvent& /* event */)
{
    wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, wxID_ANY);

    if (bEnableSelect_)
        OnBtnSelect(evt);
    else
        OnBtnEdit(evt);
}

void mmMainCurrencyDialog::OnValueChanged(wxDataViewEvent& event)
{
    int row = currencyListBox_->ItemToRow(event.GetItem());
    wxVariant var;
    currencyListBox_->GetValue(var, row, event.GetColumn());
    wxString value = var.GetString();
    Model_Currency::Data* currency = Model_Currency::instance().get(currencyID_);

    wxString calculated_amount = "";
    double conv_rate = curr_rate_;
    mmCalculator calc;
    if (calc.is_ok(value))
    {
        calculated_amount = wxString::Format("%.4f", calc.get_result());
        if (value != calculated_amount)
            currencyListBox_->SetValue(wxVariant(calculated_amount), row, BASE_RATE);
        calculated_amount.ToDouble(&conv_rate);

        if (currency)
        {
            currency->BASECONVRATE = conv_rate;
            Model_Currency::instance().save(currency);
        }
    }
    else
    {
        value = Model_Currency::toString(conv_rate, currency);
        currencyListBox_->SetValue(wxVariant(value), row, BASE_RATE);
    }

}

bool mmMainCurrencyDialog::onlineUpdateCurRate(int curr_id)
{
    wxString base_symbol = "";
    wxString msg = "";
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
        wxString buffer = "";
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

            wxRegEx pattern("\"(...)...=X\",([^,][0-9.]+),\"([^\"]*)\"");
            if (pattern.Matches(csvline))
            {
                CurrencySymbol = pattern.GetMatch(csvline, 1);
                pattern.GetMatch(csvline, 2).ToDouble(&dRate);
                sName = pattern.GetMatch(csvline, 3);
                currency_data[CurrencySymbol] = std::make_pair(dRate, sName);
            }
        }

        msg = _("Currency rate updated");
        msg << "\n\n";

        for (auto &currency : currencies)
        {
            if (!cbShowAll_->IsChecked() && !Model_Account::is_used(currency)) continue;

            const wxString currency_symbol = currency.CURRENCY_SYMBOL.Upper();
            if (!currency_symbol.IsEmpty())
            {
                if (currency_data.find(currency_symbol) != currency_data.end())
                {
                    msg << wxString::Format(_("%s\t: %0.4f -> %0.4f\n")
                        , currency_symbol, currency.BASECONVRATE, currency_data[currency_symbol].first);
                    currency.BASECONVRATE = currency_data[currency_symbol].first;
                    if (base_symbol == currency_symbol)
                        currency.BASECONVRATE = 1;
                }
                else
                {
                    if (curr_id < 0)
                        msg << wxString::Format(_("%s\t: %s\n"), currency_symbol, _("Invalid Value "));
                }
            }
        }

        Model_Currency::instance().save(currencies);

        wxMessageDialog msgDlg(this, msg, _("Currency rate updated"));
        msgDlg.ShowModal();
        fillControls();
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
    onlineUpdateCurRate();
}

void mmMainCurrencyDialog::OnMenuSelected(wxCommandEvent& event)
{
    switch (event.GetId())
    {
    case MENU_ITEM1:
    {
        int baseCurrencyID = Model_Infotable::instance().GetBaseCurrencyId();
        if (baseCurrencyID != currencyID_)
        {
            Model_Infotable::instance().SetBaseCurrencyID(currencyID_);
            fillControls();
        }
    }
    case MENU_ITEM2:
        onlineUpdateCurRate(currencyID_);

    }
}

void mmMainCurrencyDialog::OnItemRightClick(wxDataViewEvent& event)
{
    wxCommandEvent ev(wxEVT_COMMAND_MENU_SELECTED, wxID_ANY) ;
    ev.SetEventObject( this );

    wxMenu* mainMenu = new wxMenu;
    mainMenu->Append(new wxMenuItem(mainMenu, MENU_ITEM1, _("Set as Base Currency")));
    mainMenu->Append(new wxMenuItem(mainMenu, MENU_ITEM2, _("Online Update Currency Rate")));

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
