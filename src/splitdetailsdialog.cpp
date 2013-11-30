/*******************************************************
Copyright (C) 2006-2012 Madhan Kanagavel
                        Modified by: Stefano Giorgio, Nikolay

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

#include "splitdetailsdialog.h"
#include "categdialog.h"
#include "util.h"
#include "constants.h"
#include "validators.h"
#include "mmCalculator.h"
#include "model/Model_Checking.h"
#include "model/Model_Category.h"
#include "model/Model_Subcategory.h"

#include <wx/valnum.h>

IMPLEMENT_DYNAMIC_CLASS( SplitDetailDialog, wxDialog )

enum
{
    ID_DIALOG_SPLTTRANS_TYPE = wxID_HIGHEST + 1,
    ID_BUTTONCATEGORY,
    ID_TEXTCTRLAMOUNT,
};

BEGIN_EVENT_TABLE( SplitDetailDialog, wxDialog )
    EVT_BUTTON( ID_BUTTONCATEGORY, SplitDetailDialog::OnButtonCategoryClick )
    EVT_BUTTON( wxID_OK, SplitDetailDialog::OnButtonOKClick )
    EVT_BUTTON( wxID_CANCEL, SplitDetailDialog::OnCancel )
    EVT_TEXT_ENTER( ID_TEXTCTRLAMOUNT, SplitDetailDialog::onTextEntered )
END_EVENT_TABLE()

SplitDetailDialog::SplitDetailDialog()
{
}

SplitDetailDialog::SplitDetailDialog( 
    wxWindow* parent
    , Model_Splittransaction::Data* split
    , int transType
    , int accountID)
    : split_(split)
    , accountID_(accountID)
{
    transType_ = transType;
    Create(parent);
}

bool SplitDetailDialog::Create(wxWindow* parent)
{
    long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX | wxWS_EX_BLOCK_EVENTS;
    wxDialog::Create(parent, wxID_ANY, _("Split Detail Dialog"), wxDefaultPosition, wxSize(400, 300), style);

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();
    DataToControls();

    return TRUE;
}

void SplitDetailDialog::DataToControls()
{
    const Model_Category::Data* category = Model_Category::instance().get(split_->CATEGID);
    const Model_Subcategory::Data* sub_category = Model_Subcategory::instance().get(split_->SUBCATEGID);
    const wxString category_name = Model_Category::full_name(category, sub_category);

    bCategory_->SetLabel(category_name);

    if (split_->SPLITTRANSAMOUNT)
        textAmount_->SetValue(fabs(split_->SPLITTRANSAMOUNT));
    textAmount_->SetFocus();
}

void SplitDetailDialog::CreateControls()
{
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer2);

    wxStaticBox* itemStaticBoxSizer4Static = new wxStaticBox(this, wxID_ANY, _("Split Transaction Details"));
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(itemStaticBoxSizer4Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxLEFT|wxTOP|wxRIGHT, 10);

    wxPanel* itemPanel7 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    itemStaticBoxSizer4->Add(itemPanel7, 0, wxGROW|wxALL, 10);

    wxFlexGridSizer* controlSizer = new wxFlexGridSizer(0, 2, 10, 10);
    itemPanel7->SetSizer(controlSizer);

    wxStaticText* staticTextType = new wxStaticText( itemPanel7, wxID_STATIC, _("Type"));
    controlSizer->Add(staticTextType, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 0);

    const wxString itemChoiceStrings[] =
    {
        _("Withdrawal"),
        _("Deposit"),
    };
    choiceType_ = new wxChoice(itemPanel7, ID_DIALOG_SPLTTRANS_TYPE
        , wxDefaultPosition, wxDefaultSize
        , 2
        , itemChoiceStrings);
    choiceType_->SetSelection(split_->SPLITTRANSAMOUNT < 0 ? !transType_ : transType_);
    choiceType_->SetToolTip(_("Specify the type of transactions to be created."));
    controlSizer->Add(choiceType_, 0, wxALIGN_CENTER_VERTICAL | wxALL | wxADJUST_MINSIZE, 0);

    wxStaticText* staticTextAmount = new wxStaticText( itemPanel7, wxID_STATIC, _("Amount"));
    controlSizer->Add(staticTextAmount, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 0);

    textAmount_ = new mmTextCtrl( itemPanel7, ID_TEXTCTRLAMOUNT, ""
        , wxDefaultPosition, wxSize(110,-1), wxALIGN_RIGHT|wxTE_PROCESS_ENTER
        , mmCalcValidator());
    controlSizer->Add(textAmount_, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxStaticText* staticTextCategory = new wxStaticText( itemPanel7, wxID_STATIC, _("Category"));
    controlSizer->Add(staticTextCategory, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 0);
    bCategory_ = new wxButton( itemPanel7, ID_BUTTONCATEGORY, ""
        , wxDefaultPosition, wxSize(200, -1), 0 );
    controlSizer->Add(bCategory_, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    /**************************************************************************
     Control Buttons
    ***************************************************************************/
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(buttonSizer, 0, wxALIGN_RIGHT|wxALL, 5);

    wxButton* itemButtonOK = new wxButton( this, wxID_OK, _("&OK "));
    buttonSizer->Add(itemButtonOK, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButtonCancel = new wxButton( this, wxID_CANCEL, _("&Cancel "));
    buttonSizer->Add(itemButtonCancel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTONCATEGORY
 */

void SplitDetailDialog::OnButtonCategoryClick( wxCommandEvent& /*event*/ )
{
    mmCategDialog dlg(this);
    dlg.setTreeSelection(split_->CATEGID, split_->SUBCATEGID);
    if ( dlg.ShowModal() == wxID_OK )
    {
        split_->CATEGID = dlg.getCategId();
        split_->SUBCATEGID = dlg.getSubCategId();

        bCategory_->SetLabel(dlg.getFullCategName());
    }
    DataToControls();
}

void SplitDetailDialog::onTextEntered(wxCommandEvent& WXUNUSED(event))
{
    
    Model_Currency::Data *currency = Model_Currency::GetBaseCurrency();
    Model_Account::Data *account = Model_Account::instance().get(accountID_);
    if (account) currency = Model_Account::currency(account); 
    mmCalculator calc;
    if (calc.is_ok(wxString() << Model_Currency::fromString(textAmount_->GetValue(), currency)))
        textAmount_->SetValue(Model_Currency::toString(calc.get_result(), currency));
    textAmount_->SetInsertionPoint(textAmount_->GetValue().Len());

    DataToControls();
}

void SplitDetailDialog::OnButtonOKClick( wxCommandEvent& /*event*/ )
{
    if (split_->CATEGID == -1)
    {
        mmShowErrorMessage(this, _("Invalid Category Entered "), _("Error"));
        bCategory_->SetFocus();
        return;
    }

    double amount;
    if (!textAmount_->GetDouble(amount))
    {
        mmShowErrorMessage(this, _("Invalid Amount Entered "), _("Error"));
        textAmount_->SetFocus();
        return;
    }

    if ( choiceType_->GetSelection() != transType_ )
    {
        amount = -amount;
    }

    split_->SPLITTRANSAMOUNT = amount;
    EndModal(wxID_OK);
}

bool SplitDetailDialog::ShowToolTips()
{
    return TRUE;
}

wxBitmap SplitDetailDialog::GetBitmapResource( const wxString& /*name*/ )
{
    return wxNullBitmap;
}

wxIcon SplitDetailDialog::GetIconResource( const wxString& /*name*/ )
{
    return wxNullIcon;
}
void SplitDetailDialog::OnCancel(wxCommandEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}
