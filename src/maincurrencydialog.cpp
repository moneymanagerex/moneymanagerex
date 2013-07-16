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
#include "model/Model_Infotable.h"
#include "model/Model_Currency.h"

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

    currencyID_ = Model_Infotable::instance().GetBaseCurrencyId();

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
    int baseCurrencyID = Model_Infotable::instance().GetBaseCurrencyId();

    for (const auto& currency: Model_Currency::instance().all())
    {
        int currencyID = currency.CURRENCYID;

        wxVector<wxVariant> data;
        data.push_back(wxVariant(baseCurrencyID == currencyID));
        data.push_back(wxVariant(currency.CURRENCY_SYMBOL));
        data.push_back(wxVariant(currency.CURRENCYNAME));
        data.push_back(wxVariant(wxString()<<currency.BASECONVRATE));
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
        , wxID_ANY, wxDefaultPosition, wxSize(-1, 200)/*, wxDV_HORIZ_RULES, mmDoubleValidator4()*/);

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
    mmCurrencyDialog dlg(core_, -1, this);
    dlg.ShowModal();
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
    int baseCurrencyID = Model_Infotable::instance().GetIntInfo("BASECURRENCYID", 1);
    bool usedAsBase = currencyID_ == baseCurrencyID;

    if (core_->accountList_.currencyInUse(currencyID_) || usedAsBase)
    {
        wxMessageBox(_("Attempt to delete a currency being used by an account\n or as the base currency.")
            ,_("Currency Dialog"), wxOK|wxICON_ERROR);
    }
    else
    {
        if (wxMessageBox(_("Do you really want to delete the selected Currency?")
                         , _("Currency Dialog")
                         , wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION) == wxYES)
        {
            core_->currencyList_.DeleteCurrency(currencyID_);
            fillControls();
        }
    }
}

bool mmMainCurrencyDialog::Execute(mmCoreDB* core, wxWindow* parent, int& currencyID)
{
    bool result = false;

    mmMainCurrencyDialog dlg(core, parent);
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
        wxString currency_name = core_->currencyList_.getCurrencyName(currencyID_);
        mmCurrency* pCurrency = core_->currencyList_.getCurrencySharedPtr(currencyID_);
        curr_rate_ = pCurrency->baseConv_;
        itemButtonEdit_->Enable();
    }
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
    int row = currencyListBox_->ItemToRow(event.GetItem());
    wxVariant var;
    currencyListBox_->GetValue(var, row, event.GetColumn());
    wxString value = var.GetString();

    wxString calculated_mount = "";
    double conv_rate = curr_rate_;
    if (mmCalculator(value, calculated_mount))
    {
        if (value != calculated_mount)
            currencyListBox_->SetValue(wxVariant(calculated_mount), row, BASE_RATE);
        calculated_mount.ToDouble(&conv_rate);
        mmCurrency* pCurrency = core_->currencyList_.getCurrencySharedPtr(currencyID_);
        pCurrency->baseConv_ = conv_rate;
        core_->currencyList_.UpdateCurrency(pCurrency);
    }
    else
    {
        value = CurrencyFormatter::float2String(conv_rate);
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
    int baseCurrencyID = Model_Infotable::instance().GetIntInfo("BASECURRENCYID", -1);
    if (baseCurrencyID != currencyID_)
    {
        Model_Infotable::instance().Set("BASECURRENCYID", currencyID_);
        fillControls();
    }
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
