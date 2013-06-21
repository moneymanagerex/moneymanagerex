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
#include "mmCurrencyFormatter.h"
#include "constants.h"
#include "util.h"
#include "defs.h"
#include "paths.h"
#include "validators.h"

IMPLEMENT_DYNAMIC_CLASS( mmMainCurrencyDialog, wxDialog )

BEGIN_EVENT_TABLE( mmMainCurrencyDialog, wxDialog )
    EVT_BUTTON(wxID_ADD, mmMainCurrencyDialog::OnBtnAdd)
    EVT_BUTTON(wxID_EDIT, mmMainCurrencyDialog::OnBtnEdit)
    EVT_BUTTON(wxID_SELECTALL, mmMainCurrencyDialog::OnBtnSelect)
    EVT_BUTTON(wxID_REMOVE, mmMainCurrencyDialog::OnBtnDelete)

    EVT_MENU_RANGE(0, 1, mmMainCurrencyDialog::OnMenuSelected)

    EVT_DATAVIEW_ITEM_ACTIVATED(wxID_ANY,   mmMainCurrencyDialog::OnListItemActivated)
    EVT_DATAVIEW_SELECTION_CHANGED(wxID_ANY,    mmMainCurrencyDialog::OnListItemSelected)
    EVT_DATAVIEW_ITEM_VALUE_CHANGED(wxID_ANY,    mmMainCurrencyDialog::OnValueChanged)
    EVT_DATAVIEW_ITEM_CONTEXT_MENU(wxID_ANY, mmMainCurrencyDialog::OnItemRightClick)
END_EVENT_TABLE()

mmMainCurrencyDialog::mmMainCurrencyDialog(
    mmCoreDB* core,
    wxWindow* parent,
    bool bEnableSelect,
    wxWindowID id,
    const wxString& caption,
    const wxPoint& pos,
    const wxSize& size,
    long style
) : currencyID_(-1),
    core_(core),
    currencyListBox_(),
    bEnableSelect_(bEnableSelect)
{
    ColName_[CURR_BASE]   = " ";
    ColName_[CURR_SYMBOL] = _("Symbol");
    ColName_[CURR_NAME]   = _("Name");
    ColName_[BASE_RATE]   = _("Base Rate");

    currencyID_ = core_->currencyList_.GetBaseCurrencySettings();

    Create(parent, id, caption, pos, size, style);
}

bool mmMainCurrencyDialog::Create(  wxWindow* parent, wxWindowID id,
                      const wxString& caption,
                      const wxPoint& pos,
                      const wxSize& size,
                      long style )
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    //GetSizer()->Fit(this);
    //GetSizer()->SetSizeHints(this);

    SetIcon(mmex::getProgramIcon());

    fillControls();

    Centre();

    return TRUE;
}


void mmMainCurrencyDialog::fillControls()
{
    if (!core_) return;

    currencyListBox_->DeleteAllItems();
    int baseCurrencyID = core_->currencyList_.GetBaseCurrencySettings();

    wxVector<wxVariant> data;
    for (const auto& currency: core_->currencyList_.currencies_)
    {
        int currencyID = currency->currencyID_;

        data.clear();
        data.push_back( wxVariant(baseCurrencyID == currencyID) );
        data.push_back( wxVariant(currency->currencySymbol_) );
        data.push_back( wxVariant(currency->currencyName_) );
        data.push_back( wxVariant(wxString()<<currency->baseConv_) );
        currencyListBox_->AppendItem(data, (wxUIntPtr)currencyID);
    }
}

void mmMainCurrencyDialog::CreateControls()
{
    wxSizerFlags flags, flagsExpand;
    flags.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxALL, 5);
    flagsExpand.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxALL, 5).Expand();

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer22, flagsExpand);

    wxBitmapButton* update_button = new wxBitmapButton(this,
        wxID_STATIC, wxBitmap(checkupdate_xpm));
    itemBoxSizer22->Add(update_button, flags);
    update_button->Connect(wxID_STATIC, wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(mmMainCurrencyDialog::OnOnlineUpdateCurRate), NULL, this);
    update_button->SetToolTip(_("Online update currency rate"));
    itemBoxSizer22->AddSpacer(10);

    itemBoxSizer22->Add(new wxStaticText( this, wxID_STATIC,
       _("Currency List")), flags);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 1, wxGROW|wxALL, 5);

    //TODO:provide proper style and fix validator (does not working)
    currencyListBox_ = new wxDataViewListCtrl( this
        , wxID_ANY, wxDefaultPosition, wxSize(-1, 200), -1, mmDoubleValidator4());

    currencyListBox_ ->AppendToggleColumn( ColName_[CURR_BASE], wxDATAVIEW_CELL_INERT, 30 );
    currencyListBox_ ->AppendTextColumn( ColName_[CURR_SYMBOL], wxDATAVIEW_CELL_INERT, 60);
    currencyListBox_ ->AppendTextColumn( ColName_[CURR_NAME], wxDATAVIEW_CELL_INERT, 170);
    currencyListBox_ ->AppendTextColumn( ColName_[BASE_RATE], wxDATAVIEW_CELL_EDITABLE, 60 );

    itemBoxSizer3->Add(currencyListBox_, 1, wxGROW|wxALL, 1);

    wxPanel* itemPanel5 = new wxPanel( this, ID_PANEL10,
        wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(itemPanel5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel5->SetSizer(itemBoxSizer6);

    wxButton* itemButton7 = new wxButton( itemPanel5, wxID_ADD);
    itemBoxSizer6->Add(itemButton7, flags);

    itemButtonEdit_ = new wxButton( itemPanel5, wxID_EDIT );
    itemBoxSizer6->Add(itemButtonEdit_, flags);
    itemButtonEdit_->Disable();

    itemButtonDelete_ = new wxButton( itemPanel5, wxID_REMOVE );
    itemBoxSizer6->Add(itemButtonDelete_, flags);
    itemButtonDelete_->Disable();

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer9, flagsExpand);

    wxButton* itemButtonSelect = new wxButton( this, wxID_SELECTALL, _("&Select"));
    itemBoxSizer9->Add(itemButtonSelect,  4, wxALIGN_CENTER_VERTICAL|wxALL, 4);
    //itemButtonSelect->SetToolTip(_("Select the currently selected currency as the selected currency for the account"));

    if(bEnableSelect_ == false) {
        itemButtonSelect->Disable();
    }

    //Some interfaces has no any close buttons, it may confuse user. Cancel button added
    wxButton* itemCancelButton = new wxButton( this, wxID_CANCEL, _("&Cancel"));
    itemBoxSizer9->Add(itemCancelButton,  flags);
    itemCancelButton->SetFocus();

    this->SetMinSize(wxSize(350,450));
    this->Fit();
}

void mmMainCurrencyDialog::OnBtnAdd(wxCommandEvent& /*event*/)
{
    wxArrayString currency_names;
    wxArrayString currency_symbols;
    for(size_t i = 0; i < sizeof(CURRENCIES)/sizeof(wxString); ++i)
    {
       currency_symbols.Add(CURRENCIES[i]);
       currency_names.Add(CURRENCIES[++i]);
    }
    wxString currText = wxGetSingleChoice (_("Name of Currency to Add")
        , _("Add Currency"), currency_names);
    if (!currText.IsEmpty())
    {
        wxString currency_symbol = currency_symbols[currency_names.Index(currText)];
        int currID = core_->currencyList_.getCurrencyID(currText);
        if (currID == -1)
        {
            mmCurrency* pCurrency(new mmCurrency());
            pCurrency->currencyName_ = currText;
            pCurrency->currencySymbol_ = currency_symbol;
            currencyID_ = core_->currencyList_.AddCurrency(pCurrency);

            if (currencyID_ > 0)
            {
                mmCurrencyDialog* dlg = new mmCurrencyDialog(core_, currencyID_, this);
                dlg->ShowModal();
            }
            else
            {
                mmShowErrorMessage(this, _("Error"), _("Error"));
            }
        }
        else
        {
            wxMessageBox(_("Attempt to Add a currency which already exists!")
                , _("Currency Dialog"), wxOK|wxICON_ERROR);
        }
    }
    fillControls();

}

void mmMainCurrencyDialog::OnBtnEdit(wxCommandEvent& /*event*/)
{
    mmCurrencyDialog(core_, currencyID_, this).ShowModal();
    fillControls();
}

void mmMainCurrencyDialog::OnBtnSelect(wxCommandEvent& /*event*/)
{
    if (selectedIndex_ > -1) EndModal(wxID_OK);
}

void mmMainCurrencyDialog::OnBtnDelete(wxCommandEvent& /*event*/)
{
    if (selectedIndex_ < 0) return;

//    int baseCurrencyID = core_->currencyList_.getBaseCurrencySettings();
    int baseCurrencyID = core_->dbInfoSettings_->GetIntSetting("BASECURRENCYID", 1);
    bool usedAsBase = currencyID_ == baseCurrencyID;

    if (core_->accountList_.currencyInUse(currencyID_) || usedAsBase)
    {
        wxMessageBox(_("Attempt to delete a currency being used by an account\n or as the base currency.")
            ,_("Currency Dialog"), wxOK|wxICON_ERROR);
    }
    else
    {
        if (wxMessageBox(_("Do you really want to delete the selected Currency?"),
                         _("Currency Dialog"),wxICON_QUESTION|wxYES|wxNO) == wxYES)
        {
            core_->currencyList_.DeleteCurrency(currencyID_);
            fillControls();
        }
    }
}

bool mmMainCurrencyDialog::Execute(mmCoreDB* core, wxWindow* parent, int& currencyID)
{
    bool result = false;

    mmMainCurrencyDialog* dlg = new mmMainCurrencyDialog(core, parent);
    if (dlg->ShowModal() == wxID_OK)
    {
        currencyID = dlg->currencyID_;
        result = true;
    }
    dlg->Destroy();

    return result;
}

void mmMainCurrencyDialog::OnListItemSelected(wxDataViewEvent& event)
{
    wxDataViewItem item = event.GetItem();
    selectedIndex_ = currencyListBox_->ItemToRow(item);
    currencyID_ = (int)currencyListBox_->GetItemData(item);
    wxString currency_name = core_->currencyList_.getCurrencyName(currencyID_);
    mmCurrency* pCurrency = core_->currencyList_.getCurrencySharedPtr(currencyID_);
    curr_rate_ = pCurrency->baseConv_;

    wxLogDebug(wxString::Format("selected item:%i currency:%s", selectedIndex_, currency_name));

    itemButtonEdit_->Enable();
    if (!bEnableSelect_)    // prevent user deleting currencies when editing accounts.
        itemButtonDelete_->Enable(!core_->accountList_.currencyInUse(currencyID_));
}

void mmMainCurrencyDialog::OnListItemActivated(wxDataViewEvent& event)
{
    wxDataViewItem item = event.GetItem();
    selectedIndex_ = currencyListBox_->ItemToRow(item);
    wxString currency_name = core_->currencyList_.getCurrencyName(currencyID_);
    wxLogDebug(wxString::Format("activated item:%i currency:%s", selectedIndex_, currency_name));

    wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, wxID_ANY);

    if (bEnableSelect_)
        OnBtnSelect(evt);
    else
        OnBtnEdit(evt);
}

void mmMainCurrencyDialog::OnValueChanged(wxDataViewEvent& event)
{
    int col = event.GetColumn();
    wxDataViewItem item = event.GetItem();
    int row = currencyListBox_->ItemToRow(item);
    wxVariant var;
    currencyListBox_->GetValue(var, row, col);
    wxString val = var.GetString();

    wxLogDebug(wxString::Format("col:%i row:%i value:%s", col, row, val));

    double convRate;
    if (val.ToDouble(&convRate))
    {
        mmCurrency* pCurrency = core_->currencyList_.getCurrencySharedPtr(currencyID_);
        pCurrency->baseConv_ = convRate;
        core_->currencyList_.UpdateCurrency(pCurrency);
    }
    else
    {
        wxString value = CurrencyFormatter::float2String(curr_rate_);
        currencyListBox_->SetValue(wxVariant(value), row, BASE_RATE);
    }

}

void mmMainCurrencyDialog::OnOnlineUpdateCurRate(wxCommandEvent& /*event*/)
{
    wxString sMsg = "";
    if (core_->currencyList_.OnlineUpdateCurRate(sMsg))
    {
        wxMessageDialog msgDlg(this, sMsg, _("Currency rate updated"));
        msgDlg.ShowModal();
        fillControls();
    }
    else
    {
        wxMessageDialog msgDlg(this, sMsg, _("Error"), wxOK|wxICON_ERROR);
        msgDlg.ShowModal();
    }
}

void mmMainCurrencyDialog::OnMenuSelected(wxCommandEvent& event)
{
    int baseCurrencyID = core_->dbInfoSettings_->GetIntSetting("BASECURRENCYID", -1);

    if (baseCurrencyID == currencyID_) return;

    core_->dbInfoSettings_->SetIntSetting("BASECURRENCYID", currencyID_);
    core_->dbInfoSettings_->Save();

    fillControls();
    event.Skip();
}

void mmMainCurrencyDialog::OnItemRightClick(wxDataViewEvent& event)
{
    wxCommandEvent ev(wxEVT_COMMAND_MENU_SELECTED, wxID_ANY) ;
    ev.SetEventObject( this );

    wxMenu* mainMenu = new wxMenu;
    mainMenu->Append(new wxMenuItem(mainMenu, 0, _("Set as Base Currency")));

    PopupMenu(mainMenu);
    event.Skip();
}
