/*******************************************************
Copyright (C) 2006-2012 Madhan Kanagavel

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

#include "constants.h"
#include "splittransactionsdialog.h"
#include "splitdetailsdialog.h"
#include "util.h"
#include <wx/statline.h>
#include "model/Model_Category.h"

/*!
 * SplitTransactionDialog type definition
 */

IMPLEMENT_DYNAMIC_CLASS( SplitTransactionDialog, wxDialog )

/*!
 * SplitTransactionDialog event table definition
 */

 BEGIN_EVENT_TABLE(SplitTransactionDialog, wxDialog)
     EVT_BUTTON(wxID_ADD, SplitTransactionDialog::OnButtonAddClick)
     EVT_BUTTON(wxID_REMOVE, SplitTransactionDialog::OnButtonRemoveClick)
     EVT_BUTTON(wxID_EDIT, SplitTransactionDialog::OnButtonEditClick)
     EVT_BUTTON(wxID_OK, SplitTransactionDialog::OnOk)
     EVT_DATAVIEW_ITEM_ACTIVATED(wxID_ANY, SplitTransactionDialog::OnListDblClick)
     EVT_DATAVIEW_SELECTION_CHANGED(wxID_ANY, SplitTransactionDialog::OnListItemSelected)
     //EVT_DATAVIEW_ITEM_CONTEXT_MENU(wxID_ANY, SplitTransactionDialog::OnItemRightClick)
 END_EVENT_TABLE()

SplitTransactionDialog::SplitTransactionDialog( )
{
}

SplitTransactionDialog::SplitTransactionDialog(
    Model_Splittransaction::Data_Set* splits 
    , wxWindow* parent
    , int transType
    , int accountID)
    : m_splits(splits)
    , accountID_(accountID)
{
    //std::copy(m_splits->begin(), m_splits->end(), m_local_splits.begin());
    for (const auto &item : *m_splits) m_local_splits.push_back(item);

    transType_ = transType;
    selectedIndex_ = 0;
    if (transType_ == DEF_TRANSFER)
        transType_ = DEF_WITHDRAWAL;

    long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX;
    Create(parent, wxID_ANY, _("Split Transaction Dialog")
        , wxDefaultPosition, wxSize(400, 300), style);
}

bool SplitTransactionDialog::Create(wxWindow* parent, wxWindowID id,
                                    const wxString& caption,
                                    const wxPoint& pos,
                                    const wxSize& size,
                                    long style)
{
    lcSplit_ = NULL;
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

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
    if (account) currency = Model_Account::currency(account);

    lcSplit_->DeleteAllItems();
    for (const auto & entry : this->m_local_splits)
    {
        const Model_Category::Data* category = Model_Category::instance().get(entry.CATEGID);
        const Model_Subcategory::Data* sub_category = (entry.SUBCATEGID != -1 ? Model_Subcategory::instance().get(entry.SUBCATEGID) : 0);

        wxVector<wxVariant> data;
        data.push_back(wxVariant(Model_Category::full_name(category, sub_category)));
        data.push_back(wxVariant(Model_Currency::toString(entry.SPLITTRANSAMOUNT, currency)));
        lcSplit_->AppendItem(data, (wxUIntPtr)entry.SPLITTRANSID);
        if (lcSplit_->GetItemCount()-1 == selectedIndex_) lcSplit_->SelectRow(selectedIndex_);
    }
    UpdateSplitTotal();
    itemButtonNew_->SetFocus();
}

void SplitTransactionDialog::CreateControls()
{
    wxBoxSizer* dialogMainSizerV = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(dialogMainSizerV);

    wxStaticText* headingText = new wxStaticText( this, wxID_STATIC, _(" Split Category Details"));
    dialogMainSizerV->Add(headingText, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxRIGHT, 10);

    wxBoxSizer* listCtrlSizer = new wxBoxSizer(wxHORIZONTAL);
    dialogMainSizerV->Add(listCtrlSizer, 1, wxGROW|wxALL, 5);

    lcSplit_ = new wxDataViewListCtrl(this, wxID_ANY, wxDefaultPosition
        , wxSize(265, 120));
    lcSplit_->AppendTextColumn(_("Category"), wxDATAVIEW_CELL_INERT, 180);
    lcSplit_->AppendTextColumn(_("Amount"), wxDATAVIEW_CELL_INERT, 180);

    listCtrlSizer->Add(lcSplit_, 1, wxGROW | wxALL, 5);

    wxBoxSizer* totalAmountSizer = new wxBoxSizer(wxHORIZONTAL);
    wxString totalMessage = _("Total:");
    if (transType_ == DEF_WITHDRAWAL)
    {
        totalMessage.Prepend(" ");
        totalMessage.Prepend(_("Withdrawal"));
    }
    if (transType_ == DEF_DEPOSIT)
    {
        totalMessage.Prepend(" ");
        totalMessage.Prepend(_("Deposit"));
    }
    wxStaticText* transAmountText_ = new wxStaticText( this, wxID_STATIC, totalMessage);
    transAmount_ = new wxStaticText( this, wxID_STATIC, wxEmptyString);
    totalAmountSizer->Add(transAmountText_, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    totalAmountSizer->Add(transAmount_, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxLEFT, 5);
    dialogMainSizerV->Add(totalAmountSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticLine* line = new wxStaticLine (this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    dialogMainSizerV->Add(line, 0, wxGROW|wxLEFT|wxTOP|wxRIGHT, 10);

    wxBoxSizer* mainButtonSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* topRowButtonSizer = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* bottomRowButtonSizer = new wxBoxSizer(wxHORIZONTAL);
    mainButtonSizer->Add(topRowButtonSizer, 0, wxALIGN_RIGHT|wxALL, 0);
    mainButtonSizer->Add(bottomRowButtonSizer, 0, wxALIGN_RIGHT|wxALL, 0);
    dialogMainSizerV->Add(mainButtonSizer, 0, wxALIGN_RIGHT|wxALL, 10);

    itemButtonNew_ = new wxButton( this, wxID_ADD, _("&Add "));
    itemButtonEdit_ = new wxButton( this, wxID_EDIT, _("&Edit "));
    itemButtonDelete_ = new wxButton( this, wxID_REMOVE, _("&Remove "));
    topRowButtonSizer->Add(itemButtonNew_, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxRIGHT, 5);
    topRowButtonSizer->Add(itemButtonEdit_, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxRIGHT, 5);
    topRowButtonSizer->Add(itemButtonDelete_, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxRIGHT, 0);

    itemButtonOK_ = new wxButton( this, wxID_OK, _("&OK "));
    wxButton* itemButtonCancel = new wxButton( this, wxID_CANCEL, _("&Cancel "));
    itemButtonCancel->SetFocus();

    bottomRowButtonSizer->Add(itemButtonOK_, 0, wxALIGN_RIGHT|wxTOP|wxRIGHT, 5);
    bottomRowButtonSizer->Add(itemButtonCancel, 0, wxALIGN_RIGHT|wxTOP, 5);
}

void SplitTransactionDialog::OnButtonAddClick( wxCommandEvent& /*event*/ )
{
    Model_Splittransaction::Data *split = Model_Splittransaction::instance().create();
    SplitDetailDialog sdd(this, split, transType_, accountID_);
    if (sdd.ShowModal() == wxID_OK)
    {
        this->m_local_splits.push_back(*split);
    }
    DataToControls();
}

void SplitTransactionDialog::OnButtonEditClick( wxCommandEvent& /*event*/ )
{
    EditEntry(selectedIndex_);
    DataToControls();
}

void SplitTransactionDialog::OnOk(wxCommandEvent& /*event*/)
{
    //Check total amount - should be positive
    if (Model_Splittransaction::instance().get_total(this->m_local_splits) < 0)
    {
        mmShowErrorMessage(this, _("Invalid Total Amount"), _("Error"));
    }
    else
    {        // finally 
        this->m_splits->swap(this->m_local_splits);
        EndModal(wxID_OK);

    }
}
void SplitTransactionDialog::OnButtonRemoveClick( wxCommandEvent& event )
{
    if (selectedIndex_ < 0 || selectedIndex_ >= (int)this->m_local_splits.size()) return;
    Model_Splittransaction::instance().remove(this->m_local_splits[selectedIndex_].SPLITTRANSID);
    this->m_local_splits.erase(this->m_local_splits.begin() + selectedIndex_);
    DataToControls();
}

bool SplitTransactionDialog::ShowToolTips()
{
    return TRUE;
}

wxBitmap SplitTransactionDialog::GetBitmapResource( const wxString& /*name*/ )
{
    return wxNullBitmap;
}

wxIcon SplitTransactionDialog::GetIconResource( const wxString& /*name*/ )
{
    return wxNullIcon;
}

void SplitTransactionDialog::UpdateSplitTotal()
{
    double total = Model_Splittransaction::instance().get_total(this->m_local_splits);
    transAmount_->SetLabel(Model_Currency::toCurrency(total));
}

void SplitTransactionDialog::EditEntry(int index)
{
    if (index < 0 || index >= (int)this->m_local_splits.size()) return;
    Model_Splittransaction::Data& split = this->m_local_splits[index];
    SplitDetailDialog sdd(this, &split, transType_, accountID_);
    if (sdd.ShowModal() == wxID_OK)
    {
        DataToControls();
        UpdateSplitTotal();
    }
}

void SplitTransactionDialog::OnListItemSelected(wxDataViewEvent& event)
{
    wxDataViewItem item = event.GetItem();
    selectedIndex_ = lcSplit_->ItemToRow(item);
}

void SplitTransactionDialog::OnListDblClick(wxDataViewEvent& event)
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

