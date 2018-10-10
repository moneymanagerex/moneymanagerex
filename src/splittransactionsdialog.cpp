/*******************************************************
Copyright (C) 2006-2012 Madhan Kanagavel
Copyright (C) 2015 Nikolay

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

#include "splittransactionsdialog.h"
#include "splitdetailsdialog.h"
#include "constants.h"
#include "mmSimpleDialogs.h"
#include "util.h"

#include "Model_Account.h"
#include "Model_Category.h"

#include <wx/statline.h>

/*!
 * SplitTransactionDialog type definition
 */

wxIMPLEMENT_DYNAMIC_CLASS(SplitTransactionDialog, wxDialog);

/*!
 * SplitTransactionDialog event table definition
 */

 wxBEGIN_EVENT_TABLE(SplitTransactionDialog, wxDialog)
     EVT_BUTTON(wxID_ADD, SplitTransactionDialog::OnButtonAddClick)
     EVT_BUTTON(wxID_REMOVE, SplitTransactionDialog::OnButtonRemoveClick)
     EVT_BUTTON(wxID_EDIT, SplitTransactionDialog::OnButtonEditClick)
     EVT_BUTTON(wxID_OK, SplitTransactionDialog::OnOk)
     EVT_DATAVIEW_ITEM_ACTIVATED(wxID_ANY, SplitTransactionDialog::OnListDblClick)
     EVT_DATAVIEW_SELECTION_CHANGED(wxID_ANY, SplitTransactionDialog::OnListItemSelected)
     //EVT_DATAVIEW_ITEM_CONTEXT_MENU(wxID_ANY, SplitTransactionDialog::OnItemRightClick)
 wxEND_EVENT_TABLE()

SplitTransactionDialog::SplitTransactionDialog( )
{
}

 SplitTransactionDialog::SplitTransactionDialog(wxWindow* parent
    , std::vector<Split>& split
    , int transType
    , int accountID
    , double totalAmount
    , const wxString& name
    )
    : m_splits(split)
    , transType_(transType)
    , accountID_(accountID)
    , totalAmount_(totalAmount)
    , items_changed_(false)
{
    for (const auto &item : m_splits)
        m_local_splits.push_back(item);

    selectedIndex_ = -1;

    long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, _("Split Transaction Dialog")
        , wxDefaultPosition, wxSize(400, 400), style, name);
}

bool SplitTransactionDialog::Create(wxWindow* parent
    , wxWindowID id
    , const wxString& caption
    , const wxPoint& pos
    , const wxSize& size
    , long style
    , const wxString& name
    )
{
    lcSplit_ = nullptr;
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style, name);

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();

    DataToControls();

    return TRUE;
}

void SplitTransactionDialog::DataToControls()
{
    Model_Currency::Data *currency = Model_Currency::GetBaseCurrency();
    Model_Account::Data *account = Model_Account::instance().get(accountID_);
    if (account) { currency = Model_Account::currency(account); }

    lcSplit_->DeleteAllItems();
    for (const auto& entry : this->m_local_splits)
    {
        wxVector<wxVariant> data;
        data.push_back(wxVariant(Model_Category::full_name(entry.CATEGID, entry.SUBCATEGID)));
        data.push_back(wxVariant(Model_Currency::toString(entry.SPLITTRANSAMOUNT, currency)));
        lcSplit_->AppendItem(data, (wxUIntPtr)lcSplit_->GetItemCount());
    }
    if (lcSplit_->GetItemCount() > selectedIndex_ && selectedIndex_ >= 0)
        lcSplit_->SelectRow(selectedIndex_);
    UpdateSplitTotal();
    SetDisplayEditDeleteButtons();
    itemButtonNew_->SetFocus();
}

void SplitTransactionDialog::CreateControls()
{
    wxBoxSizer* dialogMainSizerV = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(dialogMainSizerV);

    wxStaticText* headingText = new wxStaticText(this, wxID_STATIC, _("Split Category Details"));
    dialogMainSizerV->Add(headingText, g_flagsV);

    wxBoxSizer* listCtrlSizer = new wxBoxSizer(wxHORIZONTAL);
    dialogMainSizerV->Add(listCtrlSizer, g_flagsExpand);

    lcSplit_ = new wxDataViewListCtrl(this, wxID_ANY
        , wxDefaultPosition, wxSize(265, 150));
    lcSplit_->AppendTextColumn(_("Category"), wxDATAVIEW_CELL_INERT, 180);
    lcSplit_->AppendTextColumn(_("Amount"), wxDATAVIEW_CELL_INERT, 180);

    listCtrlSizer->Add(lcSplit_, g_flagsExpand);

    wxBoxSizer* totalAmountSizer = new wxBoxSizer(wxHORIZONTAL);
    const wxString& totalMessage = _("Total:");

    wxStaticText* transAmountText = new wxStaticText(this, wxID_STATIC, totalMessage);
    transAmount_ = new wxStaticText(this, wxID_STATIC, wxEmptyString);
    totalAmountSizer->Add(transAmountText, g_flagsH);
    totalAmountSizer->Add(transAmount_, g_flagsH);
    dialogMainSizerV->Add(totalAmountSizer, g_flagsV);

    wxPanel* buttons_panel = new wxPanel(this, wxID_ANY);
    dialogMainSizerV->Add(buttons_panel, wxSizerFlags(g_flagsV).Center().Border(wxALL, 5));
    wxStdDialogButtonSizer*  buttons_sizer = new wxStdDialogButtonSizer;
    buttons_panel->SetSizer(buttons_sizer);

    wxSizerFlags flagsH = wxSizerFlags(g_flagsH).Border(wxLEFT | wxRIGHT | wxBOTTOM, 5).Center();
	wxSizerFlags flagsV = wxSizerFlags(g_flagsV).Border(wxLEFT | wxRIGHT | wxBOTTOM, 5).Center();
	wxBoxSizer* mainButtonSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* topRowButtonSizer = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* bottomRowButtonSizer = new wxBoxSizer(wxHORIZONTAL);
    mainButtonSizer->Add(topRowButtonSizer, flagsV);
    mainButtonSizer->Add(bottomRowButtonSizer, flagsV);
    buttons_sizer->Add(mainButtonSizer);

    itemButtonNew_ = new wxButton(buttons_panel, wxID_ADD, _("&Add "));
    itemButtonEdit_ = new wxButton(buttons_panel, wxID_EDIT, _("&Edit "));
    itemButtonDelete_ = new wxButton(buttons_panel, wxID_REMOVE, _("&Remove "));
    topRowButtonSizer->Add(itemButtonNew_, flagsH);
    topRowButtonSizer->Add(itemButtonEdit_, flagsH);
    topRowButtonSizer->Add(itemButtonDelete_, flagsH);

    itemButtonOK_ = new wxButton(buttons_panel, wxID_OK, _("&OK "));
    wxButton* itemButtonCancel = new wxButton(buttons_panel, wxID_CANCEL, wxGetTranslation(g_CancelLabel));
    itemButtonCancel->SetFocus();

    bottomRowButtonSizer->Add(itemButtonOK_, g_flagsH);
    bottomRowButtonSizer->Add(itemButtonCancel, g_flagsH);
}

void SplitTransactionDialog::OnButtonAddClick( wxCommandEvent& WXUNUSED(event) )
{
    double amount = totalAmount_ - Model_Splittransaction::get_total(m_local_splits);
    if (amount < 0.0) amount = 0.0;
    Split split = { -1, -1, amount };
    SplitDetailDialog sdd(this, split, transType_, accountID_);
    if (sdd.ShowModal() == wxID_OK)
    {
        this->m_local_splits.push_back(sdd.getResult());
        items_changed_ = true;
    }
    DataToControls();
}

void SplitTransactionDialog::OnButtonEditClick( wxCommandEvent& WXUNUSED(event) )
{
    EditEntry(selectedIndex_);
    DataToControls();
}

void SplitTransactionDialog::OnOk( wxCommandEvent& WXUNUSED(event) )
{
    //Check total amount - should be positive
    double total = 0;
    for (const auto& entry : m_local_splits)
        total += entry.SPLITTRANSAMOUNT;
    if (total < 0)
    {
        mmErrorDialogs::MessageError(this, _("Invalid Total Amount"), _("Error"));
    }
    else
    {
        // finally 
        m_splits.swap(m_local_splits);
        EndModal(wxID_OK);
    }
}

void SplitTransactionDialog::OnButtonRemoveClick(wxCommandEvent& WXUNUSED(event))
{
    if (selectedIndex_ < 0 || selectedIndex_ >= (int)this->m_local_splits.size())
        return;
    this->m_local_splits.erase(this->m_local_splits.begin() + selectedIndex_);
    selectedIndex_ = -1;
    items_changed_ = true;
    DataToControls();
}

void SplitTransactionDialog::UpdateSplitTotal()
{
    Model_Account::Data *account = Model_Account::instance().get(accountID_);
    const auto curr = account ? Model_Account::currency(account) : Model_Currency::GetBaseCurrency();
    double total = 0;
    for (const auto& entry : m_local_splits)
        total += entry.SPLITTRANSAMOUNT;
    transAmount_->SetLabelText(Model_Currency::toCurrency(total, curr));
}

void SplitTransactionDialog::EditEntry(int index)
{
    if (index < 0 || index >= (int)this->m_local_splits.size()) return;
    SplitDetailDialog sdd(this, m_local_splits[index], transType_, accountID_);
    if (sdd.ShowModal() == wxID_OK)
    {
        m_local_splits[index] = sdd.getResult();
        items_changed_ = true;
        DataToControls();
        UpdateSplitTotal();
    }
}

void SplitTransactionDialog::OnListItemSelected(wxDataViewEvent& event)
{
    wxDataViewItem item = event.GetItem();
    selectedIndex_ = lcSplit_->ItemToRow(item);
    SetDisplayEditDeleteButtons();
}

void SplitTransactionDialog::OnListDblClick(wxDataViewEvent& WXUNUSED(event))
{
    if (itemButtonEdit_->IsShown()) EditEntry(selectedIndex_);
}

void SplitTransactionDialog::SetDisplaySplitCategories()
{
    itemButtonNew_->Hide();
    itemButtonEdit_->Hide();
    itemButtonDelete_->Hide();
    itemButtonOK_->Hide();
}

void SplitTransactionDialog::SetDisplayEditDeleteButtons()
{
    bool active = selectedIndex_ >= 0 && selectedIndex_ < (int)this->m_local_splits.size();
    itemButtonEdit_->Enable(active);
    itemButtonDelete_->Enable(active);
}
